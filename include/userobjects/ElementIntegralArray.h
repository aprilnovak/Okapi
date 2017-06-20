#ifndef ELEMENTINTEGRALARRAY_H
#define ELEMENTINTEGRALARRAY_H

// MOOSE includes
#include "ElementUserObject.h"

// Forward Declarations
class ElementIntegralArray;

template <>
InputParameters validParams<ElementIntegralArray>();

/**
 * This postprocessor computes a volume integral of the specified
 * variable.
 *
 * Note that specializations of this integral are possible by deriving
 * from this class and overriding computeQpIntegral().
 */
class ElementIntegralArray : public ElementUserObject
{
public:
  ElementIntegralArray(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override {}

  /// Returns the integral value
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral() = 0;
  virtual Real computeIntegral();

  unsigned int _qp;

  std::vector<Real> _integral_value;
};

#endif
