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

[Executioner]
  type = NekExecutioner
  [./TimeStepper]
    type = NekTimeStepper
    dt = 0.02
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]
