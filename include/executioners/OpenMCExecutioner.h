#ifndef OPENMCEXECUTIONER_H
#define OPENMCEXECUTIONER_H

#include "Transient.h"
class OpenMCExecutioner;

template<>
InputParameters validParams<OpenMCExecutioner>();

class OpenMCExecutioner : public Transient
{
  public:
    OpenMCExecutioner(const InputParameters & parameters);

    virtual void init();


    virtual void preStep();
//  virtual void takeStep(Real input_dt = -1.0);
    virtual void postStep();

//  virtual bool lastSolveConverged();

};
#endif //OPENMCEXECUTIONER_H
