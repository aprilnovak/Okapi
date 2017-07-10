#include "OkapiMCSApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

// auxkernels
#include "KappaFissionToHeatSource.h"

// test kernels (not used)
#include "MomFrictionForce.h"

// executioner and timestepper
#include "OpenMCExecutioner.h"
#include "OpenMCTimeStepper.h"

// functions for passing data
#include "LegendrePolynomial.h"
#include "ZernikePolynomial.h"
#include "ZernikeLegendreReconstruction.h"

// transfers
#include "MultiAppOkapiMooseTransfer.h"

// user objects
#include "ZernikeLegendreDeconstruction.h"
#include "ZLDeconstruction.h"

template <>
InputParameters
validParams<OkapiMCSApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

OkapiMCSApp::OkapiMCSApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  OkapiMCSApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  OkapiMCSApp::associateSyntax(_syntax, _action_factory);
}

OkapiMCSApp::~OkapiMCSApp() {}

// External entry point for dynamic application loading
extern "C" void
OkapiMCSApp__registerApps()
{
  OkapiMCSApp::registerApps();
}
void
OkapiMCSApp::registerApps()
{
  registerApp(OkapiMCSApp);
}

// External entry point for dynamic object registration
extern "C" void
OkapiMCSApp__registerObjects(Factory & factory)
{
  OkapiMCSApp::registerObjects(factory);
}
void
OkapiMCSApp::registerObjects(Factory & factory)
{
  registerAuxKernel(KappaFissionToHeatSource);
  registerKernel(MomFrictionForce);
  registerExecutioner(OpenMCExecutioner);
  registerTimeStepper(OpenMCTimeStepper);
  registerFunction(LegendrePolynomial);
  registerFunction(ZernikePolynomial);
  registerFunction(ZernikeLegendreReconstruction);
  registerTransfer(MultiAppOkapiMooseTransfer);
  registerUserObject(ZernikeLegendreDeconstruction);
  registerUserObject(ZLDeconstruction);
}

// External entry point for dynamic syntax association
extern "C" void
OkapiMCSApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  OkapiMCSApp::associateSyntax(syntax, action_factory);
}
void
OkapiMCSApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
