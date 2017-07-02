#ifndef POLYNOMIALOPENMC_H
#define POLYNOMIALOPENMC_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declerations
class PolynomialOpenMC;

template<>
InputParameters validParams<PolynomialOpenMC>();

/**
 * Copies the value of a Postprocessor from one app to a scalar AuxVariable in another.
 */
class PolynomialOpenMC :
  public MultiAppTransfer
{
public:
  PolynomialOpenMC(const InputParameters & parameters);

  virtual void execute() override;

protected:
  std::vector<VariableName> _source_var_names;
  std::vector<VariableName> _to_aux_names;
  Real _radius;
  std::vector<Real> _center;
  std::vector<Real> _geom_norm;
  int _l_direction;
  int _cell;
};

#endif /* POLYNOMIALOPENMC_H */
