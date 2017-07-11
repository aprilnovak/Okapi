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
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information.");
  return params;
}

MultiAppMoonOkapiTransfer::MultiAppMoonOkapiTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar")),
    _dbg(parameters.get<bool>("dbg"))
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

      for (unsigned int i = 0; i < num_apps; i++)
        if (_multi_app->hasLocalApp(i))
        {
          if(_dbg)
            _console << "Writing flux BC coefficients from Nek to Okapi..." << std::endl;

          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            auto & soln_values = source_variables[i]->sln();
            for (auto j = beginIndex(soln_values); j < source_var_size; ++j)
            {
              if (_dbg)
                _console << soln_values[j] << ' ';
              Nek5000::expansion_fcoef_.coeff_fij[i*100+j] = soln_values[j];
            }
            if (_dbg)
              _console << '\n';
          }
        }
      FORTRAN_CALL(Nek5000::flux_reconstruction)();
      break;
    }

    // MOON -> Okapi. This direction is used to transfer coefficients for a
    // temperature BC, fluid density, and fluid temperature.
    case FROM_MULTIAPP:
    {
      FORTRAN_CALL(Nek5000::nek_expansion)();

      std::vector<MooseVariableScalar *> to_variables(num_vars_to_write);
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid,
          _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      // Check that all of the variables to write are of the same order.
      int write_var_size = to_variables[beginIndex(_to_aux_names)]->sln().size();
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        if (to_variables[i]->sln().size() != write_var_size)
          mooseError("Order of variables to write with MultiAppMoonOkapiTransfer"
            " are not all the same!");
      }

      if(_dbg)
        _console << "Writing temp BC coefficients from MOON to Okapi..." << std::endl;
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();
        auto & soln_values = to_variables[i]->sln();
        for (auto j = beginIndex(soln_values); j < write_var_size; ++j)
        {
          to_variables[i]->sys().solution().set(dof[j],
            Nek5000::expansion_tcoef_.coeff_tij[i*100+j]);

          if (_dbg)
            _console << soln_values[j] << ' ';
        }
        if (_dbg)
          _console << '\n';
        to_variables[i]->sys().solution().close();
      }

      break;
    }
  }

  _console << "Finished PolynomialToNekTransfer" << name() << std::endl;
}
