# MOOSE Requires a mesh and a variable even if they are not used.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
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
  type = OpenMCExecutioner
  num_steps = 20
  [./TimeStepper]
    type = OpenMCTimeStepper
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]
