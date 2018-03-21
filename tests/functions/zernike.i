# This test computes the Zernike polynomials. Simply change the
# coefficients in the initial conditions block to "pick out" different
# polynomials to verify that they are implemented correctly.

# This test is a test of the functions in the
# aprilnovak/moose/fe-module-deprecated branch, and once transitioning to the
# new MOOSE FE module, this test should be deleted. This test serves as a
# basis for validating the new MOOSE FE modules ability to apply a function
# obtained from an expansion to an auxvariable.

[Mesh]
  # journal file is located at tests/transfers/okapi-moose/pin_coupling
  file = cylinder.e
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
  [./poly]
  [../]
[]

[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs'
    values = '0.0 0.0 0.0 0.0 0.0 1.0'
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
    center = '0.0 0.0'
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]

  [./reconstruction] # function for comparing against (full reconstruction)
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 2
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
  [./walls]
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
