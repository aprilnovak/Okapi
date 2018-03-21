These tests all investigate if the correct error codes are returned from OpenMC based on coupling information provided by MOOSE. These tests cover:

* Attempting to obtain expansion coefficients from an OpenMC cell, but:
  1. The array (in MOOSE) is not the same size as the number of OpenMC coefficients
  2. No functional expansion tallies are defined for the requested cell
  3. No functional expansion tallies defined at all
* User-specified cell ID (in MOOSE input file) does not match any in OpenMC
* Temperature passed into OpenMC is:
  1. Higher than the highest loaded cross section data set
  2. Lower than the lowest loaded cross section data set
* Attempting to send expansion coefficients to OpenMC, but:
  1. No functional expansion tally (with received_coeffs array) is allocated, so OpenMC has no place to store the coefficients
  2. No functional expansion tally anywhere in the domain
