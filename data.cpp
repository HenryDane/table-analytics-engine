#include <vector>
#include <fstream>
#include <algorithm>
#include "main.h"
#include "util.h"

void resolve_table_state(std::vector<row_t> &table){
	if (table[0].state == 65 ||
		table[0].state == 17) return;

	for (int i = 0; i < table.size(); i++) {
		printf("\t\t\t\t%d / %d\r", i + 1, table.size());
		for (int j = i; j < table.size(); j++) {
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
				}
			}
		}
	}
	
}

int old_table_agg(std::vector<row_t> &table) {
	std::vector<row_t> table_agg;
	for (int i = 0; i < table.size(); i++) {
		row_t r = table.at(i);

		// is it already in table_agg
		bool found = false;
		int j = 0;
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
}

void period_filter(std::vector<row_t> &table, period_t period) {
	//period_t period = { "All", YEARLY,{ 0, 0, 0, 0, 0 },{ 9999, 99, 99, 99, 99 } };
	//period_t period = { "Pre_WP", YEARLY, { 0, 0, 0 }, { 1967, 9, 31 } };
	//period_t period = { "Post_WP", YEARLY, { 1967, 10, 0 }, { 2017, 9, 31 } };
	//period_t period = { "Pre_SMSCG", YEARLY, { 0, 0, 0 }, { 1987, 9, 31 } };
	//period_t period = { "Post_SMSCG", YEARLY, { 1987, 10, 1 }, { 9999, 99, 99 } };
	for (int i = 0; i < table.size(); i++) {
		if (!date_in_period(table.at(i).date, period)) {
			table.erase(table.begin() + i);
			i = std::max(0, i - 2);
		}
	}
}

// removes gaps from individual var's ts.
bool degap_table_monthly(std::vector<row_t> &table) {
//hyuckin_fix_loop:
	std::sort(table.begin(), table.end(),date_sort);

	date_t start = table.at(0).date;
	date_t end = table.at(table.size() - 1).date;

	int expected_size = ((end.year - start.year) * 12) + (end.month - start.month) /*i think*/;

	if (table.size() < expected_size) {
		printf("size difference! %d != %d\n", table.size(), expected_size);
	}
	else {
		return false;
	}

	printf("%s -> %s\n", date_toString(table.at(0).date).c_str(), date_toString(table.at(table.size() - 1).date).c_str());
	for (int i = 0; i < table.size() - 1; i++) {
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
			printf("intercepted one! %d [%d]  %d / %d\n", i, mth_diff, table.size(), expected_size);
			//goto hyuckin_fix_loop;
			std::sort(table.begin(), table.end(), date_sort);
			i = std::max(0, i - 2);
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

		// skip stuff we dont like
		if (variable == "100.4-TSL" ||
			variable == "092.8-BLP" ||
			variable == "077.0-PTS" ||
			variable == "075.0-MAL" ||
			variable == "022.0-PSP" ||
			variable == "ORI" ||
			variable == "LPS" ) {
			continue;
		}

		if (data.eof()) break;
		//		std::cout << "DAT: [" << date << "][" << month << "][" << year << "][" << variable << "][" << value << "][" << units << "]" << std::endl;

		row_t row;

		row.date.month = std::stoi(std::string(date.c_str(), 2));
		//row.date.month = -1;
		//row.date.day = 1; // monthly data only
		std::vector<std::string> date_components = split(date, '/');
		row.date.day = std::stoi(date_components.at(1));
		row.date.year = std::stoi(year);
		date_components = split(time, ':');
		row.date.hours = std::stoi(date_components.at(0));
		row.date.minuites = std::stoi(date_components.at(1));
		if (row.date.day != 1) ("%d %d %d\n", row.date.month, row.date.day, row.date.year);
		row.units = units;
		row.state = std::stoi(state);

		if (value.c_str()[0] != 'N' && value.c_str()[1] != '-') {
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