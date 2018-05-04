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
    multi_app_object_name = kappa_fission_mutable_series
    openmc_cell = 1
    execute_on = timestep_end
    openmc_tally = 1
  [../]
  [./from_bison]
    type = MultiAppMooseOkapiTransfer
    direction = from_multiapp
    multi_app = bison
    multi_app_object_name = temperature_mutable_series_uo
    openmc_cell = 1
    execute_on = timestep_end
    openmc_tally = 1
  [../]
[]

[Outputs]
  exodus = true
[]
