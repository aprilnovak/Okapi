#include "MultiAppMooseOkapiTransfer.h"
#include "OpenMCErrorHandling.h"
#include "openmc.h"
#include "math.h"

#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "SubProblem.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

#include "xtensor/xarray.hpp"

template <>
InputParameters
validParams<MultiAppMooseOkapiTransfer>()
{
  /* This is used to transfer data between Okapi and a generic MOOSE
     application when the MOOSE application is the sub App and Okapi
     the Master App. */
  InputParameters params = validParams<MultiAppFXTransfer>();
  params.suppressParameter<std::string>("this_app_object_name");
  params.set<std::string>("this_app_object_name") = "";
  params.addRequiredParam<int32_t>("openmc_cell",
                                   "OpenMC cell ID (defined in "
                                   "XML input file) for this transfer to be associated with.");
  params.addRequiredParam<int32_t>(
      "openmc_tally", "The functional expansion tally id that covers the desired cell geometry.");
  params.addParam<bool>("dbg", false, "Whether to turn on debugging information");
  params.addParam<bool>("store_results",
                        true,
                        "Whether to store the iteration "
                        "results for every Picard iteration.");
  MooseEnum geometry_type("cartesian cylindrical");
  params.addRequiredParam<MooseEnum>(
      "geometry_type", geometry_type, "The type of geometry. Either cylindrical or cartesian.");
  return params;
}

MultiAppMooseOkapiTransfer::MultiAppMooseOkapiTransfer(const InputParameters & parameters)
  : MultiAppFXTransfer(parameters),
    _cell(parameters.get<int32_t>("openmc_cell")),
    _tally(parameters.get<int32_t>("openmc_tally")),
    _dbg(parameters.get<bool>("dbg")),
    _store_results(parameters.get<bool>("store_results")),
    _geometry_multiplier(getParam<MooseEnum>("geometry_type") == "cylindrical" ? 2. : 1.),
    _checks_done(false),
    _stride_integer(0)
{
}

