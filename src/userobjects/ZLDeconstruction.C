// Okapi includes
#include "ZLDeconstruction.h"
#include "ExtraFunctions.h"

// MOOSE includes
#include "MooseVariableScalar.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ZLDeconstruction>()
{
  /* This user object computes the expansion coefficients into which a continuous
     MOOSE variable is decomposed for a single value of l. */
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredCoupledVar("variable", "The variable that will be integrated");
  params.addRequiredParam<int>("l_order", "Order of Legendre expansion.");
  params.addRequiredParam<int>("n_order",
    "Order of Zernike expansion for current set of coefficients.");
  params.addRequiredParam<std::string>("legendre_function", \
    "Name of function to compute Legendre polynomial value at a point.");
  params.addRequiredParam<std::string>("zernike_function", \
    "Name of function to compute Zernike polynomial value at a point");
  params.addRequiredParam<int>("l_direction", \
    "Direction of integration for Legendre polynomial");
   params.addRequiredParam<std::string>("aux_scalar_name", \
     "Aux scalar to store the expansion coefficients.");
   params.addRequiredParam<std::string>("volume_pp",
     "The name of the post processor that calculates volume.");
 return params;
}

ZLDeconstruction::ZLDeconstruction(const InputParameters & parameters)
  : ElementUserObject(parameters),
    MooseVariableInterface(this, false),
    _qp(0),
    _u(coupledValue("variable")),
    _l_order(getParam<int>("l_order")),
    _n_order(getParam<int>("n_order")),
    _legendre_function(dynamic_cast<LegendrePolynomial&>\
      (_mci_feproblem.getFunction(parameters.get<std::string>("legendre_function")))),
    _zernike_function(dynamic_cast<ZernikePolynomial&>\
      (_mci_feproblem.getFunction(parameters.get<std::string>("zernike_function")))),
    _l_direction(getParam<int>("l_direction")),
    _aux_scalar_name(parameters.get<std::string>("aux_scalar_name")),
    _volume_pp(getPostprocessorValueByName(parameters.get<std::string>("volume_pp")))
{
  addMooseVariableDependency(mooseVariable());

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
ZLDeconstruction::initialize()
{
  _integral_value.assign(_num_entries, 0.0);
}

// computes _all_ of the integrals
void
ZLDeconstruction::execute()
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
ZLDeconstruction::getValue(int N)
{
  gatherSum(_integral_value[N]);
  return _integral_value[N];
}

void
ZLDeconstruction::threadJoin(const UserObject & y)
{
  for (int i = 0; i < _num_entries; ++i)
  {
    const ZLDeconstruction & pps = static_cast<const ZLDeconstruction &>(y);
    _integral_value[i] += pps._integral_value[i];
  }
}

// Compute the n-th integral
Real
ZLDeconstruction::computeIntegral(int l, int m, int n)
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral(l, m, n);
  return sum;
}

Real
ZLDeconstruction::computeQpIntegral(int l, int m, int n)
{
  Real l_func = _legendre_function.getPolynomialValue(_t,
    _q_point[_qp](_l_direction), l);
  Real z_func = _zernike_function.getPolynomialValue(_t,
    _q_point[_qp](_fdir1), _q_point[_qp](_fdir2), m, n);

  return _u[_qp] * l_func * z_func * 2.0 * M_PI / _volume_pp;
}

void
ZLDeconstruction::finalize()
{
  /* Store the result of the user object in a SCALAR variable.*/
  MooseVariableScalar & scalar =
    _fe_problem.getScalarVariable(_tid, _aux_scalar_name);
  scalar.reinit();

  std::vector<dof_id_type> & dof = scalar.dofIndices();

  for (int i = 0; i < _num_entries; ++i)
    scalar.sys().solution().set(dof[i], getValue(i));

  scalar.sys().solution().close();
}
