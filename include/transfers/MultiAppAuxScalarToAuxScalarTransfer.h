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

#ifndef MULTIAPPAUXSCALARTOAUXSCALARTRANSFER_H
#define MULTIAPPAUXSCALARTOAUXSCALARTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declerations
class MultiAppAuxScalarToAuxScalarTransfer;

template <>
InputParameters validParams<MultiAppAuxScalarToAuxScalarTransfer>();

/**
 * Copies the value of a Postprocessor from one app to a scalar AuxVariable in another.
 */
class MultiAppAuxScalarToAuxScalarTransfer : public MultiAppTransfer
{
public:
  MultiAppAuxScalarToAuxScalarTransfer(const InputParameters & parameters);

  /**
   * Execute the transfer
   */
  virtual void execute() override;

protected:
  /// The name of the variable from which the scalar values are being transfered
  VariableName _from_aux_name;

  /// The name of the variable to which the scalar values are being transfered
  VariableName _to_aux_name;
};

#endif /* MULTIAPPAUXSCALARTOAUXSCALARTRANSFER_H */
