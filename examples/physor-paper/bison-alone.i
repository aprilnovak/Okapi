# physical units are assumed to be in cm

[GlobalParams]
  legendre_function = legendre
  fourier_function = fourier
  zernike_function = zernike
  l_direction = 2
  surface_area_pp = 'surface_area'
  diffusion_coefficient_name = 'thermal_conductivity'
  one_group_PKE = false
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
  [./fourier]
    type = FourierPolynomial
    center = '0.0 0.0'
  [../]
  [./legendre]
    type = LegendrePolynomial
    l_geom_norm = '0.0 1.0'
  [../]
  [./zernike]
    type = ZernikePolynomial
    radius = 0.5
    center = '0.0 0.0'
  [../]

  # ---- Reconstruction of the continuous kappa-fission dist   ---- #
  [./kappa_fission_reconstruction]
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 6
    poly_coeffs = 'l_0_coeffs_kappa_fission'
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[ICs]
  [./kappa_fission]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_kappa_fission'
    values = '4.40957e+07 476.782 6340.24 -3032.74 607185 -13667.6 -744.07 2382.4 -7489.64 -12032.4 -4483.44 10026.5 75189.1 -19403.2 -90253.7 -15540.7 -10443 -10019.8 1330.12 -4187.51 3721.4 -1521.26 -1794.36 -4862.64 17360.5 10998.8 10551.8 1365.19'

[AuxVariables]
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

[BCs]
  # ---- Apply the temperature BC received from Nek ---- #
  [./temp]
    type = DirichletBC
    variable = temp
    value = 600.0
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

  # ---- Compute heat flux BC coefficients for each Fourier order ---- #
  [./f_0_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 10
    f_order = 0
    aux_scalar_name = 'f_0_coeffs_flux_BC_bison'
    boundary = 'wall'
  [../]
  [./f_1_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 10
    f_order = 1
    aux_scalar_name = 'f_1_coeffs_flux_BC_bison'
    boundary = 'wall'
  [../]
  [./f_2_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 10
    f_order = 2
    aux_scalar_name = 'f_2_coeffs_flux_BC_bison'
    boundary = 'wall'
  [../]
  [./f_3_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 10
    f_order = 3
    aux_scalar_name = 'f_3_coeffs_flux_BC_bison'
    boundary = 'wall'
  [../]
  [./f_4_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 10
    f_order = 4
    aux_scalar_name = 'f_4_coeffs_flux_BC_bison'
    boundary = 'wall'
  [../]
  [./f_5_coeffs_flux_BC]
    type = FLDeconstruction
    flux_integral = true
    variable = temp
    l_order = 10
    f_order = 0
    legendre_function = legendre
    fourier_function = fourier
    aux_scalar_name = 'f_5_coeffs_flux_BC_bison'
    boundary = 'wall'
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
  [./surface_area]
    type = AreaPostprocessor
    boundary = 'wall'
    execute_on = timestep_begin
  [../]
[]

[Outputs]
  exodus = true
[]

