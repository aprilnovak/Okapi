#ifdef ENABLE_NEK_COUPLING

#ifndef MULTIAPPMOONOKAPITRANSFER_H
#define MULTIAPPMOONOKAPITRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

class MultiAppMoonOkapiTransfer;

template <>
InputParameters validParams<MultiAppMoonOkapiTransfer>();

class MultiAppMoonOkapiTransfer : public MultiAppTransfer
{
public:
  MultiAppMoonOkapiTransfer(const InputParameters & parameters);
  virtual void execute() override;
  void printResults(std::vector<Real> & results);

protected:
  std::vector<VariableName> _source_var_names;
  std::vector<VariableName> _to_aux_names;
  bool _dbg;
  const std::vector<int32_t> & _cell;
  const std::vector<int32_t> & _material;
  std::vector<int32_t> _index;
  std::vector<int32_t> _index_mat;

  const Real & _T_inlet;
  const Real & _T_outlet;
  const bool & _store_results;

  std::vector<std::vector<Real>> _fluid_layer_temps;
};

#endif /* MULTIAPPMOONOKAPITRANSFER_H */

#endif // ENABLE_NEK_COUPLING
