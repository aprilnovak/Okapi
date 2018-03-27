#include "MultiAppOkapiMooseTransfer.h"
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

template <>
InputParameters
validParams<MultiAppOkapiMooseTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  /* To MultiApp (MOOSE -> OkapiApp):
     source_variable = SCALAR variables holding expansion coefficients.
                       NOTE: the order of the variables is important here,
                       since they will be stored in the same order in the
                       OpenMC expansion_coeffs global array, and hence will
                       be interpreted as the Zernike coefficients for a given
                       Legendre order, in order of increasing Legendre order.
     to_aux_scalar = dummy variable (OpenMC is only wrapped as a MOOSE App,
                     there's no FEM-based variable to store anything in */

  /* From MultiApp (OkapiApp -> MOOSE):
     source_varibale = dummy variable
     to_aux_scalar = expansion coefficients used in MOOSE */

  params.addRequiredParam<std::vector<VariableName>>(
      "source_variable", "The auxiliary SCALAR variable to read values from");
  params.addRequiredParam<std::vector<VariableName>>(
      "to_aux_scalar",
      "The name of the SCALAR auxvariable in the MultiApp to transfer the "
      "value to.");
  params.addRequiredParam<int>("openmc_cell",
                               "OpenMC cell ID (defined in "
                               "input file) for this transfer to be associated with.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  return params;
}

MultiAppOkapiMooseTransfer::MultiAppOkapiMooseTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar")),
    _cell(parameters.get<int>("openmc_cell")),
    _dbg(parameters.get<bool>("dbg"))
{
}

