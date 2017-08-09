# This test computes the Legendre polynomials. Simply change the
# coefficients in the initial conditions block to "pick out" different
# polynomials to verify that they are implemented correctly.

# This test is a test of the functions in my local branch of MOOSE that
# has specific coupling features, so this test will only work if Aprils
# "coupling" branch of MOOSE is being used. But, this test can be used
# to verify the Functional Expansion module when it is completed.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 30
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
# for a given Legendre order. To test only the Legendre coefficients, this should
# be first order, since Z_0^0 is a constant.
[AuxVariables]
  [./l_0_coeffs]
    family = SCALAR
    order = FIRST
  [../]
  [./l_1_coeffs]
    family = SCALAR
    order = FIRST
  [../]
  [./l_2_coeffs]
    family = SCALAR
    order = FIRST
  [../]
  [./poly]
  [../]
[]

[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs'
    values = '0.0'
  [../]
  [./ic2]
    type = ScalarComponentIC
    variable = 'l_1_coeffs'
    values = '0.0'
  [../]
  [./ic3]
    type = ScalarComponentIC
    variable = 'l_2_coeffs'
    values = '1.0'
  [../]
[]

[AuxKernels]
  [./polyaux]
    type = FunctionAux
    variable = poly
    function = 'reconstruction'
  [../]
[]

# When coupling, these aux variables will come from a multiapp, but for testing
# the polynomial reconstruction, just set them to a arbitrary value.

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

  [./reconstruction] # function for comparing against (full reconstruction)
    type = ZernikeLegendreReconstruction
    l_order = 2
    n_order = 0
    l_direction = 2
    legendre_function = legendre
    zernike_function = zernike
    poly_coeffs = 'l_0_coeffs l_1_coeffs l_2_coeffs'
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

[Executioner]
  type = Steady
[]

[Outputs]
    exodus = true
[]
