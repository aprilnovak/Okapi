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
    order = TENTH
  [../]
  [./f_1_coeffs_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
  [./f_2_coeffs_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
  [./f_3_coeffs_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
  [./f_4_coeffs_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
  [./f_5_coeffs_flux_BC]
    family = SCALAR
    order = TENTH
  [../]
[]

[ICs]
  # ---- IC on the wall heat flux used by Nek because BISON runs ---- #
  #      before MOON.                                                 #
  [./f_0_coeffs_flux_BC]
    type = ScalarComponentIC
    variable = 'f_0_coeffs_flux_BC'
    values = '1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
  [../]
[]

[Executioner]
  type = OpenMCExecutioner
  num_steps = 100
  [./TimeStepper]
    type = OpenMCTimeStepper
    dt = 0.02
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
[]

[Outputs]
  exodus = true
[]
