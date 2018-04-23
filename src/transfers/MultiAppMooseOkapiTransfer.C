#include "MultiAppMooseOkapiTransfer.h"
#include "OpenMCErrorHandling.h"
#include "openmc.h"
#include "math.h"

#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "SubProblem.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template <>
InputParameters
validParams<MultiAppMooseOkapiTransfer>()
{
  /* This is used to transfer data between Okapi and a generic MOOSE
     application when the MOOSE application is the sub App and Okapi
     the Master App. */
  InputParameters params = validParams<MultiAppFXTransfer>();
  params.suppressParameter<std::string>("this_app_object_name");
  params.set<std::string>("this_app_object_name") = "";
  params.addRequiredParam<int32_t>("openmc_cell",
                                   "OpenMC cell ID (defined in "
                                   "XML input file) for this transfer to be associated with.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  params.addParam<bool>("store_results",
                        true,
                        "Whether to store the iteration "
                        "results for every Picard iteration.");
  return params;
}

MultiAppMooseOkapiTransfer::MultiAppMooseOkapiTransfer(const InputParameters & parameters)
  : MultiAppFXTransfer(parameters),
    _cell(parameters.get<int32_t>("openmc_cell")),
    _dbg(parameters.get<bool>("dbg")),
    _store_results(parameters.get<bool>("store_results"))
{
}

