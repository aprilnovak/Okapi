[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
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
    order = THIRD
  [../]
  [./poly]
  [../]
  [./element_l2_error]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs'
    values = '1.0 1.0 0.0'
  [../]
[]

[AuxKernels]
  [./polyaux]
    type = FunctionAux
    variable = poly
    function = 'reconstruction'
  [../]
  [./l2_error_aux]
    type = ElementL2ErrorFunctionAux
    variable = element_l2_error
    function = comparison
    coupled_variable = poly
  [../]
[]

# When coupling, these aux variables will come from a multiapp, but for testing
# the polynomial reconstruction, just set them to a arbitrary value.

[Functions]
  [./comparison]
    type = ParsedFunction
    value = 2*sqrt(x*x+y*y)*sin(atan(y/x))
  [../]

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
    l_order = 0
    n_order = 1
    l_direction = 2
    legendre_function = legendre
    zernike_function = zernike
    poly_coeffs = 'l_0_coeffs'
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
