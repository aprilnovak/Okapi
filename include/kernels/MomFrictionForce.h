#ifndef MOMFRICTIONFORCE_H
#define MOMFRICTIONFORCE_H

#include "Material.h"
#include "Reaction.h"

class MomFrictionForce;

template<>
InputParameters validParams<MomFrictionForce>();

class MomFrictionForce : public Reaction
{
public:
  MomFrictionForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _W;
};

#endif //MOMFRICTIONFORCE_H
