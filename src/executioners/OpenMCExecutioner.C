#include "OpenMCExecutioner.h"
#include "OpenMCInterface.h"

template <>
InputParameters
validParams<OpenMCExecutioner>()
{
  InputParameters params = validParams<Transient>();
  return params;
}

OpenMCExecutioner::OpenMCExecutioner(const InputParameters & parameters) : Transient(parameters) {}

/* This method is called only one time at the start of the entire simulation.
Hence, this method calls openmc_init, which reads data from input files, sets
up MPI and HDF5 compound data types, allocates memory, etc. */
void
OpenMCExecutioner::init()
{
  Transient::init();
  OpenMC::openmc_init(_communicator.get());
  //  OpenMC::initialize_coupling();
}
