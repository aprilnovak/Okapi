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

    // OPENMC_INIT takes care of all initialization tasks, i.e. reading
    // from command line, reading xml input files, initializing random
    // number seeds, reading cross sections, initializing starting source,
    // setting up timers, etc.
    void openmc_init(const int &);

    // OPENMC_RESET resets tallies and timers
    void openmc_reset();

    // OPENMC_RUN encompasses all the main logic where iterations are performed
    // over the batches, generations, and histories in a fixed source or
    // k-eigenvalue calculation.
    void openmc_run();

    // OPENMC_FINALIZE frees up memory by deallocating arrays and resetting
    // global variables.
    void openmc_finalize();

    // OPENMC_GET_CELL returns the index in the cells array of a cell with
    // a given ID (defined in the XML file). This is used to avoid repeated
    // lookups in the hash table when multiple operations will be performed
    // on a cell.
    int openmc_get_cell(int32_t id, int32_t * index);

    // set a cell temperature given a temperature. This is used for transferring
    // data from MOOSE to OpenMC. Because the instance is optional, it is not
    // received by value on the Fortran side, so we use a pointer here. The
    // cell index can be determined with a call to 'openmc_get_cell'.
    int openmc_cell_set_temperature(int32_t index, double temperature,
      int32_t * instance);

    // store expansion coefficients by cell. This is used for transferring data
    // from OpenMC to MOOSE.
    void fet_deconstruction();

    // obtain coefficients given cell index. This is used for transferring data
    // from OpenMC to MOOSE. This returns an error code.
    int get_coeffs_from_cell(int32_t index, double [], int n);

    // set coefficients in OpenMC given a cell ID. This is used for
    // transferring data from MOOSE to OpenMC. This returns an error code.
    int receive_coeffs_for_cell(int32_t index, double [], int n);

  }
}

#endif // OPENMC_INTERFACE_H
