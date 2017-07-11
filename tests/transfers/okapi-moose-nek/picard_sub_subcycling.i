# Minimal Nek-wrapped MOOSE application.
# MOOSE Requires a mesh and a variable even
# if they are not used.

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
