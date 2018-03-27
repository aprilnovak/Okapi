# This input file is used by the Master App. With this configuration,
# the Master App does not perform any actual solves - it just facilitates
# solution transfers between BISON and Nek.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[AuxVariables]
  [./l_0_flux_BC]
    family = SCALAR
    order = SIXTH
  [../]
  [./l_0_temp_BC]
    family = SCALAR
    order = SIXTH
  [../]
[]

# We only need an initial condition for temperature, since OpenMC executes on
# timestep_begin, and the initial temperatures are set in the Master App input file.
# This will set an initial temperature of 400 K in the corresponding cells in OpenMC.
# Any other temperatures in OpenMC will come from the XML files.
[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_temp_BC'
    values = '500.0 0.0 0.0 0.0 0.0 0.0'
  [../]
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
  [./bison]
    type = TransientMultiApp
    app_type = BuffaloApp
    positions = '0 0 0'
    input_files = 'bison.i'
    execute_on = timestep_begin
    library_path = /homes/anovak/projects/buffalo/lib
  [../]
  [./openmc]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'openmc.i'
    execute_on = timestep_begin
  [../]
  [./nek]
    type = TransientMultiApp
    app_type = MoonApp
    positions = '0 0 0'
    input_files = 'nek.i'
    execute_on = timestep_begin
    library_path = /homes/anovak/projects/moon/lib
  [../]
[]

[Outputs]
    exodus = true
[]
