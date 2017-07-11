# This input file performs a BISON solve over the fuel pin.

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
[]

[Variables]
  [./bison_temp]
  [../]
[]

[AuxVariables]
  [./l_0_coeffs_kappa_fission]
    family = SCALAR
    order = THIRD
  [../]
  [./l_0_coeffs_temp]
    family = SCALAR
    order = THIRD
  [../]
  [./kappa_fission] # holds eV/particle field from OpenMC
  [../]
  [./bison_heat] # holds the heat source used in kernel
  [../]
[]

[AuxKernels]
  [./kappa_fisson]
    type = FunctionAux
    variable = kappa_fission
    function = kappa_fission_reconstruction
  [../]
  #[./bison_power]
  #  type = KappaFissionToHeatSource
  #  variable = bison_heat
  #  kappa_fission_source = bison_kappa_fission
  #  power = 20
  #  kappa_fission_pp = 'kappa_fission'
  #  volume_pp = 'volume'
  #[../]
[]

[Kernels]
active = 'diffusion time'
  [./diffusion]
    type = Diffusion
    variable = bison_temp
  [../]
  [./time]
    type = TimeDerivative
    variable = bison_temp
  [../]
  [./source]
    type = CoupledForce
    variable = bison_temp
    v = bison_heat
  [../]
[]

[BCs]
  [./constant_temp]
    type = DirichletBC
    variable = bison_temp
    boundary = 'wall'
    value = 673 # Kelvin
  [../]
[]

[UserObjects]
active = ''
  [./l0]
    type = ZLDeconstruction
    variable = bison_temp
    l_order = 0
    n_order = 1
    legendre_function = legendre
    zernike_function = zernike
    l_direction = 2
    aux_scalar_name = 'l_0_coeffs_temp_bison'
    volume_pp = 'volume'
  [../]
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
active = 'volume'
  [./volume]
    type = VolumePostprocessor
    block_id = '1'
    execute_on = timestep_begin
  [../]
  [./kappa_fission]
    type = ElementIntegralVariablePostprocessor
    block_id = '1'
    execute_on = timestep_begin
    variable = bison_kappa_fission
  [../]
[]

[Outputs]
  exodus = true
[]

