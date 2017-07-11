[GlobalParams]
  dbg = true
[]

[Problem]
#  kernel_coverage_check = false
[]

# We need to define a dummy mesh for the master App, even though it isnt used.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [./bc_func]
    type = ConstantFunction
    value = 0.0
  [../]
  [./fuel_temp_function] # constant initial fuel temp
    type = ParsedFunction
    value = 900.0
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./f_0_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
  [./f_0_temp_BC]
    family = SCALAR
    order = TENTH
  [../]
[]

[ICs]
  [./f_0_flux_BC] # initial condition on the wall heat flux, used by Nek
    type = ScalarComponentIC
    variable = 'f_0_flux_BC'
    values = '1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
  [../]
[]

[AuxKernels]
active = ''
  [./fuel_temp_ic]
    type = FunctionAux
    variable = fuel_temp_continuous_ic
    function = fuel_temp_function
  [../]
[]

[Kernels]
#active = ''
  [./Diffusion]
    type = Diffusion
    variable = temp
  [../]
  [./Time]
    type = TimeDerivative
    variable = temp
  [../]
[]


[BCs]
active = ''
  [./wall]
    type = FunctionDirichletBC
    variable = temp
    boundary = 'wall'
    function = bc_func
  [../]
[]

[Executioner]
  type = OpenMCExecutioner
  num_steps = 5
  [./TimeStepper]
    type = OpenMCTimeStepper
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_perf_log = true
[]

[MultiApps]
  [./nek]
    type = TransientMultiApp
    app_type = MoonApp
    positions = '0 0 0'
    input_files = picard_sub_subcycling.i
    library_path = /homes/anovak/projects/moon/lib
    execute_on = timestep_begin
  [../]
  [./bison]
    type = TransientMultiApp
    app_type = BuffaloApp
    positions = '0 0 0'
    input_files = bison.i
    library_path = /homes/anovak/projects/buffalo/lib
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./to_nek] # transfers heat flux BC
    type = MultiAppMoonOkapiTransfer
    direction = to_multiapp
    multi_app = nek
    source_variable = 'f_0_flux_BC'
    to_aux_scalar = 'foo'
    execute_on = timestep_begin
  [../]
  [./from_nek] # writes temperature BC
    type = MultiAppMoonOkapiTransfer
    direction = from_multiapp
    multi_app = nek
    source_variable = 'foo'
    to_aux_scalar = 'f_0_temp_BC'
    execute_on = timestep_begin
  [../]
  [./to_bison]
    type = MultiAppMooseOkapiTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'bar'
    to_aux_scalar = 'l_0_coeffs_power'
    openmc_cell = 1
    execute_on = timestep_end
  [../]
  [./from_bison]
    type = MultiAppMooseOkapiTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'bar'
    openmc_cell = 1
    execute_on = timestep_end
  [../]
[]
