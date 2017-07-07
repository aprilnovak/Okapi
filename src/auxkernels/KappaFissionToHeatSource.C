#include "KappaFissionToHeatSource.h"

template<>
InputParameters validParams<KappaFissionToHeatSource>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("kappa_fission_source", "Continuous field\
  representing the kappa-fisison source (eV/source particle)");
  params.addRequiredParam<Real>("power", "pin power (W)");
  params.addRequiredParam<std::string>("kappa_fission_pp", "The name\
    of the postprocessor that integrates the kappa-fission distribution.");
  params.addRequiredParam<std::string>("volume_pp", "The name of the\
  postprocessor that calculates volume.");
  return params;
}

KappaFissionToHeatSource::KappaFissionToHeatSource(const InputParameters & parameters) :
    AuxKernel(parameters),
    _kappa_fission(coupledValue("kappa_fission_source")),
    _power(parameters.get<Real>("power")),
    _kappa_fission_pp(getPostprocessorValueByName(parameters.get<std::string>("kappa_fission_pp"))),
    _volume_pp(getPostprocessorValueByName(parameters.get<std::string>("volume_pp")))
{
}

KappaFissionToHeatSource::~KappaFissionToHeatSource()
{
}

Real
KappaFissionToHeatSource::computeValue()
{
  /* This converts a kappa fission tally (units eV/source particle)
     to a volumetric heat source (W/cm^3) to be used in coupled
     simulations. TODO: make this more general by:
       - accounting for energy produced in the coolant (note this only
         accounts for energy produced in the fuel)
       - account for a non-constant user power */

  if (_kappa_fission_pp < 0.000001)
    return 0.0;
  else
    return _kappa_fission[_qp] * _power / (_kappa_fission_pp * _volume_pp);
}
