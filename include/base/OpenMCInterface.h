// test interface file that will be used to try to call a fortran subroutine
// held in a *.F90 file. This Fortran function prototype will be declared in
// this file.

#ifndef PRONGHORN_INTERFACE_H
#define PRONGHORN_INTERFACE_H

#include "Moose.h" // include for access to FORTRAN_CALL macro
#include <vector>

// define a macro for name mangling of subroutines/functions in Fortran modules
#define FORTRAN_CALL_MOD(name, mod) __##mod##_MOD_##name

namespace OpenMC {
/* need to prevent name mangling between C++ and Fortran, since C++ permits
   function overloading, which to distinguish between different versions of
   a function with the same name, means that the C++ compiler changes the
   name in the symbol table in the object file. We need to avoid this so that
   we can get the names to correctly match up. */

  extern "C" {

    /* OpenMC can run in "serial" in two ways - using the MPI wrapper (with a
       single process), or _actually_ in serial. "#ifdef MPI" is used within
       OpenMC to differentiate between these two cases, but because this variable
       is defined within the OpenMC CMake file, it is not known to MOOSE. I
       assume that MPI is defined in OpenMC, such that openmc_init must be called
       with an intracommunicator. */

    /* Routines that are used to run OpenMC. */
    void openmc_init(const int &);
    void initialize_coupling(); // initializes cell mapping
    void openmc_reset();
    void openmc_run();
    void openmc_finalize();

    // tell OpenMC about the order of the expansion for transfer  _from_ MOOSE
//    void receive_coupling_info(int, int);

    // read expansion coefficients into OpenMC array
//    void receive_coeffs(double);

    // set a cell temperature given a temperature. This is used for transferring
    // data from MOOSE to OpenMC.
    int openmc_cell_set_temperature(int, double);

    // store expansion coefficients by cell. This is used for transferring data
    // from OpenMC to MOOSE.
    void fet_deconstruction();

    // obtain coefficients given cell ID. This is used for transferring data
    // from OpenMC to MOOSE. This returns an error code.
    int get_coeffs_from_cell(int, double [], int);

    // set coefficients in OpenMC given a cell ID. This is used for
   // transferring data from MOOSE to OpenMC. This returns an error code.
   int receive_coeffs_for_cell(int, double [], int);
  }
}

#endif // PRONGHORN_INTERFACE_H
