#ifndef MULTIAPPMOOSEOKAPITRANSFER_H
#define MULTIAPPMOOSEOKAPITRANSFER_H

// MOOSE includes
#include "MultiAppFXTransfer.h"
#include "CellInstance.h"

class MultiAppMooseOkapiTransfer;

template <>
InputParameters validParams<MultiAppMooseOkapiTransfer>();

class MultiAppMooseOkapiTransfer : public MultiAppFXTransfer
{
public:
  MultiAppMooseOkapiTransfer(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialSetup() override;
  void printResults(std::vector<Real> & results);

protected:
  bool getOrderAndCheckExpansionType(const char * type, const int32_t & index, int32_t & order);
  void runChecks();

protected:
  std::vector<CellInstance> _cells;
  std::unordered_map<int32_t, int32_t> _cell_to_shell;
  int32_t _tally;
  const bool & _dbg;
  const bool & _store_results;
  int32_t _cell_index;
  int32_t _tally_index;

  // vectors to hold values for all previous Picard iterations
  std::vector<Real> _k_eff;
  std::vector<std::vector<Real>> _fuel_temp_coeffs;
  std::vector<std::vector<Real>> _fission_coeffs;

  const Real _geometry_multiplier;
  bool _checks_done;
  int32_t _num_cells_in_filter;
  int32_t _stride_integer;
  int32_t _cell_filter_index;
};

#endif /* MULTIAPPMOOSEOKAPITRANSFER_H */
