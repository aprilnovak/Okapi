// For coupling an external core into the Moose framework, we need to inform
// MOOSE about the names of those external functions/routines we'll be calling
// so that the MOOSE-wrapped App will compile. All external code routines must
// be forward-declared here with the proper name mangling. In OpenMC, we use
// the bind(c) attribute so that the names in the Fortran dynamic library are
// the same as those in the MOOSE dynamic library. For older Fortran 77 code,
// use the FORTRAN_CALL macro defined in Moose.h, since the ISO_C_BINDING
// intrinsics don't apply for Fortran 77.

#ifndef OPENMC_INTERFACE_H
#define OPENMC_INTERFACE_H

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
       assume that "MPI" is defined in OpenMC, such that openmc_init must be
       called with an intracommunicator. */

    // Routines that are used to run OpenMC.
    void openmc_init(const int &);
    void openmc_reset();
    void openmc_run();
    void openmc_finalize();

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

#endif // OPENMC_INTERFACE_H
