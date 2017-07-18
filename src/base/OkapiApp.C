#include "OkapiApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

// executioner and timestepper
#include "OpenMCExecutioner.h"
#include "OpenMCTimeStepper.h"

// transfers
#include "MultiAppOkapiMooseTransfer.h"
#include "MultiAppMooseOkapiTransfer.h"
#include "MultiAppMoonOkapiTransfer.h"

template <>
InputParameters
validParams<OkapiApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

OkapiApp::OkapiApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  OkapiApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  OkapiApp::associateSyntax(_syntax, _action_factory);
}

OkapiApp::~OkapiApp() {}

// External entry point for dynamic application loading
extern "C" void
OkapiApp__registerApps()
{
  OkapiApp::registerApps();
}
void
OkapiApp::registerApps()
{
  registerApp(OkapiApp);
}

// External entry point for dynamic object registration
extern "C" void
OkapiApp__registerObjects(Factory & factory)
{
  OkapiApp::registerObjects(factory);
}
void
OkapiApp::registerObjects(Factory & factory)
{
  registerExecutioner(OpenMCExecutioner);
  registerTimeStepper(OpenMCTimeStepper);
  registerTransfer(MultiAppOkapiMooseTransfer);
  registerTransfer(MultiAppMooseOkapiTransfer);
  registerTransfer(MultiAppMoonOkapiTransfer);
}

// External entry point for dynamic syntax association
extern "C" void
OkapiApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  OkapiApp::associateSyntax(syntax, action_factory);
}
void
OkapiApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
