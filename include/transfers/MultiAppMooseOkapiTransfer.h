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
  void printResults(std::vector<Real> & results);

protected:
  std::vector<VariableName> _src_var_names;
  std::vector<VariableName> _to_aux_names;
  int32_t _cell;
  const bool & _dbg;
  const bool & _store_results;
  int32_t _index;

  // vectors to hold values for all previous Picard iterations
  std::vector<Real> _k_eff;
  std::vector<Real> _fuel_temp_0;
  std::vector<std::vector<Real>> _fission_coeffs;
};

#endif /* MULTIAPPMOOSEOKAPITRANSFER_H */
