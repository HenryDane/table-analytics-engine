#include <vector>
#include <fstream>
#include <algorithm>
#include <functional>
#include "main.h"
#include "scimath.h"
#include "util.h"

bool filter_bounds(std::vector<row_t> &table, double lbound, double ubound) {
	for (unsigned int i = 0; i < table.size(); i++) {
		if (table[i].value.v < lbound || table[i].value.v > ubound) {
			table[i].value.v = NAN;
			table[i].value.f = true;
		}
	}

	return true;
}

bool filter_norepeats(std::vector<row_t> &table, unsigned int threshold) {
	std::vector<double> a;

	// file list a with first threshold values of table
	if (threshold >= table.size()) {
		return true;
	}
	else {
		// haha lal its actually zeroes
		a.resize(threshold, 0);
	}

	// for each element in data
	for (unsigned int i = 0; i < table.size(); i++) {
		// drop first element of vector
		a.erase(a.begin());

		// add current element ot right of a
		a.push_back(table[i].value.v);

		// if all elements in a are equal
		if (std::adjacent_find(a.begin(), a.end(), std::not_equal_to<double>()) == a.end()) {

			// find address of first element
			int first_index = i - threshold;
			if (first_index < 0) {
				printf("its all hyucked\n");
				return false;
			}

			// look for end of repeat section
			while (table[i].value.v == a[0]) {
				i++;
			}

			// overwrite values with NAN
			for (unsigned int j = first_index; j < i; j++) {
				table[i].value.v = NAN;
				table[i].value.f = true;
			}

			a[threshold - 1] = table[i].value.v;
		}
	}

	return true;
}

bool filter_med_outliers(std::vector<row_t> &table,
		bool secfilt,
		double level,
		double scale,
		int filter_len,
		double lquantile,
		double uquantile,
		double seclevel,
		double secscale,
		int secfilt_len,
		double seclquantile,
		double secuquantile ) {
	std::vector<row_t> filt;
	std::vector<row_t> res;

	if (filter_len % 2 == 0 ||
		secfilt_len % 2 == 0) {
		printf("filter lengths must be odd \n");
		return false;
	}

	if (secfilt) {
		filt = scipy_median_filter(table, secfilt_len);
		res.resize(filt.size());

		for (unsigned int i = 0; i < table.size(); i++) {
			res[i] = table[i];
			res[i].value.v = table[i].value.v - filt[i].value.v;
		}

		double iqr = NAN;

		for (unsigned int k = 0; k < table.size(); k++) {
			printf("%d / %d \r", k + 1, table.size());
			if (std::isnan(secscale)) {
				int slicelow = std::max((unsigned int) 0, k - ((secfilt_len - 1) / 2));
				int slicehigh = std::min(table.size(), k + ((secfilt_len - 1) / 2) + 1);

				double * rwindow = new double[slicehigh - slicelow];

				for (int m = slicelow; m < slicehigh; m++) {
					rwindow[m - slicelow] = table[m].value.v;

					quantile_t q = scimath_quantile(rwindow, slicehigh - slicelow, seclquantile, secuquantile);

					iqr = q.h - q.l;
				}
			}
			else {
				iqr = secscale;
			}

			if (res[k].value.v > seclevel * iqr || res[k].value .v < - seclevel * iqr) {
				table[k].value.v = NAN;
				table[k].value.f = true;
			}
		}

		printf("\n");
	}

	filt = scipy_median_filter(table, filter_len);
	res.resize(filt.size());

	for (unsigned int i = 0; i < table.size(); i++) {
		res[i] = table[i];
		res[i].value.v = table[i].value.v - filt[i].value.v;
	}

	double iqr = NAN;

	for (unsigned int k = 0; k < table.size(); k++) {
		printf("%d / %d \r", k + 1, table.size());
		if (std::isnan(scale)) {
			int slicelow = std::max((unsigned int) 0, k - ((filter_len - 1) / 2));
			int slicehigh = std::min(table.size(), k + ((filter_len - 1) / 2) + 1);

			double * rwindow = new double[slicehigh - slicelow];

			for (int m = slicelow; m < slicehigh; m++) {
				rwindow[m - slicelow] = table[m].value.v;

				quantile_t q = scimath_quantile(rwindow, slicehigh - slicelow, lquantile, uquantile);

				iqr = q.h - q.l;
			}
		}
		else {
			iqr = scale;
		}

		if (res[k].value.v > level * iqr || res[k].value.v < -level * iqr) {
			table[k].value.v = NAN;
			table[k].value.f = true;
		}
	}

	return true;
}

