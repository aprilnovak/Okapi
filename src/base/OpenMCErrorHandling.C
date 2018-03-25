#include "OpenMCErrorHandling.h"
#include "OpenMCInterface.h"
#include "MooseObject.h"

/* This file contains functions for handling error codes returned from OpenMC
   routines. OpenMC sets its own error codes in 'openmc_err_msg', but we want to
   print more verbose messages to aid in debugging. Note that any changes made to
   these error messages will require you to modify the tests files. */

void ErrorHandling::receive_coeffs_for_cell(int err)
{
  if (err == e_tally_not_allocated)
    mooseError("Cannot set expansion coefficients for cell because"
      " no FE tallies are defined in OpenMC! Check that an FET"
      " has been defined in the tallies.xml file.");

  if (err == e_cell_not_found)
    mooseError("Invalid cell ID specified for storing expansion coefficients"
      " for FE tally! Check that there is an FET defined for"
      " the requested cell ID.");

  if (err == e_invalid_size)
    mooseError("Number of expansion coefficients to store in cell does not"
      " match previously allocated size!");
}

void ErrorHandling::get_coeffs_from_cell(int err)
{
  if (err == e_tally_not_allocated)
    mooseError("Cannot receive expansion coefficients for cell because"
      " no FE tallies are defined in OpenMC! Check that an FET"
      " has been defined in the tallies.xml file.");

  if (err == e_cell_not_found)
    mooseError("Invalid cell ID specified for receiving expansion coefficients"
      " for FE tally! Check that there is an FET defined for"
      " the requested cell ID.");

  if (err == e_invalid_size)
    mooseError("Number of expansion coefficients to receive from cell"
      " does not equal its allocated size! Check that the order of the FE"
      " matches the order of the scalar aux variables in the MOOSE input files.");
}

void ErrorHandling::openmc_cell_set_temperature(int err)
{
  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == e_warning)
      mooseWarning(openmc_error_message +
        "Setting temperature to nearest available point!");

    if (err == e_out_of_bounds)
      mooseError(openmc_error_message +
        "Check that the cell used in 'openmc_cell_set_temperature' exists in "
	"the geometry XML file.");

    // no additional descriptions needed for e_geometry or e_unassigned
  }
}

void ErrorHandling::openmc_get_cell_index(int err, const std::string & desc)
{

  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == e_invalid_id)
      mooseError(openmc_error_message +
        "Check that the OpenMC cell ID used for the " + desc + " transfer exists "
        "in the geometry XML file.");
    if (err == e_allocate)
      mooseError(openmc_error_message +
        "Check that the call to 'openmc_get_cell' in " + desc +
        " occurs after the OpenMC cells have been allocated.");
  }
}

void ErrorHandling::openmc_get_material_index(int err, const std::string & desc)
{
  if (err != 0)
  {
    std::string openmc_error_message(openmc_err_msg);
    openmc_error_message = "OpenMC Error: '" + openmc_error_message + "' ";

    if (err == e_invalid_id)
      mooseError(openmc_error_message +
        "Check that the OpenMC material ID used for the " + desc + " transfer "
        "exists in the material XML file.");
    if (err == e_allocate)
      mooseError(openmc_error_message +
        "Check that the call to 'openmc_get_material' in " + desc +
        "occurs after the OpenMC materials have been allocated.");
  }
}

void ErrorHandling::openmc_material_set_density(int err)
{
  if (err == e_out_of_bounds)
    mooseError("Material instance specified for setting density is out of bounds!");
}
