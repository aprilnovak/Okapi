# This input file performs a BISON solve over the fuel pin.

[GlobalParams]
  legendre_function = legendre
  fourier_function = fourier
  zernike_function = zernike
  l_direction = 2
  volume_pp = 'volume'
  dbg = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  block_id = '1'
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
  [./l_0_coeffs_kappa_fission] # where kappa-fission coefficients are received
    family = SCALAR
    order = TWENTYSEVENTH
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
    type = DirichletBC
    variable = temp
    function = temp_BC_reconstruction
    boundary = 'left right back'
    value = 5.0
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

[Executioner]
  type = Transient
  nl_rel_tol = 1e-6
  l_tol = 1e-6
  dt = 0.01
[]

[Postprocessors]
  [./volume]
    type = VolumePostprocessor
    block_id = '1'
    execute_on = timestep_begin
  [../]
[]

[Outputs]
  exodus = false
[]

