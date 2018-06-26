[GlobalParams]
  volume_pp = 'volume'
[]

[Mesh]
  file = cylinder.e
  block_id = '1'
[]

[Functions]
  [./kappa_fission_mutable_series]
    type = FunctionSeries
    series_type = CylindricalDuo
    orders = '0   0' # Axial first, then (r, t) FX
    physical_bounds = '-0.5 0.5   0 0 60' # z_min z_max   x_center y_center radius
    z = Legendre # Axial in z
    disc = Zernike # (r, t) default to unit disc in x-y plane
    expansion_type = 'orthonormal'
  [../]
  [./temperature_mutable_series]
    type = FunctionSeries
    series_type = CylindricalDuo
    orders = '2   1' # Axial first, then (r, t) FX
    physical_bounds = '-0.5 0.5   0 0 60' # z_min z_max   x_center y_center radius
    z = Legendre # Axial in z
    disc = Zernike # (r, t) default to unit disc in x-y plane
    generation_type = 'orthonormal'
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
    initial_condition = 300
  [../]
[]

[AuxVariables]
  [./kappa_fission] # holds eV/particle field from OpenMC
  [../]
  [./fission_heat] # holds the fission heat source
  [../]
[]

[AuxKernels]
  [./reconstruct_kapp_fission]
    type = FunctionSeriesToAux
    function = kappa_fission_mutable_series
    variable = kappa_fission
  [../]
  # the power set here is an arbitrary number - if you make this higher, you
  # can sort of see the impact of the fission distribution on the temperature
  [./fission_heat]
    type = KappaFissionToHeatSource
    variable = fission_heat
    kappa_fission_source = kappa_fission
    power = 1e8
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
    value = 300.0
  [../]
  [./top_bottom]
    type = NeumannBC
    variable = temp
    boundary = '2 3'
    value = 0.0
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
  [./kappa_average]
    type = ElementAverageValue
    variable = kappa_fission
    execute_on = timestep_end
  [../]
[]

[Outputs]
  exodus = true
[]
