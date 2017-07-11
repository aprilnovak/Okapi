#include "MultiAppMoonOkapiTransfer.h"
#include "NekInterface.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<MultiAppMoonOkapiTransfer>()
{
  /* This transfer is used to transfer information between Okapi and
     MOON when MOON is the sub App to an Okapi Master App. */
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<VariableName> >("source_variable",
    "The auxiliary scalar variable to read values from");
  params.addRequiredParam<std::vector<VariableName> >("to_aux_scalar",
    "The name of the scalar Aux variable in the MultiApp to transfer"
    " the value to.");
  return params;
}

MultiAppMoonOkapiTransfer::MultiAppMoonOkapiTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar"))
{
}

void
MultiAppMoonOkapiTransfer::execute()
{
  _console << "Beginning PolynomialToNekTransfer " << name() << std::endl;

  int num_apps = _multi_app->numGlobalApps();
  int num_vars_to_read = _source_var_names.size();
  int num_vars_to_write = _to_aux_names.size();

  switch (_direction)
  {
    // Okapi -> MOON. This direction transfers coefficients for a heat flux
    // boundary condition.
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      std::vector<MooseVariableScalar *> source_variables(num_vars_to_read);

      for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
      {
        source_variables[i] = &from_problem.getScalarVariable(_tid,
          _source_var_names[i]);
        source_variables[i]->reinit();
      }

      // Check that the order of each of the source variables is the same by
      // comparing with the size of the first variable.
      int source_var_size =
        source_variables[beginIndex(_source_var_names)]->sln().size();
      for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
      {
        if (source_variables[i]->sln().size() != source_var_size)
          mooseError("The order of the source variables for the "
            "MultiAppMoonOkapiTransfer are not all the same!");
      }

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < num_apps; i++)
        if (_multi_app->hasLocalApp(i))
        {
          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            _console << _source_var_names[i] << '\n';
            auto & soln_values = source_variables[i]->sln();
            for (auto j = beginIndex(soln_values); j < soln_values.size(); ++j)
            {
              _console << soln_values[j] << ' ';
              Nek5000::expansion_fcoef_.coeff_fij[i*100+j] = soln_values[j];
            }
            _console << '\n';
          }
        }
      FORTRAN_CALL(Nek5000::flux_reconstruction)();
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      FORTRAN_CALL(Nek5000::nek_expansion)();
      // The number of sub applications
      unsigned int num_apps = _multi_app->numGlobalApps();

      std::vector<MooseVariableScalar *> to_variables(_to_aux_names.size());
      for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
      {
        to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
      {
        // The dof indices for the scalar variable of interest
        std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();

//      // Error if there is a size mismatch between the scalar AuxVariable and the number of sub apps
//      if (num_apps != scalar.sln().size())
//        mooseError("The number of sub apps (" << num_apps << ") must be equal to the order of the scalar AuxVariable (" << scalar.order() << ")");

        // Loop over each sub-app and populate the AuxVariable values from the postprocessors
//        for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)

//        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
          // Note: This can't be done using MooseScalarVariable::insert() because different processors will be setting dofs separately.
        auto & soln_values = to_variables[i]->sln();
        for (auto j = beginIndex(soln_values); j < soln_values.size(); ++j)
        {
          to_variables[i]->sys().solution().set(dof[j], Nek5000::expansion_tcoef_.coeff_tij[i*100+j]);
          to_variables[i]->sys().solution().close();
        }
      }

      break;
    }
  }

  _console << "Finished PolynomialToNekTransfer" << name() << std::endl;
}
