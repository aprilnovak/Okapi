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
  params.addRequiredParam<int32_t>(
      "openmc_tally", "The functional expansion tally id that covers the desired cell geometry.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  params.addParam<bool>("store_results",
                        true,
                        "Whether to store the iteration "
                        "results for every Picard iteration.");
  MooseEnum geometry_type("cartesian cylindrical");
  params.addRequiredParam<MooseEnum>(
      "geometry_type", geometry_type, "The type of geometry. Either cylindrical or cartesian.");
  return params;
}

MultiAppMooseOkapiTransfer::MultiAppMooseOkapiTransfer(const InputParameters & parameters)
  : MultiAppFXTransfer(parameters),
    _cell(parameters.get<int32_t>("openmc_cell")),
    _tally(parameters.get<int32_t>("openmc_tally")),
    _dbg(parameters.get<bool>("dbg")),
    _store_results(parameters.get<bool>("store_results")),
    _geometry_type(getParam<MooseEnum>("geometry_type"))
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
  int err_index = openmc_get_cell_index(_cell, &_cell_index);
  ErrorHandling::openmc_get_cell_index(err_index, "MultiAppMooseOkapiTransfer");
  err_index = openmc_get_tally_index(_tally, &_tally_index);
  ErrorHandling::openmc_get_tally_index(err_index, "MultiAppMooseOkapiTransfer");

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
          Real temp = moose_coeffs[0];
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

          // We pass a nullptr because we're not passing the optional instance
          // parameter.
          int err_temp = openmc_cell_set_temperature(_cell_index, temp, nullptr);
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

          int32_t * filter_indices = nullptr;
          int32_t num_filter_indices;
          int err_get =
              openmc_tally_get_filters(_tally_index, &filter_indices, &num_filter_indices);
          ErrorHandling::openmc_tally_get_filters(err_get, "MultiAppMooseOkapiTransfer");

          if (num_filter_indices != 2)
            mooseError(
                "We expect there to be exactly two filters, one a cell filter and the other an "
                "expansion filter.");
          std::map<int32_t, int32_t> index_to_id;
          for (decltype(n_cells) i = 0; i < n_cells; ++i)
          {
            int32_t id;
            err_get = openmc_cell_get_id(i + 1, &id);
            ErrorHandling::openmc_cell_get_id(err_get, "MultiAppMooseOkapiTransfer");
            index_to_id[i + 1] = id;
          }

          // Important variables
          int32_t order;
          char type[20];
          int32_t * cell_indices = nullptr;
          int32_t num_cells_in_filter;
          bool cell_id_found = false;
          int32_t stride_integer = 0;
          int32_t cell_filter_index;

          std::string cell_filter_name = "cell";
          bool cell_filter_found = false;
          for (int32_t i = 0; i <= 1; ++i)
          {
            err_get = openmc_filter_get_type(filter_indices[i], type);
            ErrorHandling::openmc_filter_get_type(err_get, "MultiAppMooseOkapiTransfer");
            if (cell_filter_name.compare(type))
              continue;
            else
            {
              cell_filter_found = true;
              cell_filter_index = i;
              break;
            }
          }
          if (!cell_filter_found)
            mooseError("No cell filter specified. Check the tallies XML input file.");
          else
          {
            err_get = openmc_cell_filter_get_bins(
                filter_indices[cell_filter_index], &cell_indices, &num_cells_in_filter);
            ErrorHandling::openmc_cell_filter_get_bins(err_get, "MultiAppMooseOkapiTransfer");

            for (decltype(num_cells_in_filter) i = 0; i < num_cells_in_filter; ++i)
            {
              if (cell_indices[i] == _cell_index)
              {
                cell_id_found = true;
                stride_integer = i;
                break;
              }
            }
            if (!cell_id_found)
              mooseError("Requested cell_id not in the passed tally. Check that the cell filter in "
                         "the tallies XML file contains the ID you're requesting");
          }
          int32_t expansion_filter_index = 1 - cell_filter_index;
          err_get = openmc_filter_get_type(filter_indices[expansion_filter_index], type);
          ErrorHandling::openmc_filter_get_type(err_get, "MultiAppMooseOkapiTransfer");
          getOrderAndCheckExpansionType(type, filter_indices[expansion_filter_index], order);

          Real multiplier = _geometry_type == "cylindrical" ? 2. : 1.;

          // Determine size of array to allocate based on size of MOOSE variables
          std::vector<Real> & coefficients = to_object.getCoefficients();
          double * tally_results = nullptr;
          int shape[3];

          err_get = openmc_tally_results(_tally_index, &tally_results, shape);
          ErrorHandling::openmc_tally_results(err_get, "MultiAppMooseOkapiTransfer");

          std::vector<double> tally_results_mean(shape[1] * shape[2]);
          if (tally_results_mean.size() != coefficients.size())
            mooseError(
                "The coefficient vector size from openmc doesn't match the coefficient vector size "
                "from MOOSE. Check that the expansion orders are consistent between openmc and "
                "MOOSE input files.");
          for (auto i = beginIndex(tally_results_mean); i < tally_results_mean.size(); ++i)
            tally_results_mean[i] = tally_results[1 + i * 3];

          if (cell_filter_index == 0)
          {
            // The point at which the results we care about begin
            auto starting_point = coefficients.size() * stride_integer;
            std::vector<double> temp_results(&tally_results_mean[starting_point],
                                             &tally_results_mean[starting_point] +
                                                 coefficients.size());

            std::transform(temp_results.begin(),
                           temp_results.end(),
                           temp_results.begin(),
                           std::bind(std::multiplies<Real>(),
                                     std::placeholders::_1,
                                     multiplier / n_realizations));
            coefficients = std::move(temp_results);
          }
          else
          {
            std::vector<double> temp_results;
            for (auto i = beginIndex(coefficients); i < coefficients.size(); ++i)
              temp_results.push_back(tally_results_mean[stride_integer + i * num_cells_in_filter] *
                                     multiplier / n_realizations);
            coefficients = std::move(temp_results);
          }

          if (_dbg)
          {
            _console << "Transferring " << coefficients.size()
                     << " coefficients from OpenMC to MOOSE for cell " << _cell << std::endl;
            for (auto i = beginIndex(coefficients); i < coefficients.size(); ++i)
              _console << coefficients[i] / multiplier << " ";
            _console << std::endl;
          }
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

bool
MultiAppMooseOkapiTransfer::getOrderAndCheckExpansionType(const char * type,
                                                          const int32_t & index,
                                                          int32_t & order)
{
  int err_get;
  if (!(std::string("legendre").compare(type)))
    err_get = openmc_legendre_filter_get_order(index, &order);
  else if (!(std::string("sphericalharmonics").compare(type)))
    err_get = openmc_sphharm_filter_get_order(index, &order);
  else if (!(std::string("spatiallegendre").compare(type)))
    err_get = openmc_spatial_legendre_filter_get_order(index, &order);
  else if (!(std::string("zernike").compare(type)))
    err_get = openmc_zernike_filter_get_order(index, &order);
  else
    mooseError("No expansion filter specified. Check the tallies XML input file.");

  ErrorHandling::openmc_filter_get_order(err_get, "MultiAppMooseOkapiTransfer");
  return 1;
}
