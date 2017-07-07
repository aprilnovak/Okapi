#ifndef KAPPAFISSIONTOHEATSOURCE_H
#define KAPPAFISSIONTOHEATSOURCE_H

#include "AuxKernel.h"

class KappaFissionToHeatSource;

template<>
InputParameters validParams<KappaFissionToHeatSource>();

class KappaFissionToHeatSource : public AuxKernel
{
public:
  KappaFissionToHeatSource(const InputParameters & parameters);

  virtual ~KappaFissionToHeatSource();

protected:
  virtual Real computeValue();
  const VariableValue & _kappa_fission;
  Real _power;
  const PostprocessorValue & _kappa_fission_pp;
  const PostprocessorValue & _volume_pp;


};

#endif //KAPPAFISSIONTOHEATSOURCE_H
