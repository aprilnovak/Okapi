# This input file performs a BISON solve over the fuel pin.

[Mesh]
  file = 3D_sideset.exo
  block_id = '1'
  block_name = 'interior'
  boundary_id = '100 200 300'
  boundary_name = 'top bottom wall'
[]

[Variables]
  [./bison_temp]
  [../]
[]

[AuxVariables]
  [./l_0_coeffs_power_bison]
    family = SCALAR
    order = THIRD
  [../]
  [./l_0_coeffs_temp_bison]
    family = SCALAR
    order = THIRD
  [../]
  [./bison_kappa_fission] # holds eV/particle field from OpenMC
  [../]
  [./bison_heat] # holds the heat source used in kernel
  [../]
[]

[AuxKernels]
  [./bison_kappa_fisson]
    type = FunctionAux
    variable = bison_kappa_fission
    function = reconstruction
  [../]
  [./bison_power]
    type = KappaFissionToHeatSource
    variable = bison_heat
    kappa_fission_source = bison_kappa_fission
    power = 10.0
    volume_pp = 'volume'
  [../]
[]

# The expansion for power produced by OpenMC is expanded in the Master App.
# Then, an aux variable in the Master App (FunctionAux kernel) is used to
# take advantage of normal MOOSE transfer capabilities to send it to BISON.
# The only disadvantage here is that we would need to duplicate these
# functions for every pin...
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
  [./reconstruction]
    type = ZernikeLegendreReconstruction
    l_order = 0
    n_order = 1
    l_direction = 2
    legendre_function = legendre
    zernike_function = zernike
    poly_coeffs = 'l_0_coeffs_power_bison'
  [../]
[]

[Kernels]
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

[UserObjects]
  [./l0]
    type = ZLDeconstruction
    variable = bison_temp
    l_order = 0
    n_order_to_openmc = 1
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
  [./volume]
    type = VolumePostprocessor
    block_id = '1'
    execute_on = timestep_begin
  [../]
[]

