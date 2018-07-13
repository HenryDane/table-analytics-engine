#include <vector>
#include <algorithm>
#include "main.h"
#include "theilsen.h"
#include "mkt.h"

// from https://pubs.usgs.gov/tm/2006/tm4a7/pdf/USGSTM4A7.pdf
void theil_sen(ts_result_t &result, std::vector<row_t> &table, int units_as, double threshold) {
	std::vector<double> slopes;
	double slope = NAN;

	// calculate slopes
	for (int i = 0; i < table.size() - 1; i++) {
		for (int j = 1; j < table.size(); j++) {
#if F_SKIP_SAME_COMPARE == 1
			if (i == j) continue;
#endif
			if (units_as == 1) {
				// units/year
				slopes.push_back((table.at(j).value - table.at(i).value) / (date_as_year(table.at(j).date) - date_as_year(table.at(i).date)));
			} else if (units_as == 2) {
				// units/month
				slopes.push_back((table.at(j).value - table.at(i).value) / (date_as_month(table.at(j).date) - date_as_month(table.at(i).date)));
			} else {
				// units/day
				slopes.push_back((table.at(j).value - table.at(i).value) / (date_as_day(table.at(j).date) - date_as_day(table.at(i).date)));
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

double date_as_month(date_t &date) {
	return (date_as_day(date)) / DAYS_PER_TYP_MONTH;
}

double date_as_year(date_t &date) {
	return (date_as_day(date)) / 365;
}

// returns days since 00/00/0000 as integer with leap years
int date_as_day(date_t &date) {
	int day_sum = 0;
	for (int i = 1; i < date.month + 1; i++) {
		int days_month = 0;
		if (i == 9 || i == 4 || i == 6 || i == 11) {
			// sept/aprl/june/novm
			days_month = 30;
		}
		else if (i == 2) {
			if ((date.year - 1600) % 4 == 0) {
				// leap year
				days_month = 29;
			}
			else {
				days_month = 28;
			}
		}
		else {
			days_month = 31;
		}

		day_sum += days_month;
	}

	day_sum += date.year * 365;
	day_sum += date.day;
	return day_sum;
}