bool filter_delta_limit(std::vector<row_t> &table, double delta_limit) {
	reflag_table(table);

	for (unsigned int i = 0; i < table.size(); i++) {
		table[i].edits = 0;
	}

	for (unsigned int i = 0; i < table.size() - 1; i++) {
		if (table[i].value.f || table[i + 1].value.f) continue;
		double d = (table[i + 1].value.v - table[i].value.v) / (table[i + 1].date.numeric() - table[i].date.numeric());
		//double d = table[i + 1].value.v - table[i].value.v;
		if (abs(d) > delta_limit) {
			printf("Removing (%f, %f) with DELTA=%f before point (%f, %f)\n",
				table[i].date.numeric(), table[i].value.v,
				d,
				table[i + 1].date.numeric(), table[i + 1].value.v);
			// TODO decide which (or both to flag by delta_next and delta_prev

			table[i].edits = 10;
			table[i + 1].edits = 10;

		}
	}

	for (unsigned int i = 0; i < table.size(); i++) {
		if (table[i].edits > 0) {
			table[i].value.v = NAN;
			table[i].value.f = true;
		}
	}

	return true;
}

bool clean_data(std::vector<row_t> &table) {
	return true;
}

void resolve_table_state(std::vector<row_t> &table){
	if (table[0].state == 65 ||
		table[0].state == 17) return;

	for (unsigned int i = 0; i < table.size(); i++) {
		printf("\t\t\t\t%d / %d\r", i + 1, table.size());
		for (unsigned int j = i; j < table.size(); j++) {
			if (i == j) {
				continue;
			}

			if (table[i].id == -1000 ||
				table[j].id == -1000) {
				continue;
			}

			if (table[i].variable == table[j].variable) {
				if (is_equal(table[i].date, table[j].date)) {
					if (table[i].state == 0) {
						//printf("erased: %d %d %s %s\n", table.at(i).state, table.at(j).state, table.at(i).variable.c_str(), table.at(j).variable.c_str());
						// we prefer CDEC
						//table.erase(table.begin() + j);
						table[j].id = -1000;
						//i = std::max(0, std::min(i - 1, j - 1));
						//j = 0;
					} else if (table[j].state == 0) {
						table[i].id = -1000;
					}
					else {
						table[i].id = -1000;
					}
				}
			}
		}
	}


}

int old_table_agg(std::vector<row_t> &table) {
	std::vector<row_t> table_agg;
	for (unsigned int i = 0; i < table.size(); i++) {
		row_t r = table.at(i);

		// is it already in table_agg
		bool found = false;
		unsigned int j = 0;
		for (; j < table_agg.size(); j++) {
			if (table_agg.at(j).date.year == r.date.year && r.variable == table_agg.at(j).variable) {
				found = true;
				break;
			}
		}

		if (found) {
			table_agg.at(j).value.v += r.value.v;
			table_agg.at(j).edits++;
			table.erase(table.begin() + i);
			i = 0;
		}
		else {
			table_agg.push_back(r);
			table.erase(table.begin() + i);
			i = 0;
		}
	}

	std::ofstream outputa;
	outputa.open("results\\outputa.txt");
	if (!outputa.is_open()) {
		printf("failed to load output \n");
		return -2;
	}
	for (unsigned int i = 0; i < table_agg.size(); i++) {
		outputa << table_agg.at(i).id << " ";
		outputa << table_agg.at(i).date.day << " " << table_agg.at(i).date.month << " " << table_agg.at(i).date.year << " ";
		outputa << table_agg.at(i).variable << " ";
		outputa << table_agg.at(i).value.v << std::endl;
	}
	outputa.close();

	table = table_agg;

	return true;
}

void period_filter(std::vector<row_t> &table, period_t period) {
	//period_t period = { "All", YEARLY,{ 0, 0, 0, 0, 0 },{ 9999, 99, 99, 99, 99 } };
	//period_t period = { "Pre_WP", YEARLY, { 0, 0, 0 }, { 1967, 9, 31 } };
	//period_t period = { "Post_WP", YEARLY, { 1967, 10, 0 }, { 2017, 9, 31 } };
	//period_t period = { "Pre_SMSCG", YEARLY, { 0, 0, 0 }, { 1987, 9, 31 } };
	//period_t period = { "Post_SMSCG", YEARLY, { 1987, 10, 1 }, { 9999, 99, 99 } };
	for (unsigned int i = 0; i < table.size(); i++) {
		if (!date_in_period(table.at(i).date, period)) {
			table.erase(table.begin() + i);
			i = std::max((unsigned int) 0, i - 2);
		}
	}
}

