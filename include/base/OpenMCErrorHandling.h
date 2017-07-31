#include <string>

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

namespace ErrorHandling{

  void receive_coeffs_for_cell(int);
  void get_coeffs_from_cell(int);
  void openmc_get_cell(int, const std::string &);
}
