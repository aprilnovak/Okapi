/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "OpenMCTempScalarComponentIC.h"
#include "OpenMCInterface.h"

#include "MooseVariableScalar.h"

template <>
InputParameters
validParams<OpenMCTempScalarComponentIC>()
{
  InputParameters params = validParams<ScalarInitialCondition>();
  params.addRequiredParam<std::vector<Real>>("values",
    "Initial values to initialize the scalar variable.");
  params.addRequiredParam<int>("openmc_cell", "Cell in OpenMC to set initial "
    "temperature for.");
  params.addRequiredParam<int>("l_order", "Legendre order corresponding to this "
    "variable, which is used to determine which section of the OpenMC data "
    "structures to populate.");
  params.addParam<bool>("dbg", false, "Whether to provide debugging information.");
  return params;
}

OpenMCTempScalarComponentIC::OpenMCTempScalarComponentIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters),
    _initial_values(getParam<std::vector<Real>>("values")),
    _cell(parameters.get<int>("openmc_cell")),
    _l_order(parameters.get<int>("l_order")),
    _dbg(parameters.get<bool>("dbg"))
{
  if (_initial_values.size() != _var.order())
    mooseError("The initial vector values size given to the scalar variable '",
               name(), "' has wrong size.");
}

Real
OpenMCTempScalarComponentIC::value()
{
  if (_dbg)
  {
    _console << "Setting temperature initial conditions in OpenMC cell " <<
      _cell << "to: " << std::endl;
    for (int i = 0; i < _initial_values.size(); ++i)
      _console << _initial_values[_i] << ' ';
    _console << '\n';
  }

  return 0.0;
//  return _initial_values[_i];
}