void
MultiAppMooseOkapiTransfer::initialSetup()
{
  // Search for the _multi_app_object_name in each of the MultiApps
  for (std::size_t i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
    {
      if (i == 0) // First time through, assign without checking against previous values
        getSubAppObject = scanProblemBaseForObject(
            _multi_app->appProblemBase(i), _multi_app_object_name, _multi_app->name());
      else if (getSubAppObject != scanProblemBaseForObject(_multi_app->appProblemBase(i),
                                                           _multi_app_object_name,
                                                           _multi_app->name()))
        mooseError("The name '",
                   _multi_app_object_name,
                   "' is assigned to two different object types. Please modify your input file and "
                   "try again.");
    }
  if (getSubAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _multi_app_object_name, "' in SubApp");
}

void
MultiAppMooseOkapiTransfer::execute()
{
  _console << "Beginning MultiAppMooseOkapiTransfer Transfer " << name() << std::endl;

  unsigned int num_apps = _multi_app->numGlobalApps();

  // get the index of the cell in the cells(:) OpenMC array to be used
  // for later calls to cell-dependent OpenMC routines. This cannot be
  // put in the constructor because we cannot guarantee that the openmc_init
  // subroutine will be called before this object's constructor.
  int err_index = openmc_get_cell_index(_cell, &_index);
  ErrorHandling::openmc_get_cell_index(err_index, "MultiAppMooseOkapiTransfer");

  switch (_direction)
  {
    // MOOSE -> Okapi. This transfer is used to pass coefficients for fuel
    // temperature to Okapi.
    case FROM_MULTIAPP:
    {
      for (unsigned int I = 0; I < num_apps; ++I)
      {
        if (_multi_app->hasLocalApp(I) && _multi_app->isRootProcessor())
        {
          // Get a reference to the first thread object in the first MultiApp
          MutableCoefficientsInterface & from_object =
              (this->*getSubAppObject)(_multi_app->appProblemBase(I), _multi_app_object_name, 0);

          std::vector<Real> & coefficients = from_object.getCoefficients();

          // Initialize the moose_coeffs array and then populate by looping over
          // the source variables.
          size_t num_coeffs_from_moose = coefficients.size();
          double * moose_coeffs = &coefficients[0];

          if (_dbg)
          {
            _console << "Transferring " << num_coeffs_from_moose
                     << " coefficients from MOOSE to OpenMC for cell " << _cell << std::endl;

            for (decltype(num_coeffs_from_moose) i = 0; i < num_coeffs_from_moose; ++i)
              _console << moose_coeffs[i] << " ";
            _console << std::endl;
          }

          // Change a temperature in OpenMC. For now, only use a single coefficient,
          // since there's no continuous material tracking yet.
          Real temp = moose_coeffs[0] / sqrt(2.0 * M_PI);
          if (_dbg)
            _console << "Setting OpenMC cell " << _cell << " temperature to " << temp << std::endl;

          if (_store_results)
          {
            std::vector<Real> this_iteration;
            for (decltype(num_coeffs_from_moose) i = 0; i < num_coeffs_from_moose; ++i)
              this_iteration.push_back(moose_coeffs[i]);

            _fuel_temp_coeffs.push_back(this_iteration);

            _console << "Fuel temperature coefficients up to iteration " << _fuel_temp_coeffs.size()
                     << std::endl;

            for (unsigned int i = 0; i < _fuel_temp_coeffs.size(); ++i)
              printResults(_fuel_temp_coeffs[i]);
          }

          // We pass a NULL pointer because we're not passing the optional instance
          // parameter.
          int err_temp = openmc_cell_set_temperature(_index, temp, NULL);
          ErrorHandling::openmc_cell_set_temperature(err_temp);
        }
      }
      break;
    }

    // Okapi -> MOOSE. This transfer is used to transfer coefficients for the kappa
    // fission distribution from OpenMC to MOOSE.
    case TO_MULTIAPP:
    {
      // get the value for k and print it
      if (_store_results)
      {
        // get k_eff value from OpenMC, then store results in _k_eff vector.
        double keff[2];
        int err_keff = openmc_get_keff(keff);
        ErrorHandling::openmc_get_keff(err_keff);
        _k_eff.push_back(keff[0]);
        _console << "k_eff, up to iteration " << _k_eff.size() << ":" << std::endl;
        printResults(_k_eff);
      }

      for (unsigned int I = 0; I < num_apps; ++I)
      {
        if (_multi_app->hasLocalApp(I))
        {
          // Get a reference to the object in each MultiApp
          MutableCoefficientsInterface & to_object =
              (this->*getSubAppObject)(_multi_app->appProblemBase(I), _multi_app_object_name, 0);

          // Initialize an array to hold the expansion coefficients we'll receive
          // from OpenMC. This array holds all of the expansion coefficients
          // for the cell we specify. For a generic Zernike-Legendre expansion,
          // the order used by OpenMC is specified in the tallies XML file. It is
          // assumed that this XML order matches the available slots to write in
          // the to_aux_scalar variables provided from MOOSE, so this is used to
          // determine how large of an array to specify here that will be written.
          // A check is provided in OpenMC to make sure that the length of the
          // t % coeffs array) matches what is allocated in C++ to receive it.

          // Determine size of array to allocate based on size of MOOSE variables
          std::vector<Real> & coefficients = to_object.getCoefficients();
          size_t num_coeffs_from_openmc = coefficients.size();
          double * tally_results = nullptr;
          int shape_[3];

          // store coefficients from OpenMC into the omc_coeffs array and perform
          // error handling.
          int err_get = openmc_tally_results(_index, &tally_results, shape_);
          ErrorHandling::get_coeffs_from_cell(err_get);

          // if (_dbg)
          // {
          //   _console << "Transferring " << num_coeffs_from_openmc
          //            << " coefficients from OpenMC to MOOSE for cell " << _cell << std::endl;
          //   for (int i = 0; i < num_coeffs_from_openmc; ++i)
          //     _console << omc_coeffs[i] << " ";
          //   _console << std::endl;
          // }

          // if (_store_results)
          // {
          //   _console << "fission coefficients, up to iteration " << _fission_coeffs.size() << ":"
          //            << std::endl;

          //   // store this iteration's coefficients
          //   std::vector<Real> this_iteration;
          //   for (int i = 0; i < num_coeffs_from_openmc; ++i)
          //     this_iteration.push_back(omc_coeffs[i]);

          //   // place this iteration's coefficients at end of vector, then print
          //   _fission_coeffs.push_back(this_iteration);
          //   for (unsigned int i = 0; i < _k_eff.size(); ++i)
          //     printResults(_fission_coeffs[i]);
          // }
        }
      }
      break;
    }
  }
  _console << "Finished MultiAppMooseOkapiTransfer transfer " << name() << std::endl;
}

void
MultiAppMooseOkapiTransfer::printResults(std::vector<Real> & results)
{
  for (unsigned int i = 0; i < results.size(); ++i)
    _console << results[i] << ", ";

  _console << std::endl;
}
