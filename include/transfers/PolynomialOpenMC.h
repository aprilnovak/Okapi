#ifndef MULTIAPPPOLYNOMIALTONEK_H
#define MULTIAPPPOLYNOMIALTONEK_H

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

  /**
   * Execute the transfer
   */
  virtual void execute() override;

protected:
  std::vector<VariableName> _source_var_names;
  std::vector<VariableName> _to_aux_names;
};

#endif /* MULTIAPPPOLYNOMIALTONEK_H */
