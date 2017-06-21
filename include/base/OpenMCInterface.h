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

    // Structure data types for transfer of geometric information
/*    typedef struct CouplingGeom {
      int l_dir;
      double radius;
      double center[2];
      double height[2];
    } CouplingGeom;*/

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


    /* Routines that are used for data transfer between OpenMC and MOOSE. */
    void change_batches(int);
    void change_fuel_temp(double);
    
    // tell OpenMC about the order of the expansion for transfer  _from_ MOOSE
    void receive_coupling_info(int, int);

    // read expansion coefficients into OpenMC array
    void receive_coeffs(double);
  }
}

#endif // PRONGHORN_INTERFACE_H
