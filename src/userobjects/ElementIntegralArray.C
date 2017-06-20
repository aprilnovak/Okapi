// MOOSE includes
#include "ElementIntegralArray.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementIntegralArray>()
{
  InputParameters params = validParams<ElementUserObject>();
  return params;
}

ElementIntegralArray::ElementIntegralArray(const InputParameters & parameters)
  : ElementUserObject(parameters), _qp(0)//, _integral_value(0)
{
  _integral_value.assign(10, 0.0);
}

void
ElementIntegralArray::initialize()
{
//  _integral_value = 0;
}

void
ElementIntegralArray::execute()
{
//  _integral_value += computeIntegral();
}

Real
ElementIntegralArray::getValue()
{
//  gatherSum(_integral_value);
//  return _integral_value;
return 0.0;
}

void
ElementIntegralArray::threadJoin(const UserObject & y)
{
//  const ElementIntegralArray & pps = static_cast<const ElementIntegralArray &>(y);
//  _integral_value += pps._integral_value;
}

Real
ElementIntegralArray::computeIntegral()
{
  /*Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();*/
  return 0.0;
}
