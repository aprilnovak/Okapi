#ifndef MULTIAPPMOOSEOKAPIREACTIVITYTRANSFER_H
#define MULTIAPPMOOSEOKAPIREACTIVITYTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

class MultiAppMooseOkapiReactivityTransfer;

template<>
InputParameters validParams<MultiAppMooseOkapiReactivityTransfer>();

class MultiAppMooseOkapiReactivityTransfer :
  public MultiAppTransfer
{
public:
  MultiAppMooseOkapiReactivityTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  VariableName _to_aux_names;
  bool _dbg;
};

#endif /* MULTIAPPMOOSEOKAPIREACTIVITYTRANSFER_H */
