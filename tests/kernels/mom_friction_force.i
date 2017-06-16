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
  
  [./W_function]
    type = ParsedFunction
    value = 'x+y'
  [../]
[]

[Kernels]
  [./friction1]
    type = MomFrictionForce
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

[Materials]
  [./W]
    type = GenericFunctionMaterial
    prop_names = 'W'
    prop_values = 'W_function'
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
  type = Steady
  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./integral]
    type = ElementL2Error
    variable = rho_u
    function = bc_func1
  [../]
[]

[Outputs]
    exodus = true
  [./console]
    type = Console
    max_rows = 400
  [../]
[]
