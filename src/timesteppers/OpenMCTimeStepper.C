#include "OpenMCTimeStepper.h"
#include "OpenMCInterface.h"

template<>
InputParameters validParams<OpenMCTimeStepper>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1.0, "Size of the time step");
  return params;
}

OpenMCTimeStepper::OpenMCTimeStepper(const InputParameters & parameters) :
    TimeStepper(parameters),
    _dt(getParam<Real>("dt"))
{
}

/* This method is a pure virtual function, so it must be redefined by any
daughter class, even if the behavior doesn't change. */
Real
OpenMCTimeStepper::computeInitialDT()
{
  return _dt;
}

/* This method is a pure virtual function, so it must be redefined by any
daughter class, even if the behavior doesn't change. */
Real
OpenMCTimeStepper::computeDT()
{
  return _dt;
}

void
OpenMCTimeStepper::step()
{
  // reset tallies to zero, "clear" any OpenMC instances of std::vector
  // implementation, etc.
  OpenMC::openmc_reset();

  // perform all the logic for a Monte Carlo solve
  OpenMC::openmc_run();
}

void
OpenMCTimeStepper::postExecute()
{
  // free dynamically allocated memory, write output files, etc.
  OpenMC::openmc_finalize();
}

/* If Okapi is run as the Master App, we'll need to modify this to use a better
   estimate of convergence, since otherwise no subcycling would occur for a
   Picard step. */
bool
OpenMCTimeStepper::converged()
{
  return true;
}
