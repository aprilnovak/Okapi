#include <string>

// all error codes from OpenMC that are to be handled
extern int e_unassigned;
extern int e_out_of_bounds;
extern int e_cell_not_allocated;
extern int e_cell_invalid_id;
extern int e_cell_not_found;
extern int e_nuclide_not_allocated;
extern int e_nuclide_not_loaded;
extern int e_nuclide_not_in_library;
extern int e_material_not_allocated;
extern int e_material_invalid_id;
extern int e_tally_not_allocated;
extern int e_tally_invalid_id;
extern int e_invalid_size;
extern int e_cell_no_material;

// all warning codes from OpenMC that are to be handled
extern int w_below_min_bound;
extern int w_above_max_bound;

namespace ErrorHandling{

  void receive_coeffs_for_cell(int);
  void get_coeffs_from_cell(int);
  void openmc_cell_set_temperature(int);
  void openmc_get_cell(int, const std::string &);
  void openmc_get_material(int, const std::string &);
  void openmc_material_set_density(int);
}
