#include "KappaFissionToHeatSource.h"

template<>
InputParameters validParams<KappaFissionToHeatSource>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("kappa_fission_source", "Continuous field\
  representing the kappa-fisison source (eV/source particle)");
  params.addRequiredParam<Real>("power", "pin power (W)");
  params.addRequiredParam<std::string>("volume_pp", "The name of the\
  postprocessor that calculates volume.");
  return params;
}

KappaFissionToHeatSource::KappaFissionToHeatSource(const InputParameters & parameters) :
    AuxKernel(parameters),
    _kappa_fission(coupledValue("kappa_fission_source")),
    _power(parameters.get<Real>("power")),
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
     simulations assuming a constant number of neutrons produced per
     second. TODO: make this more general by:
       - accounting for energy produced in the coolant (note this only
         accounts for energy produced in the fuel)
       - account for a non-constant neutrons/sec produced */
  Real particles_per_sec;
  Real eV_per_J = 6.241509e18;
  Real eV_per_fission = 200.0e6;
  Real source_neutrons_per_fission = 2.45;
  Real fission_power;

  particles_per_sec = _power * source_neutrons_per_fission * eV_per_J /
    eV_per_fission;
  fission_power = _kappa_fission[_qp] * particles_per_sec /
    (_volume_pp * eV_per_J);
  return fission_power;
}
