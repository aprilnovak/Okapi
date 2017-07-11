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
  [./temp_BC_reconstruction]
    type = FourierLegendreReconstruction
    l_order = 9
    f_order = 0
    l_direction = 2
    legendre_function = legendre
    fourier_function = fourier
    poly_coeffs = 'f_0_coeffs_temp_BC_bison'
  [../]
[]

[Variables]
  [./bison_temp]
  [../]
[]

[AuxVariables]
  [./l_0_coeffs_kappa_fission] # where kappa-fission coefficients are received
    family = SCALAR
    order = THIRD
  [../]
  [./f_0_coeffs_temp_BC_bison] # where heat flux coefficients are copied
    family = SCALAR
    order = TENTH
  [../]
  [./l_0_coeffs_temp] # where temperature coefficients are placed
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
  [./temp]
    type = FunctionDirichletBC
    variable = bison_temp
    function = temp_BC_reconstruction
    boundary = 'wall'
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

