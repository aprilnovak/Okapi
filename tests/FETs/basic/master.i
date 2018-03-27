# This input file is used by the Master App. With this configuration,
# the Master App does not perform any actual solves - it just facilitates
# solution transfers between BISON and Nek.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[AuxVariables]
  [./l_0_coeffs]
    family = SCALAR
    order = SIXTH
  [../]
[]

[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = l_0_coeffs
    values = '400.0 500.0 600.0 700.0 800.0 900.0'
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
  [./openmc]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'openmc.i'
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./from_openmc]
    type = MultiAppOkapiMooseTransfer
    multi_app = openmc
    direction = from_multiapp
    source_variable = dummy
    to_aux_scalar = 'l_0_coeffs'
    openmc_cell = 1
  [../]
[]

[Outputs]
    exodus = true
[]
