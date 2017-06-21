# This input file is used by the Master App. With this configuration, 
# the Master App does not perform any actual solves - it just facilitates
# solution transfers.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[AuxVariables]
  [./l_0_coeffs_power]
    family = SCALAR
    order = SIXTH
  [../]
  [./l_0_coeffs_temp]
    family = SCALAR
    order = SIXTH
  [../]
[]

# wouldn't normally have ICs - this is only because we don't have 
# transfers from OpenMC and BISON _to_ the Master App
[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_power'
    values = '1.0 2.0 3.0 4.0 5.0 6.0'
  [../]
  [./ic2]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_temp'
    values = '100.0 200.0 300.0 400.0 500.0 600.0'
  [../]
[]

[Variables]
  [./dummy]
  [../]
[]

[Functions]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = dummy
  [../]
  [./dummy]
    type = Diffusion
    variable = dummy
  [../]
[]

[BCs]
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[MultiApps]
  [./openmc]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'picard.i'
    execute_on = timestep_begin
  [../]
  [./bison]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'bison.i'
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./to_bison]
    type = PolynomialOpenMC
    multi_app = bison
    direction = to_multiapp
    source_variable = 'l_0_coeffs_power'
    to_aux_scalar = 'l_0_coeffs'
    execute_on = timestep_end
  [../]
  [./to_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = to_multiapp
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'dummy_openmc'
    execute_on = timestep_begin
  [../]
  [./from_bison]
    type = PolynomialOpenMC
    multi_app = bison
    direction = from_multiapp
    source_variable = 'l_0_coeffs_temp_bison'
    to_aux_scalar = 'l_0_coeffs_temp'
    execute_on = timestep_end
  [../]
  [./from_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = from_multiapp
    source_variable = 'dummy_openmc'
    to_aux_scalar = 'l_0_coeffs_power'
    execute_on = timestep_begin
  [../]
[]

[Postprocessors]
[]

[Outputs]
    exodus = true
[]
