#include "MomFrictionForce.h"
#include "OpenMCInterface.h" // included because I want to call one of the Fortran functions

template<>
InputParameters validParams<MomFrictionForce>()
{
  InputParameters params = validParams<Reaction>();
  return params;
}


MomFrictionForce::MomFrictionForce(const InputParameters & parameters) :
    Reaction(parameters),
    _W(getMaterialProperty<Real>("W"))
{
}


Real
MomFrictionForce::computeQpResidual()
{
  static bool first_step = true;

  if (first_step)
  {
    OpenMC::FORTRAN_CALL(openmc_fort_func)();
    OpenMC::FORTRAN_CALL_MOD(openmc_init, initialize)(_communicator.get());
    OpenMC::FORTRAN_CALL_MOD(run_simulation, simulation)();
    OpenMC::FORTRAN_CALL_MOD(openmc_finalize, finalize)();
    first_step = false;
}
  return _W[_qp] * Reaction::computeQpResidual();
}


Real
MomFrictionForce::computeQpJacobian()
{
  return _W[_qp] * Reaction::computeQpJacobian();
}
