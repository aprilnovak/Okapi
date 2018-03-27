#include "MultiAppMooseOkapiTransfer.h"
#include "OpenMCErrorHandling.h"
#include "OpenMCInterface.h"
#include "math.h"

#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "SubProblem.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<MultiAppMooseOkapiTransfer>()
{
  /* This is used to transfer data between Okapi and a generic MOOSE
     application when the MOOSE application is the sub App and Okapi
     the Master App. */
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
    "The auxiliary SCALAR variable to read values from.");
  params.addRequiredParam<std::vector<VariableName>>("to_aux_scalar",
    "The name of the SCALAR auxvariable to transfer the value to.");
  params.addRequiredParam<int32_t>("openmc_cell", "OpenMC cell ID (defined in "
    "XML input file) for this transfer to be associated with.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  params.addParam<bool>("store_results", true, "Whether to store the iteration "
    "results for every Picard iteration.");
  return params;
}

MultiAppMooseOkapiTransfer::MultiAppMooseOkapiTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _src_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar")),
    _cell(parameters.get<int32_t>("openmc_cell")),
    _dbg(parameters.get<bool>("dbg")),
    _store_results(parameters.get<bool>("store_results"))
{
}

void
MultiAppMooseOkapiTransfer::execute()
{
  _console << "Beginning MultiAppMooseOkapiTransfer Transfer " <<
    name() << std::endl;

  unsigned int num_apps = _multi_app->numGlobalApps();
  unsigned int num_vars_to_read = _src_var_names.size();
  unsigned int num_vars_to_write = _to_aux_names.size();

  // get the index of the cell in the cells(:) OpenMC array to be used
  // for later calls to cell-dependent OpenMC routines. This cannot be
  // put in the constructor because we cannot guarantee that the openmc_init
  // subroutine will be called before this object's constructor.
  int err_index = OpenMC::openmc_get_cell(_cell, &_index);
  ErrorHandling::openmc_get_cell(err_index, "MultiAppMooseOkapiTransfer");

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
          // get the variables to read from the sub App
          FEProblemBase & from_problem = _multi_app->appProblemBase(I);
          std::vector<MooseVariableScalar *> source_vars(num_vars_to_read);
          for (auto i = beginIndex(_src_var_names); i < num_vars_to_read; ++i)
          {
            source_vars[i] = &from_problem.getScalarVariable(_tid,
              _src_var_names[i]);
            source_vars[i]->reinit();
          }

          // Check that all of the source variables are of the same order
          unsigned int source_var_size =
            source_vars[beginIndex(_src_var_names)]->sln().size();
          for (auto i = beginIndex(_src_var_names); i < num_vars_to_read; ++i)
            if (source_vars[i]->sln().size() != source_var_size)
                mooseError("Order of source variables for "
                  "MultiAppMooseOkapiTransfer FROM_MULTIAPP "
                  "are not all the same!");

          // Initialize the moose_coeffs array and then populate by looping over
          // the source variables.
          int num_coeffs_from_moose = num_vars_to_read * source_var_size;
          double moose_coeffs[num_coeffs_from_moose];
          for (auto i = beginIndex(_src_var_names); i < num_vars_to_read; ++i)
          {
            auto & soln_values = source_vars[i]->sln();
            for (auto j = beginIndex(soln_values); j < soln_values.size(); ++j)
              moose_coeffs[i * num_vars_to_read + j] = soln_values[j];
          }

          if (_dbg)
          {
            _console << "Transferring " << num_coeffs_from_moose <<
              " coefficients from MOOSE to OpenMC for cell " << _cell << std::endl;

            for (int i = 0; i < num_coeffs_from_moose; ++i)
              _console << moose_coeffs[i] << " ";
            _console << std::endl;
          }

          // send all expansion coefficients for a cell to OpenMC
          int err_recv  = OpenMC::receive_coeffs_for_cell(_index,
            moose_coeffs, num_coeffs_from_moose);
          ErrorHandling::receive_coeffs_for_cell(err_recv);

        // Change a temperature in OpenMC. For now, only use a single coefficient,
        // since there's no continuous material tracking yet.
        Real temp = (source_vars[0]->sln())[0] / sqrt(2.0 * M_PI);
        if (_dbg) _console << "Setting OpenMC cell " << _cell <<
          " temperature to " << temp << std::endl;

        if (_store_results)
        {
          std::vector<Real> this_iteration;
          for (int i = 0; i < num_coeffs_from_moose; ++i)
            this_iteration.push_back(moose_coeffs[i]);

          _fuel_temp_coeffs.push_back(this_iteration);

          _console << "Fuel temperature coefficients up to iteration " <<
            _fuel_temp_coeffs.size() << std::endl;

          for (unsigned int i = 0; i < _fuel_temp_coeffs.size(); ++i)
            printResults(_fuel_temp_coeffs[i]);
        }

        // We pass a NULL pointer because we're not passing the optional instance
        // parameter.
        int err_temp = OpenMC::openmc_cell_set_temperature(_index, temp, NULL);
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
        _k_eff.push_back(OpenMC::get_keff());
        _console << "k_eff, up to iteration " << _k_eff.size() << ":" << std::endl;
        printResults(_k_eff);
      }

      // Before transferring any data back up to the Master App (MOOSE), OpenMC
      // stores the expansion coefficients in an array sorted by OpenMC cell index.
      OpenMC::fet_deconstruction();

     for (unsigned int I = 0; I < num_apps; ++I)
       if (_multi_app->hasLocalApp(I))
       {
         // initialize variables we're going to write
         std::vector<MooseVariableScalar *> to_vars(num_vars_to_write);

         for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
         {
           to_vars[i] = &_multi_app->appProblemBase(I).getScalarVariable(
             _tid, _to_aux_names[i]);
           to_vars[i]->reinit();
         }

         // Check that all of the variables to write are the same size
         unsigned int write_var_size =
           to_vars[beginIndex(_to_aux_names)]->sln().size();
         for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
           if (to_vars[i]->sln().size() != write_var_size)
             mooseError("The order of the variables to write for the "
               " MultiAppMooseOkapiTransfer are not all the same!");

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
         int num_coeffs_from_openmc = num_vars_to_write * write_var_size;
         double omc_coeffs[num_coeffs_from_openmc];

         // store coefficients from OpenMC into the omc_coeffs array and perform
         // error handling.
         int err_get = OpenMC::get_coeffs_from_cell(_index,
           omc_coeffs, num_coeffs_from_openmc);
         ErrorHandling::get_coeffs_from_cell(err_get);

         if (_dbg)
         {
            _console << "Transferring " << num_coeffs_from_openmc <<
              " coefficients from OpenMC to MOOSE for cell " << _cell << std::endl;
            for (int i = 0; i < num_coeffs_from_openmc; ++i)
              _console << omc_coeffs[i] << " ";
            _console << std::endl;
         }

         if (_store_results)
         {
           _console << "fission coefficients, up to iteration " <<
             _fission_coeffs.size() << ":" << std::endl;

           // store this iteration's coefficients
           std::vector<Real> this_iteration;
           for (int i = 0; i < num_coeffs_from_openmc; ++i)
             this_iteration.push_back(omc_coeffs[i]);

           // place this iteration's coefficients at end of vector, then print
           _fission_coeffs.push_back(this_iteration);
           for (unsigned int i = 0; i < _k_eff.size(); ++i)
             printResults(_fission_coeffs[i]);
         }

         // Loop over the variables that we are going to write
         for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
         {
           std::vector<dof_id_type> & dof = to_vars[i]->dofIndices();
           auto & soln_values = to_vars[i]->sln();
           for (auto j = beginIndex(soln_values); j < write_var_size; ++j)
             to_vars[i]->sys().solution().set(
               dof[j], omc_coeffs[i * write_var_size + j]);

           to_vars[i]->sys().solution().close();
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
