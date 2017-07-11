[Problem]
  kernel_coverage_check = false
  solve = false
[]

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
  [./f_0_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
[]

[ICs]
  [./f_0_flux_BC]
    type = ScalarComponentIC
    variable = 'f_0_flux_BC'
    values = '1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
  [../]
[]

[Kernels]
active = ''
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
  [./bc_func]
    type = ConstantFunction
    value = 0.0
  [../]
[]

[BCs]
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
    source_variable = 'f_0_flux_BC'
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
