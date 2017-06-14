#include "ZernikeLegendreReconstruction.h"
#include "ExtraFunctions.h"

template<>
InputParameters validParams<ZernikeLegendreReconstruction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<int>("l_order", "The order of the Legendre expansion.");
  params.addRequiredParam<int>("n_order", "The N-order of the Zernike expansion.");
  params.addRequiredParam<int>("l_direction", "The coordinate direction in which\
    the Legendre expansion is applied (0 = x, 1 = y, 2 = z).");
  params.addRequiredParam<std::string>("legendre_function", "Name of the function\
    to compute the Legendre polynomial at a point.");
  params.addRequiredParam<std::string>("zernike_function", "Name of the function\
    to compute the Legendre polynomial at a point.");
  params.addRequiredCoupledVar("poly_coeffs", "Name of the aux variable containing\
    the Zernike-Legendre expansion coefficients.");
  params.addParam<bool>("dbg", false, "Print debug output");
  return params;
}

ZernikeLegendreReconstruction::ZernikeLegendreReconstruction(const InputParameters & parameters) :
    Function(parameters),
    _l_order(parameters.get<int>("l_order")),
    _n_order(parameters.get<int>("n_order")),
    _l_direction(parameters.get<int>("l_direction")),
    _legendre_function(dynamic_cast<LegendrePolynomial&>(_mci_feproblem.getFunction(parameters.get<std::string>("legendre_function")))),
    _zernike_function(dynamic_cast<ZernikePolynomial&>(_mci_feproblem.getFunction(parameters.get<std::string>("zernike_function")))),
    _dbg(parameters.get<bool>("dbg"))
{
  int _fdir1, _fdir2;

  if (_l_direction == 0) // Legendre in x-direction, Zernike in y-z
  {
    _fdir1 = 1;
    _fdir2 = 2;
  }
  else if (_l_direction == 1) // Legendre in y-direction, Zernike in x-z
  {
    _fdir1 = 0;
    _fdir2 = 2;
  }
  else // Legendre in z-direction, Zernike in x-y
  {
    _fdir1 = 0;
    _fdir2 = 1;
  }

  /* The coupled variables poly_coeffs are a vector that we need to store 
     into a member variable. This variable is assumed to hold all of the 
     Zernike coefficients for a given Legendre order. The number of 
     values in this vector will depend on the order of the Zernike 
     polynomial, but is not as simple as calculating n * m, since m can 
     only have values -n, -n + 2, -n + 4, ..., n. For a fixed Legendre
     order, there are n + 1 Zernike polynomials for any particular n, and
     0 <= n <= N. These coefficients are assumed to be ordered as (n, m), 
     such that the coefficients correspond to Z_0^0, Z_1^{-1}, Z_1^1, ... */
  int _num = num_zernike(_n_order);
  for (int i = 0; i < _num; ++i)
    _poly_coeffs.push_back(&coupledScalarValue("poly_coeffs", i));
}

ZernikeLegendreReconstruction::~ZernikeLegendreReconstruction()
{
}

Real
ZernikeLegendreReconstruction::value(Real t, const Point & p)
{
  Real val = 0.0;
  int entry;
  for (int l = 0; l < _l_order; ++l)
  {
    for (int n = 0; n < _num; ++n)
    {
      entry = 0;
      for (int m = -n; m <= n; m += 2)
      {
        val += (*_poly_coeffs[l])[entry] * \
             _legendre_function.getPolynomialValue(t, p(_l_direction), l) * \
             _zernike_function.getPolynomialValue(t, p(_fdir1), p(_fdir2), n, m);
        entry += 1;
      }
    }
  } 

  return val;
}
