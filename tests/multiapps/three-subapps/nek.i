[Mesh]
  file = 3D_sideset.exo
  block_id = '1'
  block_name = 'interior'
  boundary_id = '100 200 300'
  boundary_name = 'top bottom wall'
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./aux]
  [../]
[]

[Executioner]
  type = NekExecutioner
  num_steps = 20
  [./TimeStepper]
    type = NekTimeStepper
    dt = 0.001
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]
