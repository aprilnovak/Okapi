#ifndef OPENMCTIMESTEPPER_H
#define OPENMCTIMESTEPPER_H

#include "TimeStepper.h"

class OpenMCTimeStepper;

template <>
InputParameters validParams<OpenMCTimeStepper>();

class OpenMCTimeStepper : public TimeStepper
{
public:
  OpenMCTimeStepper(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;
  virtual void step() override;
  virtual void postExecute() override;

private:
  Real _dt;
};

#endif // OPENMCTIMESTEPPER_H
