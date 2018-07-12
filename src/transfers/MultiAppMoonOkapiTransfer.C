#ifdef ENABLE_NEK_COUPLING

#include "MultiAppMoonOkapiTransfer.h"
#include "NekInterface.h"
#include "openmc.h"
#include "OpenMCErrorHandling.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template <>
InputParameters
validParams<MultiAppMoonOkapiTransfer>()
{
  /* This transfer is used to transfer information between Okapi and
     MOON when MOON is the sub App to an Okapi Master App. */
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<VariableName>>(
      "source_variable", "The auxiliary scalar variable(s) to read values from");
  params.addRequiredParam<std::vector<VariableName>>(
      "to_aux_scalar",
      "The name of the scalar Aux variable(s) in the MultiApp to transfer"
      " the value to.");
  params.addRequiredParam<std::vector<int32_t>>(
      "openmc_cell",
      "Cell IDs in OpenMC to transfer axially-binned fluid temperatures to "
      "(ordered from low heights to high heights).");
  params.addRequiredParam<std::vector<int32_t>>(
      "openmc_material",
      "Material IDs in OpenMC to transfer axially-binned fluid densities to "
      "(ordered from low heights to high heights).");
  params.addParam<Real>("inlet_temp",
                        550,
                        "Inlet temperature about which to "
                        "scale the nondimensional Nek5000 temperature results.");
  params.addParam<Real>("outlet_temp",
                        600,
                        "Outlet temperature about which to "
                        "scale the nondimensional Nek5000 temperature results.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information.");
  params.addParam<bool>("store_results",
                        true,
                        "Whether to store the iteration "
                        "results for each Picard iteration");
  return params;
}

MultiAppMoonOkapiTransfer::MultiAppMoonOkapiTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar")),
    _dbg(parameters.get<bool>("dbg")),
    _cell(getParam<std::vector<int32_t>>("openmc_cell")),
    _material(getParam<std::vector<int32_t>>("openmc_material")),
    _T_inlet(getParam<Real>("inlet_temp")),
    _T_outlet(getParam<Real>("outlet_temp")),
    _store_results(getParam<bool>("store_results"))
{
  _index.resize(_cell.size());
  _index_mat.resize(_material.size());

  if (_cell.size() != _material.size())
    mooseError("Number of cells and material do not match for the "
               "'MultiAppMoonOkapiTransfer'!");
}

