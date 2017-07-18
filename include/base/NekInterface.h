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

    // CFILES common block
    extern struct {
      char \
        reafle[PATHLEN], \
        fldfle[PATHLEN], \
        dmpfle[PATHLEN], \
        hisfle[PATHLEN], \
        schfle[PATHLEN], \
        orefle[PATHLEN], \
        nrefle[PATHLEN];
    } cfiles_;

    // DIMN common block
    extern struct {
      int \
        nelv,  \
        nelt,  \
        nx1,   \
        ny1,   \
        nz1,   \
        nx2,   \
        ny2,   \
        nz2,   \
        nx3,   \
        ny3,   \
        nz3,   \
        ndim,  \
        nfield,\
        npert, \
        nid,   \
        nxd,   \
        nyd,   \
        nzd;
    } dimn_;

    extern struct {
      double flux_moose, temp_nek;
    } test_passing_;

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

    // holds binned axially-averaged fluid temperatures and densities created
    // by Nek that are passed to Okapi
    extern struct {
      double fluid_temp_bins [];
      int n_layer;
    } axial_averages_;

    // routines to run Nek
    void FORTRAN_CALL(nek_init)(const int&);
    void FORTRAN_CALL(nek_init_step)();
    void FORTRAN_CALL(nek_step)();
    void FORTRAN_CALL(nek_finalize_step)();

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
