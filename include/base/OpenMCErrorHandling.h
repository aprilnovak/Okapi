#include <string>

namespace ErrorHandling
{

void get_coeffs_from_cell(int);
void openmc_cell_set_temperature(int);
void openmc_get_cell_index(int, const std::string &);
void openmc_get_tally_index(int, const std::string &);
void openmc_get_material_index(int, const std::string &);
void openmc_material_set_density(int);
void openmc_get_keff(int);
void openmc_tally_results(int, const std::string &);
void openmc_tally_get_filters(int, const std::string &);
void openmc_cell_get_id(int, const std::string &);
void openmc_filter_get_type(int, const std::string &);
void openmc_cell_filter_get_bins(int, const std::string &);
void openmc_filter_get_order(int, const std::string &);
}
