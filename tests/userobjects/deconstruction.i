# This test verifies the decomposition of a continuous variable u 
# into expansion coefficients.

[GlobalParams]
  legendre_function = legendre
  zernike_function = zernike
  l_direction = 2
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 25
  ny = 25
  nz = 5
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 1.0
[]

[Variables]
  [./u]
  [../]
[]

# The order of the scalar variable should match the number of Zernike coefficients
# for a given Legendre order.
[AuxVariables]
  [./l_0_coeffs]
    family = SCALAR
    order = SIXTH
  [../]
[]

[Functions]
  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = '0.5 0.5'
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./dir]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = '1.0'
  [../]
  [./dir2]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = '2.0'
  [../]
[]

[UserObjects]
  [./l_0]
    type = ZernikeLegendreDeconstruction
    variable = u
    volume_pp = vol
    l_order = 0
    n_order = 1
    m_order = 1
    aux_scalar_name = 'l_0_coeffs'
  [../]
[]

[Postprocessors]
  [./vol]
    type = VolumePostprocessor
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
    exodus = true
[]
