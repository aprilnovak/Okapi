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
  [./dummy]
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
[]
