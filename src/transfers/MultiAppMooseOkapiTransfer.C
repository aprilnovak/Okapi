#include "MultiAppMooseOkapiTransfer.h"
#include "ExtraFunctions.h"
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
  /* This transfer is used to transfer data between Okapi and a generic
     MOOSE application when the MOOSE application is the sub App and Okapi
     the Master App. */
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
    "The auxiliary SCALAR variable to read values from");
  params.addRequiredParam<std::vector<VariableName> >("to_aux_scalar",
    "The name of the SCALAR auxvariable in the MultiApp to transfer the "
    "value to.");
  params.addRequiredParam<int>("openmc_cell", "OpenMC cell ID (defined in "
    "input file) for this transfer to be associated with.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  return params;
}

MultiAppMooseOkapiTransfer::MultiAppMooseOkapiTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar")),
    _cell(parameters.get<int>("openmc_cell")),
    _dbg(parameters.get<bool>("dbg"))
{
}

void
MultiAppMooseOkapiTransfer::execute()
{
  _console << "Beginning MultiAppMooseOkapiTransfer Transfer " <<
    name() << std::endl;

  int num_apps = _multi_app->numGlobalApps();
  int num_vars_to_read = _source_var_names.size();
  int num_vars_to_write = _to_aux_names.size();

//  FEProblemBase & from_problem = _multi_app->problemBase();

  switch (_direction)
  {
    // MOOSE -> Okapi. This transfer is used to pass coefficients for fuel
    // temperature and fuel density to Okapi.
    case FROM_MULTIAPP:
    {
      for (unsigned int i = 0; i < num_apps; ++i)
      {
        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
        {
          // get the variables to read from the sub App
          FEProblemBase & from_problem = _multi_app->appProblemBase(i);
          std::vector<MooseVariableScalar *> source_variables(num_vars_to_read);
          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            source_variables[i] = &from_problem.getScalarVariable(_tid,
              _source_var_names[i]);
            source_variables[i]->reinit(); // not done in my MOOSE transfer
          }

          // Check that all of the source variables are of the same order
          int source_var_size =
            source_variables[beginIndex(_source_var_names)]->sln().size();
          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            if (source_variables[i]->sln().size() != source_var_size)
              mooseError("Order of source variables for "
                "MultiAppMooseOkapiTranfser are not all the same!");
          }

          // Initialize the moose_coeffs array and then populate by looping over
          // the source variables.
          int num_coeffs_from_moose = num_vars_to_read * source_var_size;
          double moose_coeffs[num_coeffs_from_moose];

          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            auto & soln_values = source_variables[i]->sln();
            for (auto j = beginIndex(soln_values); j < soln_values.size(); ++j)
            {
              moose_coeffs[i * num_vars_to_read + j] = soln_values[j];
            }
          }

          if (_dbg)
          {
            _console << "Transferring " << num_coeffs_from_moose <<
              " coefficients from MOOSE to OpenMC..." << std::endl;
            _console << "For cell " << _cell << ":" << std::endl;

            for (int i = 0; i < num_coeffs_from_moose; ++i)
              _console << moose_coeffs[i] << " ";

            _console << std::endl;
          }

          // send all expansion coefficients for a cell to OpenMC at one time and
          // perform error handling
          int err  = OpenMC::receive_coeffs_for_cell(_cell,
            moose_coeffs, num_coeffs_from_moose);
          ErrorHandling::receive_coeffs_for_cell(err);

      // Change a temperature in OpenMC. For now, only use a single coefficient,
      //   since there's no continuous material tracking yet. Note that changing
      //   a temperature in OpenMC requires that you've loaded cross section data
      //   at that temperature, so use the temperature_range parameter in the
      //   settings XML file. Because this isn't expanded in OpenMC, here we have
      //   to apply the scaling factors for a zero-th order Legendre and zero-th
      //   order Zernike expansion.
      OpenMC::openmc_cell_set_temperature(_cell, \
        (source_variables[0]->sln())[0] / sqrt(2.0 * M_PI));

        }
      }

      break;
    }

    // Okapi -> MOON. This transfer is used to transfer coefficients for the kappa
    // fission distribution from OpenMC to MOOSE.
    case TO_MULTIAPP:
    {
      // Before transferring any data back up to the Master App (MOOSE),
      //   OpenMC stores the expansion coefficients in an array sorted by
      //   OpenMC cell index.
      OpenMC::fet_deconstruction();

     for (unsigned int i = 0; i < num_apps; ++i)
       if (_multi_app->hasLocalApp(i))
       {
         // initialize variables we're going to write
         std::vector<MooseVariableScalar *> to_variables(num_vars_to_write);

         for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
         {
           to_variables[i] = &_multi_app->appProblemBase(i).getScalarVariable(
             _tid, _to_aux_names[i]);
           to_variables[i]->reinit();
         }

         // Check that all of the variables to write are the same size
         int write_var_size = to_variables[beginIndex(_to_aux_names)]->sln().size();
         for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
         {
           if (to_variables[i]->sln().size() != write_var_size)
             mooseError("The order of the variables to write for the "
               " MultiAppMooseOkapiTransfer are not all the same!");
         }

         // Initialize an array to hold the expansion coefficients we'll receive
         // from OpenMC. This array holds _all_ of the expansion coefficients
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
         int err = OpenMC::get_coeffs_from_cell(_cell,
           omc_coeffs, num_coeffs_from_openmc);
         ErrorHandling::get_coeffs_from_cell(err);

         if (_dbg)
         {
            _console << "Transferring " << num_coeffs_from_openmc << " coefficients"
              " from OpenMC to MOOSE..." << std::endl;
            _console << "For cell " << _cell << ":" << std::endl;

            for (int i = 0; i < num_coeffs_from_openmc; ++i)
              _console << omc_coeffs[i] << " ";

            _console << std::endl;
         }

         // Loop over the variables that we are going to write
         for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
         {
           std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();
           auto & soln_values = to_variables[i]->sln();
           for (auto j = beginIndex(soln_values); j < write_var_size; ++j)
             to_variables[i]->sys().solution().set(dof[j], omc_coeffs[i * write_var_size + j]);

           to_variables[i]->sys().solution().close();
         }
       }

      break;
    }
  }

  _console << "Finished MultiAppMooseOkapiTransfer transfer " << name() << std::endl;
}
