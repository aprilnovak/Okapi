#include "LegendrePolynomial.h"

template<>
InputParameters validParams<LegendrePolynomial>()
{
  InputParameters params = validParams<Function>();
  /* This parameter is used for normalizing the Legendre polynomials,
     which are only orthogonal on [-1, 1]. However, the Cartesian
     domain over which we'd like to use FETs may exist over a 
     different range. So, just scale the polynomial appropriately. 
     Care should be taken that this value matches that given in the
     OpenMC input XML files. */
  params.addRequiredParam<std::vector<Real>>("l_geom_norm","Lengths needed for Legendre polynomial normalization (min, max)");
  params.addParam<bool>("dbg", false, "Print debug output");
  return params;
}

LegendrePolynomial::LegendrePolynomial(const InputParameters & parameters) :
    Function(parameters),
    _geom_norm(parameters.get<std::vector<Real>>("l_geom_norm")),
    _dbg(parameters.get<bool>("dbg"))
{
  // actual domain over which we want orthogonal polynomials
  _dz = _geom_norm[1] - _geom_norm[0];
}

LegendrePolynomial::~LegendrePolynomial()
{
}

Real
LegendrePolynomial::value(Real t, const Point & p)
{
  /* Because we need to pass in the order of the Legendre polynomial,
     the parameter list for this method does not suffice. */
  mooseWarning("value() in LegendrePolynomial should not be used");
  return 0.0;
}

Real
LegendrePolynomial::getPolynomialValue(Real t, Real p, int n)
{
  /* The Legendre polynomials are computed with a recurion relation
     that uses the P_{L-1} and P_{L-2} Legendre polynomials. This
     method returns the Legendre polynomials normalized by 
     \sqrt{(2*l + 1)/2} so that the calculation of the expansion
     coefficients does not need to carry these factors around. */

  Real z;          // Normalized position
  Real plm2 = 0.0; // L-2 Legendre polynomial value
  Real plm1 = 0.0; // L-1 Legendre polynomial value
  Real plm = 0.0;  // L   Legendre polynomial value
  
  z = 2.0 * (p - _geom_norm[0])/_dz - 1.0;

  if(_dbg)
  {
    Moose::out<<"point: " << p << std::endl;
    Moose::out<<"dz = "<< _dz << std::endl;
    Moose::out<<"_geom_norm[0] = "<<_geom_norm[0]<<std::endl;
    Moose::out<<"Normalized z = "<<z<<std::endl;
  }

  /* The recursion relation can only be used for order 2 and above. */
  if (n == 0)
    return 1.0 / sqrt(2.0);
  else if (n == 1)
    return sqrt(3.0 / 2.0) * z;
  else
  {
    plm2 = 1.0;
    plm1 = z;
    for (int ii = 2; ii <= n; ++ii)
    {
      plm = (z * (2.0 * ii - 1.0) * plm1) / ii - ((ii - 1.0) * plm2) / ii;
      plm2 = plm1;
      plm1 = plm;
    }

    if (_dbg)
      Moose::out << "Legendre total value  = " << plm * (2.0 * n + 1.0) / _dz << std::endl;
    
    return (plm * sqrt((2.0 * n + 1.0) / 2.0));
  }
}
