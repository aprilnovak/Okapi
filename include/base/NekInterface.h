#ifdef ENABLE_NEK_COUPLING

// Okapi is a wrapper around OpenMC, but we also use it as the Master App to
// pass information to Nek, so we need to include an interface to the Nek
// routines that we'll call. Because Nek is written mostly in Fortran 77, we
// control name mangling using the FORTRAN_CALL macro defined in Moose.h.
// Nek common blocks map directly to C structs.

#ifndef NEKINTERFACE_H
#define NEKINTERFACE_H

#include "Moose.h"

// String length for file paths in Nek5000
#define PATHLEN 132

namespace Nek5000
{
  extern "C" {

    // holds temperature expansion coefficients created by Nek that are passed
    // to MOOSE
    extern struct {
      double coeff_tij [];
    } expansion_tcoef_;

    // holds flux expansion coefficients created by MOOSE that are passed to Nek
    extern struct {
      double coeff_fij [];
    } expansion_fcoef_;

    // holds expansion orders for temperature BC expansion
    extern struct {
      int n_legendre;
      int m_fourier;
   } expansion_tdata_;

    // holds number of axial bins for axial fluid temperature and denisty
    // integration
    extern struct {
      int n_layer;
    } layer_data_;

    // holds axially-averaged fluid temperature
    extern struct {
      double fluid_temp_bins[];
    } fluid_bins_;

    // deconstructs a continuous variable in Nek into a set of expansion
    // coefficients using a Fourier-Legendre expansion
    void FORTRAN_CALL(nek_expansion)();

    // reconstruct a continuous field in Nek using expansion coefficients
    // from MOOSE
    void FORTRAN_CALL(flux_reconstruction)();

   // deconstruct a continuous variable in Nek into a set of axially
   // averaged values using binned volume integration
   void FORTRAN_CALL(axially_binned_integration)();
  }
}

#endif /* NEKINTERFACE_H */

#endif // ENABLE_NEK_COUPLING
