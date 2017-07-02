# This input file is used by the Master App. With this configuration,
# the Master App does not perform any actual solves - it just facilitates
# solution transfers. This test fully-couples OpenMC and BISON, using a
# zeroth order Legendre expansion and a sixth-order Zernike expansion.
# Note that at this point, OpenMC does not have Zernike-Legendre expansions,
# but _only_ Zernike expansions, so there is a constant scaling factor that
# is incorrect for this coupling, but the mechanics are correct.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[AuxVariables]
  [./l_0_coeffs_power] # sent from OpenMC to BISON
    family = SCALAR
    order = SIXTH
  [../]
  [./l_0_coeffs_temp] # sent from BISON to OpenMC
    family = SCALAR
    order = SIXTH
  [../]
[]

# We only need an initial condition for temperature, since OpenMC executes on
# timestep_begin, and the initial temperatures are set in the Master App input file.
# This will set an initial temperature of 500 K in the corresponding cells in OpenMC.
# Any other temperatures in OpenMC will come from the XML files.
[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_temp'
    values = '500.0 0.0 0.0 0.0 0.0 0.0'
  [../]
[]

# The MasterApp does not perform any solve
[Variables]
  [./dummy]
  [../]
[]

[Functions]
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

[BCs]
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
  [./bison] # currently some problem with Buffalo, use OkapiMCS for now
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'bison.i'
    execute_on = timestep_end
#    library_path = /homes/anovak/projects/buffalo/lib
  [../]
[]

[Transfers]
active = 'from_openmc'
  [./to_bison]
    type = PolynomialOpenMC
    multi_app = bison
    direction = to_multiapp
    source_variable = 'l_0_coeffs_power'
    to_aux_scalar = 'l_0_coeffs_power_bison'
    execute_on = timestep_end
  [../]

# This transfer specifies the cell index in the OpenMC cells array so that
# we know which cell to pass the information to/receive the information from.
  [./to_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = to_multiapp
    openmc_cell = 1
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'dummy_openmc'
    execute_on = timestep_begin
  [../]

  [./from_bison]
    type = PolynomialOpenMC
    multi_app = bison
    direction = from_multiapp
    source_variable = 'l_0_coeffs_temp_bison'
    to_aux_scalar = 'l_0_coeffs_temp'
    execute_on = timestep_end
  [../]

  [./from_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = from_multiapp
    openmc_cell = 1
    source_variable = 'dummy_openmc'
    to_aux_scalar = 'l_0_coeffs_power'
    execute_on = timestep_begin
  [../]
[]

[Postprocessors]
[]

[Outputs]
    exodus = true
[]
