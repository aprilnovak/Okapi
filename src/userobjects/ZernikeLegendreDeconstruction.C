#include "ZernikeLegendreDeconstruction.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "ExtraFunctions.h"

template<>
InputParameters validParams<ZernikeLegendreDeconstruction>()
{
  /* This class inherits from a class that uses the BlockRestrictable parameters,
     which means that the variable and/or domain blocks to which this integral is
     to be applied can be specified in the input file using the variable = `` and
     blocks = `` syntax. IMPORTANT: This user object should only be used over
     cylindrical domains! Because a postprocessor is used to compute volume, no
     error will be given if this is used over a non-cylindrical region. */

  // TODO: check that the volume from the postprocessor matches the volume as
  // calculated by the coupled Legendre and Zernike functions.

  InputParameters params = validParams<ElementIntegralUserObject>();
  params.addRequiredCoupledVar("variable", "The variable that will be integrated");
  params.addRequiredParam<std::string>("legendre_function",
    "Name of function to compute Legendre polynomial value at a point.");
  params.addRequiredParam<std::string>("zernike_function",
    "Name of function to compute Zernike polynomial value at a point");
  params.addRequiredParam<int>("l_direction",
    "Direction of integration for Legendre polynomial");
  params.addRequiredParam<int>("l_order", "The order of the Legendre expansion");
  params.addRequiredParam<int>("n_order", "The order of the Zernike expansion");
  params.addRequiredParam<int>("m_order", "The order of the Zernike expansion");
  params.addRequiredParam<std::string>("aux_scalar_name",
    "Aux scalar to store the Legendre expansion coefficient");
  params.addRequiredParam<std::string>("volume_pp",
    "The name of the post processor that calculates volume.");
  return params;
}

ZernikeLegendreDeconstruction::ZernikeLegendreDeconstruction(const InputParameters & parameters) :
    ElementIntegralUserObject(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _legendre_function(dynamic_cast<LegendrePolynomial&>\
      (_mci_feproblem.getFunction(parameters.get<std::string>("legendre_function")))),
    _zernike_function(dynamic_cast<ZernikePolynomial&>\
      (_mci_feproblem.getFunction(parameters.get<std::string>("zernike_function")))),
    _l_direction(parameters.get<int>("l_direction")),
    _l_order(parameters.get<int>("l_order")),
    _n_order(parameters.get<int>("n_order")),
    _m_order(parameters.get<int>("m_order")),
    _aux_scalar_name(parameters.get<std::string>("aux_scalar_name")),
    _volume_pp(getPostprocessorValueByName(parameters.get<std::string>("volume_pp")))
{
  addMooseVariableDependency(mooseVariable());

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

Real
ZernikeLegendreDeconstruction::computeQpIntegral()
{
  Real l_func = _legendre_function.getPolynomialValue(_t, _q_point[_qp](_l_direction), _l_order);
  Real z_func = _zernike_function.getPolynomialValue(_t, _q_point[_qp](_fdir1), _q_point[_qp](_fdir2),\
    _m_order, _n_order);

  return _u[_qp] * l_func * z_func * 2.0 * M_PI / _volume_pp;
}

void
ZernikeLegendreDeconstruction::finalize()
{
  /* In the finalize step, store the result in a SCALAR variable. This user object
     can only set one C_l^{nm} coefficient at a time. The entry in the SCALAR
     variable that is to be filled depends on the values of n and m. */
  MooseVariableScalar & scalar = _fe_problem.getScalarVariable(
    _tid, _aux_scalar_name);
  scalar.reinit();

  std::vector<dof_id_type> & dof = scalar.dofIndices();

  /* Determine the index at which the fixed n-values begin. */
  int n_begin;
  if (_n_order == 0)
    n_begin = 0;
  else
    n_begin = num_zernike(_n_order - 1);

  /* Then, determine the index relative to n_begin at which the m-value belongs. */
  int m_begin = 0;
  for (int m = -_n_order; m <= _n_order; m += 2)
  {
    if (m == _m_order)
      break;
    else
      m_begin += 1;
  }

  scalar.sys().solution().set(dof[n_begin + m_begin], getValue());
  scalar.sys().solution().close();
}
