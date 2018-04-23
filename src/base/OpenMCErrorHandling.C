#include "OpenMCErrorHandling.h"
#include "MooseObject.h"

// openmc include
#include "openmc.h"

/* This file contains functions for handling error codes returned from OpenMC
   routines. OpenMC sets its own error codes in 'openmc_err_msg', but we want to
   print more verbose messages to aid in debugging. Note that any changes made to
   these error messages will require you to modify the tests files. */

void
ErrorHandling::get_coeffs_from_cell(int err)
{
  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == OPENMC_E_ALLOCATE)
      mooseError(openmc_error_message + "Cannot receive expansion coefficients for cell because"
                                        " no FE tallies are defined in OpenMC! Check that an FET"
                                        " has been defined in the tallies.xml file.");

    if (err == OPENMC_E_OUT_OF_BOUNDS)
      mooseError(openmc_error_message +
                 "Invalid cell ID specified for receiving expansion coefficients"
                 " for FE tally! Check that there is an FET defined for"
                 " the requested cell ID.");
  }
}

void
ErrorHandling::openmc_cell_set_temperature(int err)
{
  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == OPENMC_E_WARNING)
      mooseWarning(openmc_error_message + "Setting temperature to nearest available point!");

    if (err == OPENMC_E_OUT_OF_BOUNDS)
      mooseError(openmc_error_message +
                 "Check that the cell used in 'openmc_cell_set_temperature' exists in "
                 "the geometry XML file.");

    // no additional descriptions needed for e_geometry or e_unassigned
  }
}

void
ErrorHandling::openmc_get_cell_index(int err, const std::string & desc)
{

  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == OPENMC_E_INVALID_ID)
      mooseError(openmc_error_message + "Check that the OpenMC cell ID used for the " + desc +
                 " transfer exists "
                 "in the geometry XML file.");
    if (err == OPENMC_E_ALLOCATE)
      mooseError(openmc_error_message + "Check that the call to 'openmc_get_cell' in " + desc +
                 " occurs after the OpenMC cells have been allocated.");
  }
}

void
ErrorHandling::openmc_get_material_index(int err, const std::string & desc)
{
  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == OPENMC_E_INVALID_ID)
      mooseError(openmc_error_message + "Check that the OpenMC material ID used for the " + desc +
                 " transfer "
                 "exists in the material XML file.");
    if (err == OPENMC_E_ALLOCATE)
      mooseError(openmc_error_message + "Check that the call to 'openmc_get_material' in " + desc +
                 "occurs after the OpenMC materials have been allocated.");
  }
}

void
ErrorHandling::openmc_material_set_density(int err)
{
  if (err == OPENMC_E_OUT_OF_BOUNDS)
    mooseError("Material instance specified for setting density is out of bounds!");
}

void
ErrorHandling::openmc_get_keff(int err)
{
  // the openmc_get_keff subroutine in OpenMC/eigenvalue.F90 should be adapted
  // to using the C-API error codes and return a warning/error message from OpenMC
  // via the openmc_err_msg array. This error handling function is sufficient
  // until then.
  if (err != 0)
    mooseError("Less that four k_eff realizations in call to "
               "'openmc_get_keff' subroutines!");
}
