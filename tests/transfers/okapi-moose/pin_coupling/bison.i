# This input file performs a BISON solve over the fuel pin. The geometry is
# a cylinder of radius 0.5 and height 1.0, centered at 0, 0, 0 (see
# geometry.xml file for a more explicit OpenMC-based description of the
# geometry). OpenMC solves for this pin surrounded by water, while BISON only
# solves for the pin. A fixed surface temperature is imposed in BISON. No data
# transfer is performed for the fluid.

# The fission distribution doesnt look anything like it should because the
# number of particles used is super low, so everything is contaminated by
# noise.

[GlobalParams]
  legendre_function = legendre
  fourier_function = fourier
  zernike_function = zernike
  l_direction = 2
  volume_pp = 'volume'
  dbg = false
[]

[Mesh]
  file = cylinder.e
  block_id = '1'
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
    order = TWENTYFIRST
  [../]

  [./l_0_coeffs_temp] # where temperature coefficients are placed
    family = SCALAR
    order = THIRD
  [../]
  [./l_1_coeffs_temp] # where temperature coefficients are placed
    family = SCALAR
    order = THIRD
  [../]
  [./l_2_coeffs_temp] # where temperature coefficients are placed
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

  # the power set here is an arbitrary number - if you make this higher, you
  # can sort of see the impact of the fission distribution on the temperature
  [./fission_heat]
    type = KappaFissionToHeatSource
    variable = fission_heat
    kappa_fission_source = kappa_fission
    power = 1e3
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
  [./sides]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 600.0
  [../]
  [./top_bottom]
    type = NeumannBC
    variable = temp
    boundary = '2 3'
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
  [./l_1_temp_coeffs]
    type = ZLDeconstruction
    variable = temp
    l_order = 1
    n_order = 1
    aux_scalar_name = 'l_1_coeffs_temp'
  [../]
  [./l_2_temp_coeffs]
    type = ZLDeconstruction
    variable = temp
    l_order = 2
    n_order = 1
    aux_scalar_name = 'l_2_coeffs_temp'
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
  exodus = true
[]

