# This input file is used by the Master App. With this configuration, 
# the Master App does not perform any actual solves - it just facilitates
# solution transfers between BISON and Nek.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = dummy
  [../]
  [./dummy]
    type = Diffusion
    variable = dummy
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[MultiApps]
  [./openmc]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'openmc.i'
    execute_on = timestep_begin
  [../]
[]

[Outputs]
    exodus = true
[]
