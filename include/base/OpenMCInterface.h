// test interface file that will be used to try to call a fortran subroutine
// held in a *.F90 file. This Fortran function prototype will be declared in
// this file.

#ifndef PRONGHORN_INTERFACE_H
#define PRONGHORN_INTERFACE_H

#include "Moose.h" // include for access to FORTRAN_CALL macro

// define a macro for name mangling of subroutines/functions in Fortran modules
#define FORTRAN_CALL_MOD(name, mod) __##mod##_MOD_##name

namespace OpenMC {
/* need to prevent name mangling between C++ and Fortran, since C++ permits 
   function overloading, which to distinguish between different versions of
   a function with the same name, means that the C++ compiler changes the 
   name in the symbol table in the object file. We need to avoid this so that
   we can get the names to correctly match up. */

  extern "C" {
  /* Prototype a simple subroutine not in a module. This requires a different
     macro than a subroutine in a module. This prototype is simply for testing. */
    //void FORTRAN_CALL(openmc_fort_func)();

  /* Prototype a subroutine in a module, which requires a different macro than
     a subroutine not in a module. */
    
    /* OpenMC can run in "serial" in two ways - using the MPI wrapper (with a 
       single process), or _actually_ in serial. "#ifdef MPI" is used within
       OpenMC to differentiate between these two cases, but because this variable
       is defined within the OpenMC CMake file, it is not known to MOOSE. I 
       assume that MPI is defined in OpenMC, such that openmc_init must be called
       with an intracommunicator. */
    void openmc_init(const int &);
    void openmc_reset();
    void openmc_run();
    void openmc_finalize();
    //void FORTRAN_CALL_MOD(openmc_init, initialize)(const int &);
    //void FORTRAN_CALL_MOD(run_simulation, simulation)();
    //void FORTRAN_CALL_MOD(openmc_finalize, finalize)();
  }
}

#endif // PRONGHORN_INTERFACE_H
