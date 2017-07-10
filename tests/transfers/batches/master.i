# This input file is used by the Master App (MOOSE). Note that this test
# should use cross sections from nndc_hdf5 because I have not included
# more temperature-dependent cross sections.

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
    order = FIRST
    family = SCALAR
  [../]
[]

[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs'
    values = '35.0'
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
  
  [./W_function]
    type = ParsedFunction
    value = 'x+y'
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
  type = Transient
  num_steps = 3
[]

[MultiApps]
  [./openmc]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'picard.i'
  [../]
  [./pronghorn]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'pronghorn.i'
  [../]
[]

# A successful transfer to OpenMC is made by writing to a global variable
# (n_batches). Deactivating this transfer causes the number of batches in
# the settings.xml file to be used - otherwise, the number specified in
# l_0_coeffs is used.
[Transfers]
  [./to_openmc]
    type = MultiAppOkapiMooseTransfer
    multi_app = openmc
    direction = to_multiapp
    source_variable = 'l_0_coeffs'
    to_aux_scalar = 'dummy'
  [../]
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
