#ifndef OKAPIAPP_H
#define OKAPIAPP_H

#include "MooseApp.h"

class OkapiApp;

template <>
InputParameters validParams<OkapiApp>();

class OkapiApp : public MooseApp
{
public:
  OkapiApp(InputParameters parameters);
  virtual ~OkapiApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* OKAPIAPP_H */
