[GlobalParams]
  legendre_function = legendre
  fourier_function = fourier
  zernike_function = zernike
  l_direction = 2
  diffusion_coefficient_name = 'thermal_conductivity'
  volume_pp = 'volume'
  dbg = false
[]

[Mesh]
  file = 3D_sideset.exo
  block_id = '1'
  block_name = 'interior'
  boundary_id = '100 200 300'
  boundary_name = 'top bottom wall'
[]

[Functions]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]
  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = '0.0 0.0'
  [../]

  # ---- Reconstruction of the continuous kappa-fission dist ---- #
  [./kappa_fission_reconstruction]
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 5
    poly_coeffs = 'l_0_coeffs_kappa_fission'
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./keff]
    family = SCALAR
    order = FIRST
  [../]

  [./l_0_coeffs_kappa_fission] # where kappa-fission coefficients are received
    family = SCALAR
    order = TWENTYFIRST
  [../]

  # ---- Fuel temperature coefficients. Until Z-L expansion are ----#
  #      available in OpenMC, no higher Legendre orders should      #
  #      be used. Note that only the C_{000} coefficient is used.   #
  [./l_0_coeffs_temp] # where temperature coefficients are placed
    family = SCALAR
    order = THIRD
  [../]

  [./kappa_fission] # holds eV/particle field from OpenMC
  [../]

  [./fission_heat] # holds the fission heat source
  [../]
[]

[AuxKernels]
  [./kappa_fisson]
    type = FunctionAux
    variable = kappa_fission
    function = kappa_fission_reconstruction
  [../]
  [./fission_heat]
    type = KappaFissionToHeatSource
    variable = fission_heat
    kappa_fission_source = kappa_fission
    one_group_PKE = true
    keff = keff
    power = 15
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = temp
  [../]
  [./time]
    type = TimeDerivative
    variable = temp
  [../]
  [./source]
    type = CoupledForce
    variable = temp
    v = fission_heat
  [../]
[]

[ICs]
  [./keff]
    type = ScalarComponentIC
    variable = keff
    values = '1.0'
  [../]
[]

[BCs]
  [./temp]
    type = DirichletBC
    variable = temp
    value = 900.0
    boundary = 'wall'
  [../]
  # ---- Insulated on top and bottom ---- #
  [./flux]
    type = NeumannBC
    variable = temp
    boundary = 'top bottom'
    value = 0.0
  [../]
[]

[UserObjects]
  [./l_0_temp_coeffs]
    type = ZLDeconstruction
    variable = temp
    l_order = 0
    n_order = 1
    aux_scalar_name = 'l_0_coeffs_temp'
  [../]
[]

[Materials]
  [./thermal_conductivity]
    type = GenericConstantMaterial
    value = 1.0
  [../]
[]

[Executioner]
  type = Transient
  nl_rel_tol = 1e-6
  l_tol = 1e-6
  dt = 0.02
[]

[Postprocessors]
  [./volume]
    type = VolumePostprocessor
    block_id = '1'
    execute_on = timestep_begin
  [../]
[]

[Outputs]
  exodus = true
[]

