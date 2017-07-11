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

#ifndef OPENMCTEMPSCALARCOMPONENTIC_H
#define OPENMCTEMPSCALARCOMPONENTIC_H

#include "ScalarInitialCondition.h"

class OpenMCTempScalarComponentIC;

template <>
InputParameters validParams<OpenMCTempScalarComponentIC>();

class OpenMCTempScalarComponentIC : public ScalarInitialCondition
{
public:
  OpenMCTempScalarComponentIC(const InputParameters & parameters);

protected:
  virtual Real value();

  std::vector<Real> _initial_values;
  int _cell;
  bool _dbg;
};

#endif /* OPENMCTEMPSCALARCOMPONENTIC_H */
