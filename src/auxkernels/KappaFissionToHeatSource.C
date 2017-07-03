#include "KappaFissionToHeatSource.h"

template<>
InputParameters validParams<KappaFissionToHeatSource>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<Real>("power", "pin power (W)");
  params.addRequiredParam<std::string>("volume_pp", "The name of the\
    postprocessor that calculates volume.");
  return params;
}

KappaFissionToHeatSource::KappaFissionToHeatSource(const InputParameters & parameters) :
    AuxKernel(parameters),
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
     simulations. The power is divided by the eV/source particle
     coupled value in order to determine how many source particles
     are created per second. This is then used to determine by what
     factor the eV/source particle tally results need to be multiplied
     to obtain W/cm^3. */
  double particles_per_sec = _power / _u[_qp];

  long double J_per_eV = 1.602176565e-19;

  return J_per_eV * _u[_qp] * particles_per_sec / _volume_pp;
}
