#include <string>

// all error codes from OpenMC that are to be handled
extern int e_unassigned;
extern int e_allocate;
extern int e_out_of_bounds;
extern int e_invalid_size;
extern int e_invalid_argument;
extern int e_invalid_type;
extern int e_invalid_id;
extern int e_geometry;
extern int e_data;
extern int e_physics;

// all warning codes from OpenMC that are to be handled
extern int e_warning;

extern char openmc_err_msg [256];

namespace ErrorHandling{

  void receive_coeffs_for_cell(int);
  void get_coeffs_from_cell(int);
  void openmc_cell_set_temperature(int);
  void openmc_get_cell_index(int, const std::string &);
  void openmc_get_material_index(int, const std::string &);
  void openmc_material_set_density(int);
}
