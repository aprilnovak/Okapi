# This test computes the Legendre polynomials. Simply change the
# coefficients in the initial conditions block to "pick out" different
# polynomials to verify that they are implemented correctly.

# This test is a Zernike-Legendre expansion over a cylinder, but only a first-
# order expansion is used for the Zernike portion.

[Mesh]
  # journal file is located at tests/transfers/okapi-moose/pin_coupling
  file = cylinder.e
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

[Functions]
  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = '0.0 0.0'
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '-0.5 0.5'
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
  [./sides]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = '1.0'
  [../]
  [./top_bottom]
    type = DirichletBC
    variable = u
    boundary = '2 3'
    value = '2.0'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
    exodus = true
[]
