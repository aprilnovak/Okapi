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
};
#endif //OPENMCEXECUTIONER_H
