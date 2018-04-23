#include <string>

namespace ErrorHandling
{

void get_coeffs_from_cell(int);
void openmc_cell_set_temperature(int);
void openmc_get_cell_index(int, const std::string &);
void openmc_get_material_index(int, const std::string &);
void openmc_material_set_density(int);
void openmc_get_keff(int);
}
