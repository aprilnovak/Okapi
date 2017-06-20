#include "ElementIntegralArray.h"
#include "ExtraFunctions.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementIntegralArray>()
{
  /* This user object computes the expansion coefficients into which a continuous MOOSE
     variable is decomposed for transfer _to_ OpenMC. */
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<int>("l_order_to_openmc", "Order of Legendre expansion used in OpenMC");
  params.addRequiredParam<int>("n_order_to_openmc", "Order of Zernike expansion used in OpenMC");
  return params;
}

ElementIntegralArray::ElementIntegralArray(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _qp(0),
    _l_order(getParam<int>("l_order_to_openmc")),
    _n_order(getParam<int>("n_order_to_openmc"))
{
  _num_entries = num_zernike(_n_order);
  _integral_value.assign(_num_entries, 0.0);
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
