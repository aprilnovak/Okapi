# This input file performs a BISON solve over the fuel pin.

[Mesh]
  file = 3D_sideset.exo
  block_id = '1'
  block_name = 'interior'
  boundary_id = '100 200 300'
  boundary_name = 'top bottom wall'
[]

[Functions]
  [./fourier]
    type = FourierPolynomial
    center = '0.0 0.0'
    dbg = false
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
    dbg = false
  [../]
  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = '0.0 0.0'
    dbg = false
  [../]
  [./kappa_fission_reconstruction]
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 1
    l_direction = 2
    legendre_function = legendre
    zernike_function = zernike
    poly_coeffs = 'l_0_coeffs_kappa_fission'
    dbg = false
  [../]

  # ---- Reconstruction of the surface temperature BC from Nek ---- #
  #      Note that the l_order and f_order are interpreted          #
  #      as the order, and should be each one smaller than          #
  #      the number of polynomials.                                 #
  [./temp_BC_reconstruction]
    type = FourierLegendreReconstruction
    l_order = 10
    f_order = 5
    l_direction = 2
    legendre_function = legendre
    fourier_function = fourier
    poly_coeffs = 'f_0_coeffs_temp_BC_bison f_1_coeffs_temp_BC_bison f_2_coeffs_temp_BC_bison f_3_coeffs_temp_BC_bison f_4_coeffs_temp_BC_bison f_5_coeffs_temp_BC_bison '
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./l_0_coeffs_kappa_fission] # where kappa-fission coefficients are received
    family = SCALAR
    order = THIRD
  [../]

  # ---- where temp BC coefficients are received from Okapi ---- #
  [./f_0_coeffs_temp_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_1_coeffs_temp_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_2_coeffs_temp_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_3_coeffs_temp_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_4_coeffs_temp_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_5_coeffs_temp_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]

  # ---- where heat flux BC coefficients are placed ---- #
  [./f_0_coeffs_flux_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_1_coeffs_flux_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_2_coeffs_flux_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_3_coeffs_flux_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_4_coeffs_flux_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_5_coeffs_flux_BC_bison]
    family = SCALAR
    order = ELEVENTH
  [../]

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
    power = 20
    volume_pp = 'volume'
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

[BCs]
  [./temp]
    type = FunctionDirichletBC
    variable = temp
    function = temp_BC_reconstruction
    boundary = 'wall'
  [../]
[]

[UserObjects]
  [./l_0_temp_coeffs]
    type = ZLDeconstruction
    variable = temp
    l_order = 0
    n_order = 1
    legendre_function = legendre
    zernike_function = zernike
    l_direction = 2
    aux_scalar_name = 'l_0_coeffs_temp'
    volume_pp = 'volume'
  [../]

  # ---- Compute heat flux BC coefficients --- #
  [./f_0_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 9
    f_order = 0
    legendre_function = legendre
    fourier_function = fourier
    l_direction = 2
    aux_scalar_name = 'f_0_coeffs_flux_BC_bison'
    surface_area_pp = 'surface_area'
    diffusion_coefficient_name = 'thermal_conductivity'
    boundary = 'wall'
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
[]

[Postprocessors]
  [./volume]
    type = VolumePostprocessor
    block_id = '1'
    execute_on = timestep_begin
  [../]
  [./surface_area]
    type = AreaPostprocessor
    boundary = 'wall'
    execute_on = timestep_begin
  [../]
[]

[Outputs]
  exodus = true
[]

