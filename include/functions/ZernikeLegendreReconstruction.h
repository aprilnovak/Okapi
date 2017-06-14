#ifndef ZERNIKELEGENDRERECONSTRUCTION_H
#define ZERNIKELEGENDRERECONSTRUCTION_H

#include "Function.h"
#include "math.h"
#include "LegendrePolynomial.h"
#include "ZernikePolynomial.h"

class ZernikeLegendreReconstruction : public Function
{
public:
  ZernikeLegendreReconstruction(const InputParameters & parameters);
  virtual ~ZernikeLegendreReconstruction();

  virtual Real value(Real t, const Point & p);
  
protected:
  int _l_order;
  int _n_order;
  int _l_direction;
  LegendrePolynomial & _legendre_function;
  ZernikePolynomial & _zernike_function;
  std::vector<VariableValue *> _poly_coeffs;
  bool _dbg;
  int _fdir1;
  int _fdir2;
  int _num; 
private:  
    
};

template<>
InputParameters validParams<ZernikeLegendreReconstruction>();

#endif
