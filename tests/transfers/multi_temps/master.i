# This input file is used by the Master App (MOOSE)

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

[AuxVariables]
  [./l_0_coeffs]
    order = SIXTH
    family = SCALAR
  [../]
  [./l_1_coeffs]
    order = SIXTH
    family = SCALAR
  [../]
[]

[ICs]
  [./ic1]
    type = ScalarComponentIC
    variable = 'l_0_coeffs'
    values = '300 400 500 600 700 800'
  [../]
  [./ic2]
    type = ScalarComponentIC
    variable = 'l_1_coeffs'
    values = '350 450 550 650 750 850'
  [../]
[]

[Variables]
  [./rho_u]
  [../]
[]

[Functions]
  [./bc_func1] # the assumed solution for rho_u
    type = ParsedFunction
    value = exp(x)
  [../]

  [./forcing1] # the needed forcing function
    type = ParsedFunction
    value = (x+y)*exp(x)-exp(x)
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = rho_u
  [../]
  [./dummy1]
    type = Diffusion
    variable = rho_u
  [../]
  [./forcing1]
    type = UserForcingFunction
    variable = rho_u
    function = forcing1
  [../]
[]

[BCs]
  [./rho_u]
    type = FunctionDirichletBC
    variable = rho_u
    boundary = 'bottom top right left'
    function = bc_func1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[MultiApps]
  [./openmc]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'picard.i'
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./to_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = to_multiapp
    source_variable = 'l_0_coeffs l_1_coeffs'
    to_aux_scalar = 'dummy'
    execute_on = timestep_end
    center = '0.5 0.5'
    radius = '0.5'
    l_geom_norm = '0 1'
    l_direction = 2
  [../]
[]

[Outputs]
    exodus = true
[]
