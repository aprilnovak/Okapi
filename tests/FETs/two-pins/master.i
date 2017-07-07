# This input file is used by the Master App. With this configuration,
# the Master App does not perform any actual solves - it just facilitates
# solution transfers. This test fully-couples OpenMC and BISON, using a
# zeroth order Legendre expansion and a second-order Zernike expansion.
# Note that at this point, OpenMC does not have Zernike-Legendre expansions,
# but _only_ Zernike expansions, so there is a constant scaling factor that
# is incorrect for this coupling, but the mechanics are correct.

# In order to transfer a continuous field variable to BISON, we need to
# have solve = true in order to get the AuxKernels to evaluate. But, we
# dont need to perform a _nonlinear_ solve, so we can set no kernels for
# the dummy variable "u".

[Problem]
  solve = true
  kernel_coverage_check = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables] # dummy variable
  [./u]
  [../]
[]

[AuxVariables]
  [./l_0_coeffs_power] # sent from MOOSE to BISON
    family = SCALAR
    order = THIRD
  [../]
  [./l_0_coeffs_temp] # sent from MOOSE to OpenMC
    family = SCALAR
    order = THIRD
  [../]
  [./openmc_kappa_fission] # sent to BISON, already expanded by MOOSE
  [../]
[]

# We only need an initial condition for temperature, since OpenMC executes on
# timestep_begin, and the initial temperatures are set in the Master App
# input file. This will set an initial temperature of 500 K in the
# corresponding cells in OpenMC. Any other temperatures in OpenMC will come
# from the XML files.
[ICs]
  [./ic]
    type = ScalarComponentIC
    variable = 'l_0_coeffs_temp'
    values = '900.0 0.0 0.0'
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
  [./bison]
    type = TransientMultiApp
    app_type = OkapiMCSApp
    positions = '0 0 0'
    input_files = 'bison.i'
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./to_bison]
    type = MultiAppScalarToAuxScalarTransfer
    multi_app = bison
    direction = to_multiapp
    source_variable = 'l_0_coeffs_power'
    to_aux_scalar = 'l_0_coeffs_power_bison'
    execute_on = timestep_end
  [../]
  [./from_bison]
    type = MultiAppScalarToAuxScalarTransfer
    multi_app = bison
    direction = from_multiapp
    source_variable = 'l_0_coeffs_temp_bison'
    to_aux_scalar = 'l_0_coeffs_temp'
    execute_on = timestep_end
  [../]

# This transfer specifies the cell index in the OpenMC cells array so that
# we know which cell to pass the information to/receive the information from.
  [./to_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = to_multiapp
    openmc_cell = 20
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'dummy_openmc'
    execute_on = timestep_begin
  [../]
  [./from_openmc]
    type = PolynomialOpenMC
    multi_app = openmc
    direction = from_multiapp
    openmc_cell = 20
    source_variable = 'dummy_openmc'
    to_aux_scalar = 'l_0_coeffs_power'
    execute_on = timestep_begin
  [../]
[]

[Outputs]
    exodus = true
[]
