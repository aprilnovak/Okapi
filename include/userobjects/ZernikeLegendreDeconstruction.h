#ifndef ZERNIKELEGENDREDECONSTRUCTION_H
#define ZERNIKELEGENDREDECONSTRUCTION_H

#include "ElementIntegralUserObject.h"
#include "MooseVariableInterface.h"
#include "LegendrePolynomial.h"
#include "ZernikePolynomial.h"

class ZernikeLegendreDeconstruction;

template<>
InputParameters validParams<ZernikeLegendreDeconstruction>();

class ZernikeLegendreDeconstruction :
  public ElementIntegralUserObject,
  public MooseVariableInterface
{
public:
  ZernikeLegendreDeconstruction(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
  virtual void finalize() override;
    
  const VariableValue & _u;
  LegendrePolynomial & _legendre_function;
  ZernikePolynomial & _zernike_function;
  int _l_direction;
  int _fdir1;
  int _fdir2;
  int _l_order;
  int _n_order;
  int _m_order;
  std::string _aux_scalar_name;
  const PostprocessorValue & _volume_pp;
};

#endif
