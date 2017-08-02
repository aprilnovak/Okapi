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
    "The auxiliary scalar variable(s) to read values from");
  params.addRequiredParam<std::vector<VariableName> >("to_aux_scalar",
    "The name of the scalar Aux variable(s) in the MultiApp to transfer"
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
  _console << "Beginning MultiAppMoonOkapiTransfer " << name() << std::endl;

  int num_apps = _multi_app->numGlobalApps();
  int num_vars_to_read = _source_var_names.size();
  int num_vars_to_write = _to_aux_names.size();

  switch (_direction)
  {
    // Okapi -> MOON. This direction transfers coefficients for a heat flux
    // boundary condition that is computed by BISON (Okapi acts as the middle-man).
    case TO_MULTIAPP:
    {
      // Get the source variables from the Okapi master App
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
        if (source_variables[i]->sln().size() != source_var_size)
          mooseError("The order of the source variables for the "
            "MultiAppMoonOkapiTransfer direction TO_MULTIAPP are "
            "not all the same!");

      // Check that the number of variables matches the number of Fourier
      // terms used in Nek, and that the length of each variable matches the
      // number of Legendre terms in Nek. In general, m_fourier and n_legendre
      // could be set in an external code to reduce redunant information
      // specification by the user, but care has to be taken that the values are
      // set before they are needed.
      if (num_vars_to_read != Nek5000::expansion_tdata_.m_fourier)
        mooseError("Number of Fourier polynomials in Okapi does not agree "
          "with number expected from Nek! Change m_fourier in the Nek usr file "
          "or the number of source variables in MultiAppMoonOkapiTransfer!");
      if (source_var_size != Nek5000::expansion_tdata_.n_legendre)
        mooseError("Number of Legendre polynomials in Okapi does not agree "
          "with number expected fron Nek! Change n_legendre in the Nek usr file "
          "or the order of each source variable in MultiAppMoonOkapiTransfer!");

      for (unsigned int i = 0; i < num_apps; i++)
        if (_multi_app->hasLocalApp(i))
        {
          if(_dbg)
            _console << "Writing flux BC coeffs from Okapi to MOON..." << std::endl;

          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            auto & soln_values = source_variables[i]->sln();
            if (_dbg) _console << "Fourier order " << i << ": ";
            for (auto j = beginIndex(soln_values); j < source_var_size; ++j)
            {
              if (_dbg) _console << soln_values[j] << ' ';
              // store the coefficients in Nek5000 arrays.
              Nek5000::expansion_fcoef_.coeff_fij[i*100+j] = soln_values[j];
            }
            if (_dbg) _console << '\n';
          }
        }
      // recosntruct a continuous heat flux on the wall boundary
      FORTRAN_CALL(Nek5000::flux_reconstruction)();
      break;
    }

    // MOON -> Okapi. This direction is used to transfer coefficients for a
    // temperature BC, fluid density, and fluid temperature. Currently only
    // the temperature BC and fluid temperature are passed.
    case FROM_MULTIAPP:
    {
      // expand the surface heat flux into coefficients.
      FORTRAN_CALL(Nek5000::nek_expansion)();

      // compute the bins of axially-averaged fluid temperature
      FORTRAN_CALL(Nek5000::axially_binned_integration)();

      // Initialize all of the variables we'll write to store the temp BC
      std::vector<MooseVariableScalar *> to_variables(num_vars_to_write);
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid,
          _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      // Check that all of the variables to write are of the same order
      int write_var_size = to_variables[beginIndex(_to_aux_names)]->sln().size();
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        if (to_variables[i]->sln().size() != write_var_size)
          mooseError("The order of the variables to be written for the "
            "MultiAppMoonOkapiTransfer are not all the same!");
      }

      // Check that the number of variables matches the number of Fourier
      // terms used in Nek, and that the length of each variable matches the
      // number of Legendre terms in Nek. In general, m_fourier and n_legendre
      // could be set in an external code to reduce redunant information
      // specification by the user, but care has to be taken that the values are
      // set before they are needed.
      if (num_vars_to_write != Nek5000::expansion_tdata_.m_fourier)
        mooseError("Number of Fourier polynomials in Okapi does not agree "
          "with number expected from Nek! Change m_fourier in the Nek usr file "
          "or the number of write variables in MultiAppMoonOkapiTransfer!");
      if (write_var_size != Nek5000::expansion_tdata_.n_legendre)
        mooseError("Number of Legendre polynomials in Okapi does not agree "
          "with number expected fron Nek! Change n_legendre in the Nek usr file "
          "or the order of every write variable in MultiAppMoonOkapiTransfer!");

      // Write temp BC coefficients from MOON to Okapi
      if(_dbg)
        _console << "Writing temp BC coeffs from MOON to Okapi..." << std::endl;
      for (auto i = beginIndex(_to_aux_names); i < num_vars_to_write; ++i)
      {
        std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();
        auto & soln_values = to_variables[i]->sln();
        if (_dbg) _console << "Fourier order " << i << ": ";
        for (auto j = beginIndex(soln_values); j < write_var_size; ++j)
        {
          if (_dbg) _console << Nek5000::expansion_tcoef_.coeff_tij[i*100+j] << ' ';
          to_variables[i]->sys().solution().set(dof[j],
            Nek5000::expansion_tcoef_.coeff_tij[i*100+j]);
        }
        if (_dbg) _console << '\n';
        to_variables[i]->sys().solution().close();
      }

      // Write axially-binned fluid average temperatures from MOON to Okapi
      if(_dbg)
        _console << "Writing axially-binned fluid temperature from MOON to Okapi..."
          << "(" << Nek5000::layer_data_.n_layer << " bins)" << std::endl
          << "Temperatures: " << std::endl;
      for (int i = 0; i < Nek5000::layer_data_.n_layer; ++i)
      {
        if (_dbg) _console << Nek5000::fluid_bins_.fluid_temp_bins[i]
          << ' ';
        // TODO: use the axial averages in OpenMC by calling some OpenMC routine
      }
      if (_dbg) _console << std::endl;

      break;
    }
  }

  _console << "Finished MultiAppMoonOkapiTransfer" << name() << std::endl;
}
