# This input file performs a BISON solve over the fuel pin.

[GlobalParams]
  volume_pp = 'volume'
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
  [./kappa_fission_mutable_series]
    type = FunctionSeries
    series_type = CylindricalDuo
    orders = '0   5' # Axial first, then (r, t) FX
    physical_bounds = '0 1  0 0 0.5' # z_min z_max   x_center y_center radius
    z = Legendre
    disc = Zernike
    expansion_type = 'sqrt_mu'
  [../]
  [./temperature_mutable_series]
    type = FunctionSeries
    series_type = CylindricalDuo
    orders = '0   1' # Axial first, then (r, t) FX
    physical_bounds = '0 1  0 0 0.5' # z_min z_max   x_center y_center radius
    z = Legendre
    disc = Zernike
    generation_type = 'sqrt_mu'
  [../]
[]

[UserObjects]
  [./temperature_mutable_series_uo]
    type = FXVolumeUserObject
    function = temperature_mutable_series
    variable = temp
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./kappa_fission] # holds eV/particle field from OpenMC
  [../]
  [./fission_heat] # holds the fission heat source
  [../]
[]

[AuxKernels]
  [./kappa_fisson]
    type = FunctionSeriesToAux
    variable = kappa_fission
    function = kappa_fission_mutable_series
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
    boundary = 'left right back'
    value = 5.0
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
    execute_on = timestep_begin
  [../]
[]

[Outputs]
  exodus = false
[]
