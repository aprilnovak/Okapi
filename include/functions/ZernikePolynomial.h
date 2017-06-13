#ifndef ZERNIKEPOLYNOMIAL_H
#define ZERNIKEPOLYNOMIAL_H

#include "Function.h"
#include "math.h"

class ZernikePolynomial : public Function
{
public:
  ZernikePolynomial(const InputParameters & parameters);
  virtual ~ZernikePolynomial();

  virtual Real value(Real t, const Point & p);
  Real getPolynomialValue(Real t, Real x, Real y, int m, int n);
  
protected:

private:
  
  Real _radius;
  std::vector<Real> _center;
  bool _dbg;
};

template<>
InputParameters validParams<ZernikePolynomial>();

#endif
