[GlobalParams]
  dbg = true
[]

[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
  [../]
[]

[Executioner]
  type = OpenMCExecutioner
  num_steps = 2
  [./TimeStepper]
    type = OpenMCTimeStepper
    dt = 0.01
  [../]
[]

[MultiApps]
  [./bison]
    type = TransientMultiApp
    app_type = BuffaloApp
    positions = '0 0 0'
    input_files = bison.i
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./to_bison]
    type = MultiAppMooseOkapiTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'bar'
    to_aux_scalar = 'l_0_coeffs_kappa_fission'
    openmc_cell = 1
    execute_on = timestep_end
  [../]
  [./from_bison]
    type = MultiAppMooseOkapiTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'l_0_coeffs_temp l_1_coeffs_temp l_2_coeffs_temp'
    to_aux_scalar = 'bar'
    openmc_cell = 1
    execute_on = timestep_end
  [../]
[]

[Outputs]
  exodus = true
[]
