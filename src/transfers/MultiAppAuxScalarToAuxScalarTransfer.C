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

#include "MultiAppAuxScalarToAuxScalarTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "MultiApp.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

// Define the input parameters
template <>
InputParameters
validParams<MultiAppAuxScalarToAuxScalarTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<VariableName>(
      "from_aux_scalar",
      "The name of the scalar Aux variable in the MultiApp to transfer the value from.");
  params.addRequiredParam<VariableName>(
      "to_aux_scalar",
      "The name of the scalar Aux variable in the MultiApp to transfer the value to.");
  return params;
}

MultiAppAuxScalarToAuxScalarTransfer::MultiAppAuxScalarToAuxScalarTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_aux_name(getParam<VariableName>("from_aux_scalar")),
    _to_aux_name(getParam<VariableName>("to_aux_scalar"))
{
}

void
MultiAppAuxScalarToAuxScalarTransfer::execute()
{
  _console << "Beginning PostprocessorToAuxScalarTransfer " << name() << std::endl;

  // Perform action based on the transfer direction
  switch (_direction)
  {
    // MasterApp -> SubApp
    case TO_MULTIAPP:
    {
      // Extract the scalar variable that is being transferred
      FEProblemBase & from_problem = _multi_app->problemBase();
      MooseVariableScalar * from_variable =
        &from_problem.getScalarVariable(_tid, _from_aux_name);

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
        {
          // Get reference to the scalar variable that will be written
          MooseVariableScalar * to_variable =
              &_multi_app->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

          to_variable->reinit();

          // Determine number of DOFs that we're going to read and write
          std::vector<dof_id_type> & to_dof = to_variable->dofIndices();
          auto & from_values = from_variable->sln();

          // Check that the DOF matches
          if (from_variable->sln().size() != to_variable->sln().size())
            mooseError("Order of SCALAR AuxVariabls do not match for sending and\
receiving data for the MultiAppAuxScalarToAuxScalarTransfer!");

          for (auto j = beginIndex(from_values); j < from_values.size(); ++j)
          {
            to_variable->sys().solution().set(to_dof[j], from_values[j]);
            to_variable->sys().solution().close();
          }
        }
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      // The number of sub applications
      unsigned int num_apps = _multi_app->numGlobalApps();

      // The AuxVariable that will be read from the subApp
      MooseVariableScalar * to_variable =
          &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_name);

      // Ensure that the variable is up to date
      to_variable->reinit();

      // The dof indices for the scalar variable of interest
      std::vector<dof_id_type> & to_dof = to_variable->dofIndices();

      // Loop over each sub-app and populate the AuxVariable values
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
        {
          // Extract the scalar variable that is being transferred
          FEProblemBase & from_problem = _multi_app->appProblemBase(i);
          MooseVariableScalar * from_variable =
            &from_problem.getScalarVariable(_tid, _from_aux_name);

          // Loop over the scalar aux variable that we're going to write
          auto & from_values = from_variable->sln();

          // Check that DOFs match
          if (from_variable->sln().size() != to_variable->sln().size())
            mooseError("Order of SCALAR AuxVariables do not match for sending and\
receiving data for the MultiAppAuxScalarToAuxScalarTransfer!");

          for (auto j = beginIndex(from_values); j < from_values.size(); ++j)
          {
            to_variable->sys().solution().set(to_dof[j], from_values[j]);
            to_variable->sys().solution().close();
          }
        }
      }

      break;
    }
  }

  _console << "Finished PostprocessorToAuxScalarTransfer " << name() << std::endl;
}
