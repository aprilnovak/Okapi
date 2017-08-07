#ifndef MULTIAPPMOOSEOKAPITRANSFER_H
#define MULTIAPPMOOSEOKAPITRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

class MultiAppMooseOkapiTransfer;

template<>
InputParameters validParams<MultiAppMooseOkapiTransfer>();

class MultiAppMooseOkapiTransfer :
  public MultiAppTransfer
{
public:
  MultiAppMooseOkapiTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  std::vector<VariableName> _src_var_names;
  std::vector<VariableName> _to_aux_names;
  int32_t _cell;
  bool _dbg;
  int32_t _index;
};

#endif /* MULTIAPPMOOSEOKAPITRANSFER_H */
