#include "ElementIntegralArray.h"
#include "ExtraFunctions.h"
#include "libmesh/quadrature.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<ElementIntegralArray>()
{
  /* This user object computes the expansion coefficients into which a continuous MOOSE
     variable is decomposed for transfer _to_ OpenMC. */
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<int>("l_order_to_openmc", "Order of Legendre expansion used in OpenMC");
  params.addRequiredParam<int>("n_order_to_openmc", "Order of Zernike expansion used in OpenMC");
  params.addRequiredParam<std::string>("legendre_function", \
    "Name of function to compute Legendre polynomial value at a point.");
  params.addRequiredParam<std::string>("zernike_function", \
    "Name of function to compute Zernike polynomial value at a point");
  params.addRequiredParam<int>("l_direction", \
    "Direction of integration for Legendre polynomial");
  return params;
}

ElementIntegralArray::ElementIntegralArray(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _qp(0),
    _l_order(getParam<int>("l_order_to_openmc")),
    _n_order(getParam<int>("n_order_to_openmc")),
    _legendre_function(dynamic_cast<LegendrePolynomial&>\
      (_mci_feproblem.getFunction(parameters.get<std::string>("legendre_function")))),
    _zernike_function(dynamic_cast<ZernikePolynomial&>\
      (_mci_feproblem.getFunction(parameters.get<std::string>("zernike_function")))),
    _l_direction(getParam<int>("l_direction"))
{
  _num_entries = num_zernike(_n_order);
  _integral_value.assign(_num_entries, 0.0);

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
}

void
ElementIntegralArray::initialize()
{
  _integral_value.assign(_num_entries, 0.0);
}

// called only once, so this computes _all_ of the integrals
void
ElementIntegralArray::execute()
{
  int i = 0;
  for (int n = 0; n <= _n_order; ++n)
  {
    for (int m = -n; m <= n; m += 2)
    {
      _integral_value[i] += computeIntegral(_l_order, m, n);
      i += 1;
    }
  }
}

// return the n-th integral
Real
ElementIntegralArray::getValue(int N)
{
  gatherSum(_integral_value[N]);
  return _integral_value[N];
}

void
ElementIntegralArray::threadJoin(const UserObject & y)
{
  for (int i = 0; i < _num_entries; ++i)
  {
    const ElementIntegralArray & pps = static_cast<const ElementIntegralArray &>(y);
    _integral_value[i] += pps._integral_value[i];
  }
}

// Compute the n-th integral
Real
ElementIntegralArray::computeIntegral(int l, int m, int n)
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral(l, m, n);
  return sum;
}

Real
ElementIntegralArray::computeQpIntegral(int l, int m, int n)
{
  Real l_func = _legendre_function.getPolynomialValue(_t, _q_point[_qp](_l_direction), l);
  Real z_func = _zernike_function.getPolynomialValue(_t, _q_point[_qp](_fdir1), _q_point[_qp](_fdir2), m, n);

  /*std::cout << "l_func: " << l_func << std::endl;
  std::cout << "z_func: " << z_func << std::endl;
  std::cout << "volume: " << _volume_pp << std::endl;
  std::cout << "pi: " << M_PI << std::endl;*/
  return 1.0;
//  return _u[_qp] * l_func * z_func * 2.0 * M_PI / _volume_pp;
}
