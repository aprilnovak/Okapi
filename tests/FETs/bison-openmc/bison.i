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
  [./l_0_coeffs_temp_bison] # send to l_0_coeffs_temp
    family = SCALAR
    order = THIRD
  [../]
  [./bison_power]
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

