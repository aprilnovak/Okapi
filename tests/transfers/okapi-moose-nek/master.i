[GlobalParams]
  dbg = true
[]

[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  # ---- middle-man storage for the temp BC computed by Nek ---- #
  [./f_0_coeffs_temp_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_1_coeffs_temp_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_2_coeffs_temp_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_3_coeffs_temp_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_4_coeffs_temp_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_5_coeffs_temp_BC]
    family = SCALAR
    order = ELEVENTH
  [../]

  # ---- middle-man storage for the flux BC computed by Nek ---- #
  [./f_0_coeffs_flux_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_1_coeffs_flux_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_2_coeffs_flux_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_3_coeffs_flux_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_4_coeffs_flux_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
  [./f_5_coeffs_flux_BC]
    family = SCALAR
    order = ELEVENTH
  [../]
[]

[ICs]
  # ---- IC on the wall heat flux used by Nek because BISON runs ---- #
  #      before MOON.                                                 #
  [./f_0_coeffs_flux_BC]
    type = ScalarComponentIC
    variable = 'f_0_coeffs_flux_BC'
    values = '1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
  [../]
[]

[Executioner]
  type = OpenMCExecutioner
  num_steps = 100
  [./TimeStepper]
    type = OpenMCTimeStepper
    dt = 0.01
  [../]
[]

[MultiApps]
  [./nek]
    type = TransientMultiApp
    app_type = MoonApp
    positions = '0 0 0'
    input_files = nek.i
    library_path = /homes/anovak/projects/moon/lib
    execute_on = timestep_begin
    sub_cycling = true
  [../]
  [./bison]
    type = TransientMultiApp
    app_type = BuffaloApp
    positions = '0 0 0'
    input_files = bison.i
    library_path = /homes/anovak/projects/buffalo/lib
    execute_on = timestep_end
  [../]
[]

[Transfers]
  # ---- Transfer heat flux BC from Nek to Okapi (middle-man storage ---- #
  #      before being passed to BISON)                                    #

  [./to_nek] # transfers heat flux BC
    type = MultiAppMoonOkapiTransfer
    direction = to_multiapp
    multi_app = nek
    source_variable = 'f_0_coeffs_flux_BC f_1_coeffs_flux_BC f_2_coeffs_flux_BC f_3_coeffs_flux_BC f_4_coeffs_flux_BC f_5_coeffs_flux_BC'
    to_aux_scalar = 'foo'
    execute_on = timestep_begin
  [../]

  # --------------------------------------------------------------------- #
  #      Transfer temperature BC from Nek to OKapi (middle-man            #
  #      storage before being passed to BISON)                            #

  [./from_nek] # writes temperature BC
    type = MultiAppMoonOkapiTransfer
    direction = from_multiapp
    multi_app = nek
    source_variable = 'foo'
    to_aux_scalar = 'f_0_coeffs_temp_BC f_1_coeffs_temp_BC f_2_coeffs_temp_BC f_3_coeffs_temp_BC f_4_coeffs_temp_BC f_5_coeffs_temp_BC'
    execute_on = timestep_begin
  [../]



  # ---- Transfer kappa fission coefficients from Okapi to BISON ---- #

  [./to_bison]
    type = MultiAppMooseOkapiTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'bar'
    to_aux_scalar = 'l_0_coeffs_kappa_fission'
    openmc_cell = 1
    execute_on = timestep_end
  [../]



  # --- Transfer SCALAR aux variables from Okapi to BISON ---- #
  #     Due to the nature of the MultiAppScalarToAuxScalar     #
  #     transfer, this needs to occur one at a time            #

  [./to_bison_temp_f0]
    type = MultiAppScalarToAuxScalarTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'f_0_coeffs_temp_BC'
    to_aux_scalar = 'f_0_coeffs_temp_BC_bison'
    execute_on = timestep_end
  [../]
  [./to_bison_temp_f1]
    type = MultiAppScalarToAuxScalarTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'f_1_coeffs_temp_BC'
    to_aux_scalar = 'f_1_coeffs_temp_BC_bison'
    execute_on = timestep_end
  [../]
  [./to_bison_temp_f2]
    type = MultiAppScalarToAuxScalarTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'f_2_coeffs_temp_BC'
    to_aux_scalar = 'f_2_coeffs_temp_BC_bison'
    execute_on = timestep_end
  [../]
  [./to_bison_temp_f3]
    type = MultiAppScalarToAuxScalarTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'f_3_coeffs_temp_BC'
    to_aux_scalar = 'f_3_coeffs_temp_BC_bison'
    execute_on = timestep_end
  [../]
  [./to_bison_temp_f4]
    type = MultiAppScalarToAuxScalarTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'f_4_coeffs_temp_BC'
    to_aux_scalar = 'f_4_coeffs_temp_BC_bison'
    execute_on = timestep_end
  [../]
  [./to_bison_temp_f5]
    type = MultiAppScalarToAuxScalarTransfer
    direction = to_multiapp
    multi_app = bison
    source_variable = 'f_5_coeffs_temp_BC'
    to_aux_scalar = 'f_5_coeffs_temp_BC_bison'
    execute_on = timestep_end
  [../]



  [./from_bison]
    type = MultiAppMooseOkapiTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'l_0_coeffs_temp'
    to_aux_scalar = 'bar'
    openmc_cell = 1
    execute_on = timestep_end
  [../]



  # --- Transfer SCALAR aux variables from BISON to Okapi ---- #
  #     Due to the nature of the MultiAppScalarToAuxScalar     #
  #     transfer, this needs to occur one at a time            #

  [./from_bison_flux_0]
    type = MultiAppScalarToAuxScalarTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'f_0_coeffs_flux_BC_bison'
    to_aux_scalar = 'f_0_coeffs_flux_BC'
    execute_on = timestep_end
  [../]
  [./from_bison_flux_1]
    type = MultiAppScalarToAuxScalarTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'f_1_coeffs_flux_BC_bison'
    to_aux_scalar = 'f_1_coeffs_flux_BC'
    execute_on = timestep_end
  [../]
  [./from_bison_flux_2]
    type = MultiAppScalarToAuxScalarTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'f_2_coeffs_flux_BC_bison'
    to_aux_scalar = 'f_2_coeffs_flux_BC'
    execute_on = timestep_end
  [../]
  [./from_bison_flux_3]
    type = MultiAppScalarToAuxScalarTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'f_3_coeffs_flux_BC_bison'
    to_aux_scalar = 'f_3_coeffs_flux_BC'
    execute_on = timestep_end
  [../]
  [./from_bison_flux_4]
    type = MultiAppScalarToAuxScalarTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'f_4_coeffs_flux_BC_bison'
    to_aux_scalar = 'f_4_coeffs_flux_BC'
    execute_on = timestep_end
  [../]
  [./from_bison_flux_5]
    type = MultiAppScalarToAuxScalarTransfer
    direction = from_multiapp
    multi_app = bison
    source_variable = 'f_5_coeffs_flux_BC_bison'
    to_aux_scalar = 'f_5_coeffs_flux_BC'
    execute_on = timestep_end
  [../]
[]

[Outputs]
  exodus = true
[]
