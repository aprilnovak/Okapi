#include "ZernikeLegendreDeconstruction.h"

template<>
InputParameters validParams<ZernikeLegendreDeconstruction>()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();
  params.addRequiredParam<std::string>("legendre_function", "Name of function to compute Legendre polynomial value at a point");
  params.addRequiredParam<std::string>("zernike_function", "Name of function to compute Fourier polynomial value at a point");
  params.addRequiredParam<int>("l_direction", "Direction of integration for Legendre polynomial");
  params.addRequiredParam<int>("l_order", "The order of the Legendre expansion");
  params.addRequiredParam<int>("f_order", "The order of the Fourier expansion");
  params.addRequiredParam<std::string>("aux_scalar_name", "Aux scalar to store the Legendre expansion coefficient");
  params.addParam<MaterialPropertyName>("diffusion_coefficient_name",
                                        "thermal_conductivity",
                                        "Property name of the diffusivity (Default: thermal_conductivity)");
  params.addRequiredParam<std::string>("surface_area_pp", "The name of the post processor that calculates surface area");
  return params;
}

ZernikeLegendreDeconstruction::ZernikeLegendreDeconstruction(const InputParameters & parameters) :
    // TODO we really shouldn't have to dynamic cast into FourierPolynomial and Legendre Polynomial here
    // but this was a quick example
    ElementIntegralVariableUserObject(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _legendre_poly_function(dynamic_cast<LegendrePolynomial&>(_mci_feproblem.getFunction(parameters.get<std::string>("legendre_function_name")))),
    _fourier_poly_function(dynamic_cast<FourierPolynomial&>(_mci_feproblem.getFunction(parameters.get<std::string>("fourier_function_name")))),
    _l_direction(parameters.get<int>("l_direction")),
    _l_order(parameters.get<int>("l_order")),
    _f_order(parameters.get<int>("f_order")),
    _aux_scalar_name(parameters.get<std::string>("aux_scalar_name")),
    _coupled_grad(coupledGradient("variable")),
    _diffusion_coefficient(getMaterialProperty<Real>("diffusion_coefficient_name")),
    _surface_area_pp(getPostprocessorValueByName(parameters.get<std::string>("surface_area_pp")))
                            
{
  addMooseVariableDependency(mooseVariable());

  // For now hard code that l_direction better be 2
  // TODO come back and put logic in to determine f diections given l directions
  if ( _l_direction == 2)
  {
    _f_direction_1 = 0;
    _f_direction_2 = 1;
  }
  else
  {
    mooseError("Need to implement logic for l direction not equal to 2");
  }
  
  
}

Real
ZernikeLegendreDeconstruction::computeQpIntegral()
{
  Real l_poly_val = _legendre_poly_function.getPolynomialValue(_t,_q_point[_qp](_l_direction),_l_order);
  Real f_poly_val = _fourier_poly_function.getPolynomialValue(_t,_q_point[_qp](_f_direction_1),_q_point[_qp](_f_direction_2), _f_order);
  // There is an added correction factor that accounts for the surface area of the pin
  // The analytic expression is (2 / R / delta z) but we compute numerically as
  // (2 / (Surf Area / (2*PI) ) ) = 4PI / Surf Area
  return -_diffusion_coefficient[_qp] * _coupled_grad[_qp].norm() * l_poly_val * f_poly_val * 4.0*M_PI / _surface_area_pp;
}
void
ZernikeLegendreDeconstruction::finalize()
{
  // In the finalize step store the result of the integral in the scalar variable
  MooseVariableScalar & scalar =  _fe_problem.getScalarVariable(_tid, _aux_scalar_name);
  scalar.reinit();
  // The dof indices for the scalar variable
  std::vector<dof_id_type> & dof = scalar.dofIndices();
  scalar.sys().solution().set(dof[_l_order], getValue());
  scalar.sys().solution().close();
}
