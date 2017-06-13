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
  OpenMC::openmc_run();
//  OpenMC::FORTRAN_CALL_MOD(run_simulation, simulation)();
}

bool
OpenMCTimeStepper::converged()
{
  return true;
}
