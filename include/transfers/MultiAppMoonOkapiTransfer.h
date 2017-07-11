#ifndef MULTIAPPMOONOKAPITRANSFER_H
#define MULTIAPPMOONOKAPITRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declerations
class MultiAppMoonOkapiTransfer;

template<>
InputParameters validParams<MultiAppMoonOkapiTransfer>();

/**
 * Copies the value of a Postprocessor from one app to a scalar AuxVariable in another.
 */
class MultiAppMoonOkapiTransfer :
  public MultiAppTransfer
{
public:
  MultiAppMoonOkapiTransfer(const InputParameters & parameters);

  /**
   * Execute the transfer
   */
  virtual void execute() override;

protected:
  std::vector<VariableName> _source_var_names;
  std::vector<VariableName> _to_aux_names;
  bool _dbg;
};

#endif /* MULTIAPPMOONOKAPITRANSFER_H */
