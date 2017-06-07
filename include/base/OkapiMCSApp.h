#ifndef OKAPIMCSAPP_H
#define OKAPIMCSAPP_H

#include "MooseApp.h"

class OkapiMCSApp;

template <>
InputParameters validParams<OkapiMCSApp>();

class OkapiMCSApp : public MooseApp
{
public:
  OkapiMCSApp(InputParameters parameters);
  virtual ~OkapiMCSApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* OKAPIMCSAPP_H */