void
MultiAppMooseOkapiTransfer::initialSetup()
{
  // Search for the _multi_app_object_name in each of the MultiApps
  for (std::size_t i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
    {
      if (i == 0) // First time through, assign without checking against previous values
        getSubAppObject = scanProblemBaseForObject(
            _multi_app->appProblemBase(i), _multi_app_object_name, _multi_app->name());
      else if (getSubAppObject != scanProblemBaseForObject(_multi_app->appProblemBase(i),
                                                           _multi_app_object_name,
                                                           _multi_app->name()))
        mooseError("The name '",
                   _multi_app_object_name,
                   "' is assigned to two different object types. Please modify your input file and "
                   "try again.");
    }
  if (getSubAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _multi_app_object_name, "' in SubApp");
}

void
MultiAppMooseOkapiTransfer::execute()
{
  _console << "Beginning MultiAppMooseOkapiTransfer Transfer " << name() << std::endl;

  unsigned int num_apps = _multi_app->numGlobalApps();

  // get the index of the cell in the cells(:) OpenMC array to be used
  // for later calls to cell-dependent OpenMC routines. This cannot be
  // put in the constructor because we cannot guarantee that the openmc_init
  // subroutine will be called before this object's constructor.
  int err_index = openmc_get_cell_index(_cell, &_cell_index);
  ErrorHandling::openmc_get_cell_index(err_index, "MultiAppMooseOkapiTransfer");
  err_index = openmc_get_tally_index(_tally, &_tally_index);
  ErrorHandling::openmc_get_tally_index(err_index, "MultiAppMooseOkapiTransfer");

  switch (_direction)
  {
    // MOOSE -> Okap
    case FROM_MULTIAPP:
    {
      for (unsigned int I = 0; I < num_apps; ++I)
      {
        if (_multi_app->hasLocalApp(I) && _multi_app->isRootProcessor())
        {
          if (_store_results)
          {
            for (const auto& c : cells_) 
            {
              auto shell_id = cell_to_shell.at(c.material_index_);
              auto t = _shell_temperatures.at(shell_id)
              if (_dbg)
                _console << "Setting OpenMC cell " << c << " temperature to " << t << std::endl;
              int err_temp = c.set_temperature(t);
              ErrorHandling::openmc_cell_set_temperature(err_temp);
            }
          }
        }
      }
      break;
    }

    // Okapi -> MOOSE
    case TO_MULTIAPP:
    {
      for (unsigned int I = 0; I < num_apps; ++I)
      {
        if (_multi_app->hasLocalApp(I))
        {

          // Create array to store volumetric heat deposition in each material
          xt::xtensor<double, 1> heat = xt::empty<double>({n_materials_});

          // Get heat source normalized by user-specified power
          heat = openmc_driver_->heat_source(power_);

          // TODO:  Give this to Bison?

        }
        else
        {
          // TODO: ???
        }
      }
      break;
    }
  }
  _console << "Finished MultiAppMooseOkapiTransfer transfer " << name() << std::endl;
}

void
MultiAppMooseOkapiTransfer::printResults(std::vector<Real> & results)
{
  for (unsigned int i = 0; i < results.size(); ++i)
    _console << results[i] << ", ";

  _console << std::endl;
}

bool
MultiAppMooseOkapiTransfer::getOrderAndCheckExpansionType(const char * type,
                                                          const int32_t & index,
                                                          int32_t & order)
{
  int err_get;
  if (!(std::string("legendre").compare(type)))
    err_get = openmc_legendre_filter_get_order(index, &order);
  else if (!(std::string("sphericalharmonics").compare(type)))
    err_get = openmc_sphharm_filter_get_order(index, &order);
  else if (!(std::string("spatiallegendre").compare(type)))
    err_get = openmc_spatial_legendre_filter_get_order(index, &order);
  else if (!(std::string("zernike").compare(type)))
    err_get = openmc_zernike_filter_get_order(index, &order);
  else
    mooseError("No expansion filter specified. Check the tallies XML input file.");

  ErrorHandling::openmc_filter_get_order(err_get, "MultiAppMooseOkapiTransfer");
  return 1;
}

void
MultiAppMooseOkapiTransfer::runChecks()
{
  int32_t * filter_indices = nullptr;
  int32_t num_filter_indices;
  int err_get = openmc_tally_get_filters(_tally_index, &filter_indices, &num_filter_indices);
  ErrorHandling::openmc_tally_get_filters(err_get, "MultiAppMooseOkapiTransfer");

  if (num_filter_indices != 2)
    mooseError("We expect there to be exactly two filters, one a cell filter and the other an "
               "expansion filter. Check the tallies XML file to verify the existence of both for "
               "the requested tally");
  {
    int32_t id;
    err_get = openmc_cell_get_id(i + 1, &id);
    ErrorHandling::openmc_cell_get_id(err_get, "MultiAppMooseOkapiTransfer");
    index_to_id[i + 1] = id;
  }

  // Important variables
  int32_t order;
  char type[20];
  int32_t * cell_indices = nullptr;
  bool cell_id_found = false;

  std::string cell_filter_name = "cell";
  bool cell_filter_found = false;
  for (int32_t i = 0; i <= 1; ++i)
  {
    err_get = openmc_filter_get_type(filter_indices[i], type);
    ErrorHandling::openmc_filter_get_type(err_get, "MultiAppMooseOkapiTransfer");
    if (cell_filter_name.compare(type))
      continue;
    else
    {
      cell_filter_found = true;
      _cell_filter_index = i;
      break;
    }
  }
  if (!cell_filter_found)
    mooseError("No cell filter specified. Check the tallies XML input file.");
  else
  {
    err_get = openmc_cell_filter_get_bins(
        filter_indices[_cell_filter_index], &cell_indices, &_num_cells_in_filter);
    ErrorHandling::openmc_cell_filter_get_bins(err_get, "MultiAppMooseOkapiTransfer");

    for (decltype(_num_cells_in_filter) i = 0; i < _num_cells_in_filter; ++i)
    {
      if (cell_indices[i] == _cell_index)
      {
        cell_id_found = true;
        _stride_integer = i;
        break;
      }
    }
    if (!cell_id_found)
      mooseError("Requested cell_id not in the passed tally. Check that the cell filter in "
                 "the tallies XML file contains the ID you're requesting");
  }
  int32_t expansion_filter_index = 1 - _cell_filter_index;
  err_get = openmc_filter_get_type(filter_indices[expansion_filter_index], type);
  ErrorHandling::openmc_filter_get_type(err_get, "MultiAppMooseOkapiTransfer");
  getOrderAndCheckExpansionType(type, filter_indices[expansion_filter_index], order);

  _checks_done = true;
}

//! Get tally results array
//!
//! \return Internal tally results array. First dimension is filter
//!   combinations, second dimension is scores, third dimension is of size
//!   three (temporary value, sum of realizations, sum-squared)
xt::xtensor<double, 3> 
MultiAppMooseOkapiTransfer::tally_results()
{
  // Get material bins
  int32_t* mats;
  int32_t n_mats;
  int err_temp = openmc_material_filter_get_bins(index_filter_, &mats, &n_mats);
  // TODO: ErrorHandling

  // Get tally results and number of realizations
  double* results;
  std::array<std::size_t, 3> shape;
  err_chk(openmc_tally_results(index_tally_, &results, shape.data()));
  int32_t m;
  err_temp = openmc_tally_get_n_realizations(index_tally_, &m);
  // TODO: ErrorHandling

  // Determine size
  std::size_t size {shape[0] * shape[1] * shape[2]};

  // Adapt array into xtensor with no ownership
  return xt::adapt(results, size, xt::no_ownership(), shape);
}

//! Get energy deposition in each material
//!
//! \param power User-specified power in [W]
//! \return Heat source in each material as [W/cm3]
xt::xtensor<double, 1> 
MultiAppMooseOkapiTransfer::heat_source(double power)
{
  // Get tally results
  auto results {tally_results()};

  // Determine number of realizatoins for normalizing tallies
  int32_t m;
  int err_temp = openmc_tally_get_n_realizations(index_tally_, &m);
  // TODO: ErrorHandling

  // Determine energy production in each material
  auto mean_value = xt::view(results, xt::all(), 0, 1);
  xt::xtensor<double, 1> heat = JOULE_PER_EV * mean_value / m;

  // Get total heat production [J/source]
  double total_heat = xt::sum(heat)();

  // Normalize heat source in each material and collect in an array
  for (int i = 0; i < cells_.size(); ++i) {
    // Get volume
    double V = cells_.at(i).volume_;

    // Convert heat from [J/source] to [W/cm^3]. Dividing by total_heat gives
    // the fraction of heat deposited in each material. Multiplying by power
    // givens an absolute value in W
    heat(i) *= power / (total_heat * V);
  }

  return heat;
}

