#ifndef ZLDECONSTRUCTION_H
#define ZLDECONSTRUCTION_H

// MOOSE includes
#include "MooseVariableInterface.h"

// Okapi includes
#include "ElementUserObject.h"
#include "LegendrePolynomial.h"
#include "ZernikePolynomial.h"

class ZLDeconstruction;

template <>
InputParameters validParams<ZLDeconstruction>();

class ZLDeconstruction : public ElementUserObject, public MooseVariableInterface
{
public:
  ZLDeconstruction(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  virtual Real getValue(int);

protected:
  virtual Real computeQpIntegral(int, int, int);
  virtual Real computeIntegral(int, int, int);

  unsigned int _qp;
  const VariableValue & _u;
  int _l_order;
  int _n_order;
  int _num_entries;
  LegendrePolynomial & _legendre_function;
  ZernikePolynomial & _zernike_function;
  int _l_direction;
  int _fdir1;
  int _fdir2;

  std::string _aux_scalar_name;
  const PostprocessorValue & _volume_pp;

  std::vector<Real> _integral_value;
};

#endif
