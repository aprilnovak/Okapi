#ifndef POLYNOMIALOPENMC_H
#define POLYNOMIALOPENMC_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declerations
class PolynomialOpenMC;

template<>
InputParameters validParams<PolynomialOpenMC>();

class PolynomialOpenMC :
  public MultiAppTransfer
{
public:
  PolynomialOpenMC(const InputParameters & parameters);

  virtual void execute() override;

protected:
  std::vector<VariableName> _source_var_names;
  std::vector<VariableName> _to_aux_names;
  int _cell;
  bool _dbg;
};

#endif /* POLYNOMIALOPENMC_H */
