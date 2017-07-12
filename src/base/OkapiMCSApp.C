#include "OkapiMCSApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

// ics
#include "OpenMCTempScalarComponentIC.h"

// test kernels (not used)
#include "MomFrictionForce.h"

// executioner and timestepper
#include "OpenMCExecutioner.h"
#include "OpenMCTimeStepper.h"

// transfers
#include "MultiAppOkapiMooseTransfer.h"
#include "MultiAppMooseOkapiTransfer.h"
#include "MultiAppMoonOkapiTransfer.h"

// user objects
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
  registerInitialCondition(OpenMCTempScalarComponentIC);
  registerKernel(MomFrictionForce);
  registerExecutioner(OpenMCExecutioner);
  registerTimeStepper(OpenMCTimeStepper);
  registerTransfer(MultiAppOkapiMooseTransfer);
  registerTransfer(MultiAppMooseOkapiTransfer);
  registerTransfer(MultiAppMoonOkapiTransfer);
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
