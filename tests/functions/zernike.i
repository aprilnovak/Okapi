[Mesh]
  # cylinder with r = 0.5 and z = (0, 1)
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
  [./l_0_coeffs]
    family = SCALAR
    order = FIRST # this order should match the number of Zernike coefficients for a given Legendre order
    initial_condition = '1.0'
  [../]
[]

# When coupling, these aux variables will come from a multiapp, but for testing
# the polynomial reconstruction, just set them to a arbitrary value.

[Functions]
  [./bc_func] # the analytical form of the currently-tested Zernike-Legendre expansion
    type = ParsedFunction
    value = sqrt(2)
  [../]
  [./forcing] # the needed forcing function
    type = ParsedFunction
    value = 0
  [../]

  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = 0.0
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]

  [./reconstruction] # function for comparing against (full reconstruction)
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 0
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
  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing
  [../]
[]

[BCs]
  [./u]
    type = FunctionDirichletBC
    variable = u
    boundary = 'top bottom wall'
    function = bc_func
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./integral]
    type = ElementL2Error
    variable = u
    function = reconstruction
  [../]
[]

[Outputs]
    exodus = true
  [./console]
    type = Console
    max_rows = 400
  [../]
[]