// removes gaps from individual var's ts.
bool degap_table_monthly(std::vector<row_t> &table) {
//hyuckin_fix_loop:
	std::sort(table.begin(), table.end(),date_sort);

	date_t start = table.at(0).date;
	date_t end = table.at(table.size() - 1).date;

	unsigned int expected_size = ((end.year - start.year) * 12) + (end.month - start.month) /*i think*/;

	if (table.size() < expected_size) {
		printf("size difference! %d != %d\n", table.size(), expected_size);
	}
	else {
		return false;
	}

	printf("%s -> %s\n", date_toString(table.at(0).date).c_str(), date_toString(table.at(table.size() - 1).date).c_str());
	for (unsigned int i = 0; i < table.size() - 1; i++) {
		int mth_diff = (table.at(i + 1).date.year * 12 + table.at(i + 1).date.month) -
			(table.at(i).date.year * 12 + table.at(i).date.month);

		if (mth_diff > 1) {
			row_t r_inj;
			r_inj.date.year = table.at(i).date.year;
			r_inj.date.month = table.at(i).date.month + 1;

			if (r_inj.date.month > 12) {
				r_inj.date.year++;
				r_inj.date.month = 1;
			}

			r_inj.date.day = 1;
			r_inj.date.minuites = 0;
			r_inj.date.hours = 0;

			r_inj.value.v = NAN;
			r_inj.value.f = true;

			r_inj.variable = table.at(i).variable;

			r_inj.units = table.at(i).units;

			table.push_back(r_inj);
			//printf("intercepted one! %d [%d]  %d / %d\n", i, mth_diff, table.size(), expected_size);
			//goto hyuckin_fix_loop;
			std::sort(table.begin(), table.end(), date_sort);
			i = std::max((unsigned int) 0, i - 2);
		}
	}

	if (table.size() != expected_size) {
		printf("retrying.\n");
		degap_table_monthly(table);
	}

	return true;
}

int read_db(std::ifstream &data, std::vector<row_t> &table) {
	int skipped = 0;
	std::string header;
	//      DATE      TIME      MONTH     YEAR      VAR       VALUE     UNITS     STATE
	data >> header >> header >> header >> header >> header >> header >> header >> header;

	int index = 0;
	std::string line = " ";
	while (data.good()) {
		if (data.eof()) break;
		printf("Reading row %d \r", index);

		std::string date;
		std::string time;
		std::string month;
		std::string year;
		std::string variable;
		std::string value;
		std::string units;
		std::string state;

		if (!data.eof()) data >> date;
		if (!data.eof()) data >> time;
		if (!data.eof()) data >> month;
		if (!data.eof()) data >> year;
		if (!data.eof()) data >> variable;
		if (!data.eof()) data >> value;
		if (!data.eof()) data >> units;
		if (!data.eof()) data >> state;

		if (data.eof()) break;

		row_t row;

		row.date.month = std::stoi(std::string(date.c_str(), 2));
		std::vector<std::string> date_components = split(date, '/');
		row.date.day = std::stoi(date_components.at(1));
		row.date.year = std::stoi(year);
		date_components = split(time, ':');
		row.date.hours = std::stoi(date_components.at(0));
		row.date.minuites = std::stoi(date_components.at(1));
		if (row.date.day != 1) printf("%d %d %d\n", row.date.month, row.date.day, row.date.year);
		row.units = units;
		row.state = std::stoi(state);

		if (value.c_str()[0] != 'N' && value.c_str()[1] != '-' && value.c_str()[0] != 'm') {
			row.value.v = std::stod(value);
		}
		else {
			skipped++;
			row.value.v = NAN;
			row.value.f = true;
		}
		row.variable = variable;
		row.id = index;

		table.push_back(row);

		index++;

	}
	printf("\n");

	return skipped;
}

/*
	expects a table with 1+ variables
*/
std::vector<row_t> normalize_table(std::vector<row_t> t, std::vector<std::string> vars) {
	std::vector<row_t> nt;

	for (std::string s : vars) {
		std::vector<row_t> vt;

		for (row_t r : t)
			if (r.variable == s)
				vt.push_back(r);

		double min = min_table(vt);
		double max = max_table(vt);

		//for (row_t r : vt)
		for (unsigned int i = 0; i < vt.size(); i++)
			vt[i].value.v = (vt[i].value.v - min) / (max - min);

		nt.insert(nt.begin(), vt.begin(), vt.end());
	}

	return nt;
}
