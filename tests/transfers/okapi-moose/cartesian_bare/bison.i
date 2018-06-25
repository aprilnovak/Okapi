[GlobalParams]
  volume_pp = 'volume'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1200
  xmin = -60
  xmax = 60
  block_id = '1'
[]

[Functions]
  [./kappa_fission_mutable_series]
    type = FunctionSeries
    series_type = Cartesian
    orders = '10'
    physical_bounds = '-60 60'
    x = Legendre
    expansion_type = 'orthonormal'
  [../]
  [./temperature_mutable_series]
    type = FunctionSeries
    series_type = Cartesian
    orders = '10'
    physical_bounds = '-60 60'
    x = Legendre
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
    power = 1e6
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
    boundary = 'left right'
    value = 300.0
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
  exodus = true
[]
