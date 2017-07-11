#include "OpenMCErrorHandling.h"
#include "MooseObject.h"

/* This file contains functions for handling error codes returned from OpenMC
   routines. */
void ErrorHandling::receive_coeffs_for_cell(int err)
{
  if (err == -1)
    mooseError("Invalid cell ID specified for storing expansion coefficients"
      " for kappa-fission-zn tally!");
  if (err == -2)
    mooseError("Number of expansion coefficients passed for cell"
      " does not equal its allocated size!");
  if (err == -3)
    mooseError("Cannot set expansion coefficients for cell because"
      " no kappa-fission-zn tallies are defined in OpenMC!");
}

void ErrorHandling::get_coeffs_from_cell(int err)
{
  if (err == -1)
    mooseError("Invalid cell ID specified for retrieving expansion"
      " coefficients for kappa-fission-zn tally!");
  if (err == -2)
    mooseError("Length of array to receive coefficients does not match"
      " number of coefficients! Check that the array to hold coefficients"
      " has been allocated with the proper size.");
  if (err == -3)
    mooseError("Cannot get expansion coefficients from cell because no"
      " kappa-fission-zn tallies are defined in OpenMC!");
}
