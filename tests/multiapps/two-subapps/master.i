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
  [./l_0_coeffs_temp] # For now, just change the average rod temp in OpenMC
    family = SCALAR
    order = FIRST
  [../]
[]

# We only need an initial condition for temperature, since OpenMC executes on
# timestep_begin, and the initial temperatures are set in the Master App input file.
# This will set an initial temperature of 400 K in the corresponding cells in OpenMC.
# Any other temperatures in OpenMC will come from the XML files.
[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_temp'
    values = '500.0'
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
    input_files = 'openmc.i'
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
    type = MultiAppOkapiMooseTransfer
    multi_app = bison
    direction = to_multiapp
    source_variable = 'l_0_coeffs_power'
    to_aux_scalar = 'l_0_coeffs'
    execute_on = timestep_end
  [../]
  [./to_openmc]
    type = MultiAppOkapiMooseTransfer
    multi_app = openmc
    direction = to_multiapp
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'dummy_openmc'
    execute_on = timestep_begin
    center = '0.5 0.5'
    radius = 0.5
    l_geom_norm = '0.0 1.0'
    l_direction = 2
  [../]

  [./from_bison]
    type = MultiAppOkapiMooseTransfer
    multi_app = bison
    direction = from_multiapp
    source_variable = 'l_0_coeffs_temp_bison'
    to_aux_scalar = 'l_0_coeffs_temp'
    execute_on = timestep_end
  [../]
  [./from_openmc]
    type = MultiAppOkapiMooseTransfer
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