void
MultiAppMoonOkapiTransfer::execute()
{
  _console << "Beginning MultiAppMoonOkapiTransfer " << name() << std::endl;

  unsigned int num_apps = _multi_app->numGlobalApps();
  unsigned int num_vars_to_read = _source_var_names.size();
  unsigned int num_vars_to_write = _to_aux_names.size();

  // get the index of the cell in the cells(:) OpenMC array to be used
  // for later calls to cell-dependent OpenMC routines. Likewise, get the
  // indices of the materials in those cells.
  for (std::size_t i = 0; i < _cell.size(); ++i)
  {
    int err_index = openmc_get_cell_index(_cell[i], &_index[i]);
    ErrorHandling::openmc_get_cell_index(err_index, "MultiAppMoonOkapiTransfer");

    int err_index_mat = openmc_get_material_index(_material[i], &_index_mat[i]);
    ErrorHandling::openmc_get_material_index(err_index_mat, "MultiAppMoonOkapiTransfer");
  }

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
        source_variables[i] = &from_problem.getScalarVariable(_tid, _source_var_names[i]);
        source_variables[i]->reinit();
      }

      // Check that the order of each of the source variables is the same by
      // comparing with the size of the first variable.
      int source_var_size = source_variables[beginIndex(_source_var_names)]->sln().size();
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
          if (_dbg)
            _console << "Writing flux BC coeffs from Okapi to MOON..." << std::endl;

          for (auto i = beginIndex(_source_var_names); i < num_vars_to_read; ++i)
          {
            auto & soln_values = source_variables[i]->sln();
            if (_dbg)
              _console << "Fourier order " << i << ": ";
            for (auto j = beginIndex(soln_values); j < source_var_size; ++j)
            {
              if (_dbg)
                _console << soln_values[j] << ' ';

              // store the coefficients in Nek5000 arrays.
              Nek5000::expansion_fcoef_.coeff_fij[i * 100 + j] = soln_values[j];
            }
            if (_dbg)
              _console << '\n';
          }
        }

      // recosntruct a continuous heat flux on the wall boundary
      FORTRAN_CALL(Nek5000::flux_reconstruction)();
      break;
    }

    // MOON -> Okapi. This direction is used to transfer coefficients for a
    // temperature BC, fluid density, and fluid temperature.
    case FROM_MULTIAPP:
    {
      // expand the surface heat flux into coefficients.
      FORTRAN_CALL(Nek5000::nek_expansion)();

      // compute the bins of axially-averaged fluid temperature
      FORTRAN_CALL(Nek5000::axially_binned_integration)();

      // Initialize all of the variables we'll write to store the temp BC
      std::vector<MooseVariableScalar *> to_variables(num_vars_to_write);
      for (unsigned int i = 0; i < num_vars_to_write; ++i)
      {
        to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      // Check that all of the variables to write are of the same order
      int write_var_size = to_variables[beginIndex(_to_aux_names)]->sln().size();
      for (unsigned int i = 0; i < num_vars_to_write; ++i)
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

      // Check that the number of OpenMC cells provided for transferring axial
      // fluid temperatures matches the number of layers in Nek5000
      if (_cell.size() != Nek5000::layer_data_.n_layer)
        mooseError("Number of OpenMC cells provided for transferring fluid "
                   "temperatures does not match the number of layers computed in Nek! "
                   "Check n_layer in the Nek usr file or the number of entries in "
                   "'openmc_cell'!");

      // Write temp BC coefficients from MOON to Okapi
      if (_dbg)
        _console << "Writing temp BC coeffs from MOON to Okapi..." << std::endl;

      for (unsigned int i = 0; i < num_vars_to_write; ++i)
      {
        std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();
        auto & soln_values = to_variables[i]->sln();
        if (_dbg)
          _console << "Fourier order " << i << ": ";

        for (auto j = beginIndex(soln_values); j < write_var_size; ++j)
        {
          if (_dbg)
            _console << Nek5000::expansion_tcoef_.coeff_tij[i * 100 + j] << ' ';
          to_variables[i]->sys().solution().set(dof[j],
                                                Nek5000::expansion_tcoef_.coeff_tij[i * 100 + j]);
        }
        if (_dbg)
          _console << '\n';
        to_variables[i]->sys().solution().close();
      }

      // Write axially-binned fluid average temperatures from MOON to Okapi
      if (_dbg)
        _console << "Writing axially-binned fluid temp from MOON to Okapi..."
                 << "(" << Nek5000::layer_data_.n_layer << " bins)" << std::endl
                 << "Temperatures: " << std::endl;

      double layer_temps[_cell.size()];
      for (std::size_t i = 0; i < _cell.size(); ++i)
      {
        if (_dbg)
          _console << Nek5000::fluid_bins_.fluid_temp_bins[i] << ' ';

        // apply the scaling factor
        layer_temps[i] =
            Nek5000::fluid_bins_.fluid_temp_bins[i] * (_T_outlet - _T_inlet) + _T_inlet;
      }
      if (_dbg)
        _console << std::endl;

      if (_store_results)
      {
        std::vector<Real> this_iteration;
        for (std::size_t i = 0; i < _cell.size(); ++i)
          this_iteration.push_back(layer_temps[i]);

        _fluid_layer_temps.push_back(this_iteration);

        _console << "Fluid layer temps up to iteration " << _fluid_layer_temps.size() << ":"
                 << std::endl;
        for (std::size_t i = 0; i < _fluid_layer_temps.size(); ++i)
          printResults(_fluid_layer_temps[i]);
      }

      // manually change the temperatures of each fluid layer in OpenMC. This
      // will be changed in the future when we implement continuous temperature
      // tracking so that the user doesn't need to define these cells in the
      // geometry XML file.
      if (_dbg)
        _console << "Layer densities: " << std::endl;
      for (std::size_t i = 0; i < _cell.size(); ++i)
      {
        int err_set = openmc_cell_set_temperature(_index[i], layer_temps[i], NULL);
        ErrorHandling::openmc_cell_set_temperature(err_set);

        // simple correlation for water at 15 MPa, converted to g/cm^3
        double layer_density =
            -0.0097 * layer_temps[i] * layer_temps[i] + 8.8796 * layer_temps[i] - 1167.1;
        layer_density /= 100.0;

        // convert to atoms/barn-cm as required by OpenMC
        layer_density = layer_density * (6.022E23) / (18.01588 * 1E24);

        if (_dbg)
          _console << layer_density << ' ';

        int err_set_density = openmc_material_set_density(_index[i], layer_density);
        ErrorHandling::openmc_material_set_density(err_set_density);
      }

      break;
    }
  }

  _console << "Finished MultiAppMoonOkapiTransfer" << name() << std::endl;
}

void
MultiAppMoonOkapiTransfer::printResults(std::vector<Real> & results)
{
  for (unsigned int i = 0; i < results.size(); ++i)
    _console << results[i] << ", ";

  _console << std::endl;
}

#endif // ENABLE_NEK_COUPLING
