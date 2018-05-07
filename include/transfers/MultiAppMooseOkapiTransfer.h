#ifndef MULTIAPPMOOSEOKAPITRANSFER_H
#define MULTIAPPMOOSEOKAPITRANSFER_H

// MOOSE includes
#include "MultiAppFXTransfer.h"

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

protected:
  int32_t _cell;
  int32_t _tally;
  const bool & _dbg;
  const bool & _store_results;
  int32_t _cell_index;
  int32_t _tally_index;

  // vectors to hold values for all previous Picard iterations
  std::vector<Real> _k_eff;
  std::vector<std::vector<Real>> _fuel_temp_coeffs;
  std::vector<std::vector<Real>> _fission_coeffs;
};

#endif /* MULTIAPPMOOSEOKAPITRANSFER_H */