void
MultiAppOkapiMooseTransfer::execute()
{
  _console << "Beginning MultiAppOkapiMooseTransfer Transfer " << name() << std::endl;
  /* This returns the total number of _sub_ Apps, which is equal to the number
     of (x,y,z) coordinate pairs in the input file. Each of these positions
     represents an offset of a sub-application in the space of the master App
     (the local Master app). */
  int num_apps = _multi_app->numGlobalApps();

  int num_vars_to_read = _source_var_names.size();
  int num_vars_to_write = _to_aux_names.size();

  FEProblemBase & from_problem = _multi_app->problemBase();

  /* Store the legendre and zernike expansion orders to pass them into OpenMC.
     A check is made to ensure that the number of Zernike coefficients is the
     same for every coupled scalar auxvariable. */
  int legendre_order_from_MOOSE = 0;
  int zernike_order_from_MOOSE = 0;
  int old_zernike_order = 0;

  int old_zernike_order_OpenMC = 0;

  switch (_direction)
  {
    /* MasterApp -> SubApp. For MOOSE-OpenMC coupling, this represents the
       transfer of information from MOOSE to OpenMC. This transfers all
       of the expansion coefficients. */
    case TO_MULTIAPP:
    {
      std::vector<MooseVariableScalar *> source_variables(num_vars_to_read);
      for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
      {
        source_variables[i] = &from_problem.getScalarVariable(_tid, _source_var_names[i]);
        source_variables[i]->reinit();
      }

      /* Loop through each of the sub apps. Because MC neutron transport
         simulations require knowledge of the entire domain (i.e. cannot specify
         something like a temperature boundary condition in a Monte Carlo code),
         we'll likely never have multiple OpenMC sub apps. */
      for (unsigned int i = 0; i < num_apps; i++)
        if (_multi_app->hasLocalApp(i))
        {
          /* first, we need to determine how many coefficients will be passed from
             MOOSE to OpenMC. A check is made after computing these quantities to
             ensure that all of the source auxvariables are of the same order, but
             so that we don't need to duplicate the loop structure below, we assume
             that the number of variables to transfer equals the number of variables
             to read multiplied by the order of the first of these variables. If
             the source variables are actually the same size, an error will be thrown
             before this would become a problem. */
          int num_coeffs_from_moose =
              num_vars_to_read * source_variables[beginIndex(_source_var_names)]->sln().size();
          double moose_coeffs[num_coeffs_from_moose];

          /* Loop through each SCALAR variable. The order of the Legendre
             expansion equals the number of source variables - 1, since the
             Legendre order can be zero. */
          legendre_order_from_MOOSE = num_vars_to_read - 1;
          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            // Loop through each entry in a single SCALAR variable
            auto & soln_values = source_variables[i]->sln();

            // Check that Zernike order is the same for each coupled scalar auxvar
            old_zernike_order = zernike_order_from_MOOSE;

            if (i > 0)
              if (old_zernike_order != zernike_order_from_MOOSE)
                mooseError("The order of the Zernike expansion does not "
                           "match each coupling Legendre coefficient aux variable.");

            zernike_order_from_MOOSE = zernike_order_from_coeffs(soln_values.size());

            for (auto j = beginIndex(soln_values); j < soln_values.size(); ++j)
              moose_coeffs[i * num_vars_to_read + j] = soln_values[j];

            _console << '\n';
          }

          if (_dbg)
          {
            _console << "Transferring " << num_coeffs_from_moose
                     << " coefficients"
                        " from MOOSE to OpenMC..."
                     << std::endl;
            _console << "For cell " << _cell << ":" << std::endl;

            for (int i = 0; i < num_coeffs_from_moose; ++i)
              _console << moose_coeffs[i] << " ";

            _console << std::endl;
          }

          // send all expansion coefficients for a cell to OpenMC at one time
          int err = OpenMC::receive_coeffs_for_cell(_cell, moose_coeffs, num_coeffs_from_moose);
          ErrorHandling::receive_coeffs_for_cell(err);

          /* Change a temperature in OpenMC. For now, only use a single coefficient,
             since there's no continuous material tracking yet. Note that changing
             a temperature in OpenMC requires that you've loaded cross section data
             at that temperature, so use the temperature_range parameter in the
             settings XML file. Because this isn't expanded in OpenMC, here we have
             to apply the scaling factors for a zero-th order Legendre and zero-th
             order Zernike expansion. */
          OpenMC::openmc_cell_set_temperature(
              _cell, (source_variables[0]->sln())[0] / sqrt(2.0 * M_PI), NULL);
        }
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      /* Before transferring any data back up to the Master App (MOOSE),
         OpenMC stores the expansion coefficients in an array sorted by
         OpenMC cell index. */
      OpenMC::fet_deconstruction();

      // initialize variables we're going to write
      std::vector<MooseVariableScalar *> to_variables(num_vars_to_write);
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        to_variables[i] = &from_problem.getScalarVariable(_tid, _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      /* Initialize an array to hold the expansion coefficients we'll receive
         from OpenMC. This array holds _all_ of the expansion coefficients
         for the cell we specify. For a generic Zernike-Legendre expansion,
         the order used by OpenMC is specified in the tallies XML file. It is
         assumed that this XML order matches the available slots to write in
         the to_aux_scalar variables provided from MOOSE, so this is used to
         determine how large of an array to specify here that will be written.
         A check is provided in OpenMC to make sure that the length of the
         t % coeffs array) matches what is allocated in C++ to receive it.*/

      // Check that all of the coupled variables are of the same length
      int zernike_order_from_OpenMC =
          zernike_order_from_coeffs(to_variables[beginIndex(_to_aux_names)]->sln().size());
      for (int i = 0; i < num_vars_to_write; ++i)
      {
        if (i > 0)
          if (old_zernike_order_OpenMC != zernike_order_from_OpenMC)
            mooseError("The order of the Zernike expansion does not "
                       "match each coupling Legendre coefficient aux variable.");
        old_zernike_order_OpenMC = (to_variables[i]->sln()).size();
      }

      // Determine size of array to allocate based on size of MOOSE variables
      int num_zernike_coeffs_per_var = (to_variables[beginIndex(_to_aux_names)]->sln()).size();

      int num_coeffs_from_openmc = num_vars_to_write * num_zernike_coeffs_per_var;
      double omc_coeffs[num_coeffs_from_openmc];

      // store coefficients from OpenMC into the omc_coeffs array
      int err = OpenMC::get_coeffs_from_cell(_cell, omc_coeffs, num_coeffs_from_openmc);
      ErrorHandling::get_coeffs_from_cell(err);

      if (_dbg)
      {
        _console << "Transferring " << num_coeffs_from_openmc
                 << " coefficients"
                    " from OpenMC to MOOSE..."
                 << std::endl;
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
        for (auto j = beginIndex(soln_values); j < soln_values.size(); ++j)
        {
          to_variables[i]->sys().solution().set(dof[j],
                                                omc_coeffs[i * num_zernike_coeffs_per_var + j]);
        }
        to_variables[i]->sys().solution().close();
      }

      break;
    } // end FROM_MULTIAPP
  }

  _console << "Finished MultiAppOkapiMooseTransfer transfer" << name() << std::endl;
}
