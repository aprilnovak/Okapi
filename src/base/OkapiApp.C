#include "OkapiApp.h"
#ifdef ENABLE_BUFFALO_COUPLING
#include "BuffaloApp.h"
#endif
#ifdef ENABLE_MOON_NEK_COUPLING
#include "MoonApp.h"
#endif
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
#include "MultiAppMooseOkapiReactivityTransfer.h"

#ifdef ENABLE_NEK_COUPLING
#include "MultiAppMoonOkapiTransfer.h"
#endif // ENABLE_NEK_COUPLING

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
#ifdef ENABLE_BUFFALO_COUPLING
  BuffaloApp::registerObjects(_factory);
#endif
#ifdef ENABLE_NEK_COUPLING
  MoonApp::registerObjects(_factory);
#endif
  OkapiApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
#ifdef ENABLE_BUFFALO_COUPLING
  BuffaloApp::associateSyntax(_syntax, _action_factory);
#endif
#ifdef ENABLE_NEK_COUPLING
  MoonApp::associateSyntax(_syntax, _action_factory);
#endif
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
#ifdef ENABLE_BUFFALO_COUPLING
  registerApp(BuffaloApp);
#endif
#ifdef ENABLE_NEK_COUPLING
  registerApp(MoonApp);
#endif
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
  registerTransfer(MultiAppMooseOkapiReactivityTransfer);
#ifdef ENABLE_NEK_COUPLING
  registerTransfer(MultiAppMoonOkapiTransfer);
#endif
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
