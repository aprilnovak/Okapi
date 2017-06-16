# This test verifies the decomposition of a continuous variable v
# into expansion coefficients.

[GlobalParams]
  legendre_function = legendre
  zernike_function = zernike
  l_direction = 2
  volume_pp = vol
[]

[Mesh]
  file = 3D_sideset.exo
#  type = GeneratedMesh
#  dim = 3
#  nx = 5
#  ny = 5
#  nz = 5
#  xmin = 0.0
#  xmax = 1.0
#  ymin = 0.0
#  ymax = 1.0
#  zmin = 0.0
#  zmax = 1.0
  block_id = '1'
  boundary_id = '100 200 300'
  boundary_name = 'top bottom wall'
[]

[Variables]
  [./u]
  [../]
[]

# The order of the scalar variable should match the number of Zernike coefficients
# for a given Legendre order.
[AuxVariables]
  [./l_0_coeffs_start] # starting coefficients - used in construction
    family = SCALAR
    order = SIXTH
  [../]
  [./l_0_coeffs_end] # ending coefficients - used to store deconstruction
    family = SCALAR
    order = SIXTH
  [../]
  [./v]
  [../]
[]

[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_start'
    values = '100.0 200.0 300.0 400.0 500.0 600.0'
  [../]
[]

[Functions]
  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = '0.0 0.0'
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]
  [./reconstruction]
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 2
    poly_coeffs = 'l_0_coeffs_start'
  [../]
[]

[AuxKernels]
  [./analytical]
    type = FunctionAux
    variable = v
    function = 'reconstruction'
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
    boundary = 'top'
    value = '1.0'
  [../]
  [./dir2]
    type = DirichletBC
    variable = u
    boundary = 'bottom'
    value = '2.0'
  [../]
[]

[UserObjects]
  [./z00]
    type = ZernikeLegendreDeconstruction
    variable = v
    l_order = 0
    n_order = 0
    m_order = 0
    aux_scalar_name = 'l_0_coeffs_end'
  [../]
  [./z1n1]
    type = ZernikeLegendreDeconstruction
    variable = v
    l_order = 0
    n_order = 1
    m_order = -1
    aux_scalar_name = 'l_0_coeffs_end'
  [../]
  [./z11]
    type = ZernikeLegendreDeconstruction
    variable = v
    l_order = 0
    n_order = 1
    m_order = 1
    aux_scalar_name = 'l_0_coeffs_end'
  [../]
  [./z2n2]
    type = ZernikeLegendreDeconstruction
    variable = v
    l_order = 0
    n_order = 2
    m_order = -2
    aux_scalar_name = 'l_0_coeffs_end'
  [../]
  [./z20]
    type = ZernikeLegendreDeconstruction
    variable = v
    l_order = 0
    n_order = 2
    m_order = 0
    aux_scalar_name = 'l_0_coeffs_end'
  [../]
  [./z22]
    type = ZernikeLegendreDeconstruction
    variable = v
    l_order = 0
    n_order = 2
    m_order = 2
    aux_scalar_name = 'l_0_coeffs_end'
  [../]
[]

[Postprocessors]
  [./vol]
    type = VolumePostprocessor
    block_id = '1'
    execute_on = timestep_begin
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
    exodus = true
[]
