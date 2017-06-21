# MOOSE Requires a mesh and a variable even if they are not used.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./l_0_coeffs]
    family = SCALAR
    order = SIXTH
  [../]
  [./l_0_coeffs_temp_bison]
    family = SCALAR
    order = SIXTH
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient
[]

