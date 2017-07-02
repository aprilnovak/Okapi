# This input file performs a BISON solve over the fuel pin.

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
  [./l_0_coeffs_power_bison] # receive from l_0_coeffs_power
    family = SCALAR
    order = SIXTH
  [../]
  [./l_0_coeffs_temp_bison] # send temp back to MOOSE
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

