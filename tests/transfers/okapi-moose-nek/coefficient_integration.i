#
# Testing a solution that is second order in space and first order in time
#

[Mesh]
  # This is a cylinder with r=0.5, z=(0,1)
  file = 3D_sideset.exo
  block_id = '1'
  block_name = 'interior'
  boundary_id = '100 200 300'
  boundary_name = 'top bottom wall'
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./heat_flux_scalar_f_0_l]
    family = SCALAR
    order = TENTH
  [../]
  [./heat_flux_scalar_f_1_l]
    family = SCALAR
    order = TENTH
  [../]
  [./heat_flux_scalar_f_2_l]
    family = SCALAR
    order = TENTH
  [../]
  [./heat_flux_scalar_f_3_l]
    family = SCALAR
    order = TENTH
  [../]
  [./heat_flux_scalar_f_4_l]
    family = SCALAR
    order = TENTH
  [../]
  [./temp_bc_scalar_f_0_l]
    family = SCALAR
    order = TENTH
  [../]
  [./temp_bc_scalar_f_1_l]
    family = SCALAR
    order = TENTH
  [../]
  [./temp_bc_scalar_f_2_l]
    family = SCALAR
    order = TENTH
  [../]
  [./temp_bc_scalar_f_3_l]
    family = SCALAR
    order = TENTH
  [../]
  [./temp_bc_scalar_f_4_l]
    family = SCALAR
    order = TENTH
  [../]
[]

[ICs]
  [./v_ic]
    type = ScalarComponentIC
    variable = 'temp_bc_scalar_f_0_l'
    values = '1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
  [../]
  [./temp_ic]
    type = FunctionIC
    variable = temp
    function = '0.0'
  [../]
[]

[Kernels]
  [./Diffusion]
    type = Diffusion
    variable = temp
  [../]
  [./Time]
    type = TimeDerivative
    variable = temp
  [../]
[]

[Functions]
active = 'bc_func'
  # BCFunction just returns 0.0 right now
  [./bc_func]
    type = ConstantFunction
  [../]
  [./legendre_function]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]
  [./fourier_function]
    type = FourierPolynomial
  [../]
  [./fl_reconstruction]
    type = FourierLegendreReconstruction
    l_order = 10
    f_order = 5
    l_direction = 2
    legendre_function_name = 'legendre_function'
    fourier_function_name = 'fourier_function'
    poly_scalars = 'temp_bc_scalar_f_0_l temp_bc_scalar_f_1_l temp_bc_scalar_f_2_l temp_bc_scalar_f_3_l temp_bc_scalar_f_4_l'
  [../]
[]

[BCs]
active = 'wall'
  [./wall1]
    type = FunctionDirichletBC
    variable = temp
    boundary = 'wall'
    function = fl_reconstruction
  [../]
  [./wall]
    type = FunctionDirichletBC
    variable = temp
    boundary = 'wall'
    function = bc_func
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.0'
    block = 'interior'
  [../]
  [./cp]
    type = GenericConstantMaterial
    prop_names = 'specific_heat'
    prop_values = '1.0'
    block = 'interior'
  [../]
  [./rho]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1.0'
    block = 'interior'
  [../]
[]

[Postprocessors]
  [./surf_area]
    type = AreaPostprocessor
    boundary = wall
    execute_on = timestep_begin
  [../]
[]

[Executioner]
  type = Transient
  scheme     = 'Explicit-Euler' # Others available: backward Euler, Crank-Nicholson, etc.
  dt         = 0.001      # Initial timestep size
  start_time = 0        # Starting time
  num_steps  = 5000     # Number of Steps
  nl_rel_tol = 1e-6     # Nonlinear relative tolerance
  l_tol      = 1e-6     # Linear tolerance

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
  print_perf_log = true
[]

[MultiApps]
  [./nek]
    type = TransientMultiApp
    app_type = MoonApp
    sub_cycling = true
    positions = '0 0 0'
    input_files = picard_sub_subcycling.i
    library_path = /homes/anovak/projects/moon/lib
  [../]
[]

[Transfers]
active = 'to_nek'
  [./to_nek]
    type = MultiAppMoonOkapiTransfer
    direction = to_multiapp
    multi_app = nek
    source_variable = 'heat_flux_scalar_f_0_l heat_flux_scalar_f_1_l heat_flux_scalar_f_2_l heat_flux_scalar_f_3_l heat_flux_scalar_f_4_l '
    to_aux_scalar = 'foo'
  [../]

  [./from_nek]
    type = MultiAppMoonOkapiTransfer
    direction = from_multiapp
    multi_app = nek
    source_variable = 'foo'
    to_aux_scalar = 'temp_bc_scalar_f_0_l temp_bc_scalar_f_1_l temp_bc_scalar_f_2_l temp_bc_scalar_f_3_l temp_bc_scalar_f_4_l'
  [../]
[]
