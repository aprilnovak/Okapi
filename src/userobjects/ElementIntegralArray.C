// MOOSE includes
#include "ElementIntegralArray.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementIntegralArray>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<int>("num", "Length of array to compute");
  return params;
}

ElementIntegralArray::ElementIntegralArray(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _qp(0),
    _num(getParam<int>("num"))
{
  _integral_value.assign(_num, 0.0);
}

void
ElementIntegralArray::initialize()
{
  _integral_value.assign(_num, 0.0);
}

// called only once, so this computes _all_ of the integrals
void
ElementIntegralArray::execute()
{
  for (int i = 0; i < _num; ++i)
  {
    _integral_value[i] += computeIntegral(i);
  }
}

// return the n-th integral
Real
ElementIntegralArray::getValue(int n)
{
  gatherSum(_integral_value[n]);
  return _integral_value[n];
}

void
ElementIntegralArray::threadJoin(const UserObject & y)
{
  for (int i = 0; i < _num; ++i)
  {
    const ElementIntegralArray & pps = static_cast<const ElementIntegralArray &>(y);
    _integral_value[i] += pps._integral_value[i];
  }
}

// Compute the n-th integral
Real
ElementIntegralArray::computeIntegral(int n)
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral(n);
  return sum;
}
