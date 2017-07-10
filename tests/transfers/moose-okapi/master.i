[Problem]
  solve = true
  kernel_coverage_check = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables] # dummy variable
  [./u]
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = u
  [../]
[]

[AuxVariables]
  [./l_0_coeffs_power] # sent from MOOSE to BISON
    family = SCALAR
    order = THIRD
  [../]
  [./l_0_coeffs_temp] # sent from MOOSE to OpenMC
    family = SCALAR
    order = THIRD
  [../]
  [./openmc_kappa_fission] # sent to BISON, already expanded by MOOSE
  [../]
[]

# We only need an initial condition for temperature, since OpenMC executes on
# timestep_begin, and the initial temperatures are set in the Master App
# input file. This will set an initial temperature of 500 K in the
# corresponding cells in OpenMC. Any other temperatures in OpenMC will come
# from the XML files.
[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_temp'
    values = '900.0 0.0 0.0'
  [../]
[]

[Executioner]
  type = OpenMCExecutioner
  num_steps = 3
  [./TimeStepper]
    type = OpenMCTimeStepper
  [../]
[]

[MultiApps]
  [./bison]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'bison.i'
    execute_on = timestep_end
  [../]
[]

[Transfers]
active = ''
  [./to_bison]
    type = MultiAppOkapiMooseTransfer
    multi_app = bison
    direction = to_multiapp
    openmc_cell = 1
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'dummy_openmc'
    execute_on = timestep_begin
    dbg = true
  [../]
  [./from_bison]
    type = MultiAppOkapiMooseTransfer
    multi_app = openmc
    direction = from_multiapp
    openmc_cell = 1
    source_variable = 'dummy_openmc'
    to_aux_scalar = 'l_0_coeffs_power'
    execute_on = timestep_begin
    dbg = true
  [../]
[]

[Outputs]
    exodus = true
[]
