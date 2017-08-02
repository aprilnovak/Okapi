#include "OpenMCErrorHandling.h"
#include "MooseObject.h"

/* This file contains functions for handling error codes returned from OpenMC
   routines. Specific error messages are defined for each of the OpenMC routines
   to assist in debugging errors. Note that any changes made to these error
   messages will require you to modify all of the tests files. */
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
  if (err == w_below_min_bound)
    mooseWarning("Cross sections not available for specified temperature, "
      "setting temperature to lowest available data point!");

  if (err == w_above_max_bound)
    mooseWarning("Cross sections not available for specified temperature, "
      "setting temperature to highest available data point!");

  if (err == e_cell_no_material)
    mooseError("Cannot specify cell filled with a universe for "
      "'openmc_cell_set_temperature' routine!");

  if (err == e_out_of_bounds)
    mooseError("Cell instance specified for setting temperature is out of bounds!");
}

void ErrorHandling::openmc_get_cell(int err, const std::string & desc)
{
  if (err == e_cell_invalid_id)
    mooseError("Invalid cell ID used in call to 'openmc_get_cell' routine!"
      " Check that the cell ID used for the " + desc + " transfer exists in"
      " the geometry XML file.");
  if (err == e_cell_not_allocated)
    mooseError("Cell is not allocated in call to 'openmc_get_cell' routine!"
      " Check that the call to this routine occurs after the cells array has"
      " been allocated.");
}
