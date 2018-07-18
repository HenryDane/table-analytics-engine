#include <vector>
#include <algorithm>
#include "main.h"
#include "theilsen.h"
#include "util.h"
#include "scimath.h"

// from https://pubs.usgs.gov/tm/2006/tm4a7/pdf/USGSTM4A7.pdf
void theil_sen(ts_result_t &result, std::vector<row_t> &table, int units_as, double threshold) {
	std::vector<double> slopes;
	double slope = NAN;

	// calculate slopes
	for (unsigned int i = 0; i < table.size() - 1; i++) {
		for (unsigned int j = 1; j < table.size(); j++) {
			if (i == j) continue;
			if (units_as == 1) {
				// units/year
				slopes.push_back((table.at(j).value.v - table.at(i).value.v) / (date_as_year(table.at(j).date) - date_as_year(table.at(i).date)));
			} else if (units_as == 2) {
				// units/month
				slopes.push_back((table.at(j).value.v - table.at(i).value.v) / (date_as_month(table.at(j).date) - date_as_month(table.at(i).date)));
			} else {
				// units/day
				slopes.push_back((table.at(j).value.v - table.at(i).value.v) / (date_as_day(table.at(j).date) - date_as_day(table.at(i).date)));
			}
		}
	}

	// sort slopes
	std::sort(slopes.begin(), slopes.end());

	// pick median or average two middle values
	if (slopes.size() % 2) {
		// odd size
		slope = slopes.at(slopes.size() / 2);
	} else {
		/// even size
		slope = (slopes.at(slopes.size() / 2) + slopes.at((slopes.size() / 2) + 1) / 2);
	}

	// confidence boundaries
	int n_p = ((table.size() - 1) * table.size()) / 2;
	double z = norm_cdf(1.96); // is this wrong?
	double r_u = (n_p + (z * sqrt((table.size() * (table.size() - 1) * (2 * table.size() + 5)) / 18))) / 2 + 1; // eqn 4 (this might be wrong)
	double r_l = (n_p - (z * sqrt((table.size() * (table.size() - 1) * (2 * table.size() + 5)) / 18))) / 2 + 1; // eqn 5 (this might be wrong)

	result.slope = slope;
	result.r_u = r_u;
	result.r_l = r_l;
}