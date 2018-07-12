#include "MultiAppMooseOkapiReactivityTransfer.h"
#include "OpenMCErrorHandling.h"
#include "openmc.h"

#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "SubProblem.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template <>
InputParameters
validParams<MultiAppMooseOkapiReactivityTransfer>()
{
  /* This is used to transfer data between Okapi and a generic MOOSE
     application when the MOOSE application is the sub App and Okapi
     the Master App. */
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<VariableName>(
      "to_aux_scalar", "The name of the SCALAR auxvariable to transfer the value to.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  return params;
}

MultiAppMooseOkapiReactivityTransfer::MultiAppMooseOkapiReactivityTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_aux_names(getParam<VariableName>("to_aux_scalar")),
    _dbg(parameters.get<bool>("dbg"))
{
}

void
MultiAppMooseOkapiReactivityTransfer::execute()
{
  _console << "Beginning MultiAppMooseOkapiReactivityTransfer Transfer " << name() << std::endl;

  unsigned int num_apps = _multi_app->numGlobalApps();

  switch (_direction)
  {
    // MOOSE -> Okapi. No reactivity information is passed from MOOSE to Okapi.
    case FROM_MULTIAPP:
    {
      mooseError("The 'MultiAppMooseOkapiReactivityTransfer' should only "
                 "be used for the direction 'TO_MULTIAPP'!");
      break;
    }

    // Okapi -> MOOSE. This transfer is used to transfer k_eff from Okapi to
    // MOOSE, where it can be used for computing approximate power changes.
    case TO_MULTIAPP:
    {
      for (unsigned int I = 0; I < num_apps; ++I)
        if (_multi_app->hasLocalApp(I))
        {
          // initialize variables we're going to write
          MooseVariableScalar * to_vars =
              &_multi_app->appProblemBase(I).getScalarVariable(_tid, _to_aux_names);
          to_vars->reinit();

          // get k_eff from OpenMC
          double keff[2];
          int err_keff = openmc_get_keff(keff);
          ErrorHandling::openmc_get_keff(err_keff);

          if (_dbg)
            _console << "Sending k_eff value of: " << keff[0] << " from OpenMC to MOOSE!"
                     << std::endl;

          // we will use this value for keff in a PKE approximation, so print a
          // warning if the reactivity is greater than 0.00645 (beta)
          if ((keff[0] - 1.0) / keff[0] >= 0.00645)
            mooseWarning("Reactivity is greater than 'beta'! Results for changes "
                         "in fission power will be inaccurate using PKE approximation!");

          std::vector<dof_id_type> & dof = to_vars->dofIndices();
          to_vars->sys().solution().set(dof[0], keff[0]);
          to_vars->sys().solution().close();
        }

      break;
    }
  }

  _console << "Finished MultiAppMooseOkapiReactivityTransfer transfer " << name() << std::endl;
}
