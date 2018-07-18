#define _CRT_SECURE_NO_WARNINGS // look I like sprintf() ok?

#include <vector>
#include <algorithm>
#include <stdio.h>
#include "main.h"

bool is_inside(std::vector<std::string> vec, std::string str) {
	for (unsigned int i = 0; i < vec.size(); i++) {
		if (str.compare(vec.at(i)) == 0) {
			return true;
		}
	}
	return false;
}

double min_table(std::vector<row_t> &table) {
	double min_val = 0;
	for (unsigned int i = 0; i < table.size(); i++) {
		min_val = std::min(min_val, table.at(i).value.v);
	}
	return min_val;
}

double max_table(std::vector<row_t> &table) {
	double max_val = 0;
	for (unsigned int i = 0; i < table.size(); i++) {
		max_val = std::max(max_val, table.at(i).value.v);
	}
	return max_val;
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

date_t min_table_date(std::vector<row_t> &table) {
	date_t date = { 9999, 99, 99 }; // YYYY MM DD
	for (unsigned int i = 0; i < table.size(); i++) {
		if (date_as_day(table.at(i).date) < date_as_day(date)) {
			date = table.at(i).date;
		}
	}

	return date;
}

date_t max_table_date(std::vector<row_t> &table) {
	date_t date = { 0000, 00, 00 }; // YYYY MM DD
	for (unsigned int i = 0; i < table.size(); i++) {
		if (date_as_day(table.at(i).date) > date_as_day(date)) {
			date = table.at(i).date;
		}
	}

	return date;
}

bool date_sort(row_t i, row_t j) {
	if (date_as_day(i.date) < date_as_day(j.date)) {
		return true;
	}

	return false;
}

void filter_by_variable(std::vector<row_t> table, std::vector<row_t> &otable, std::vector<std::string> varnames, bool merge) {
	std::vector<row_t> ltable;
	otable.clear();

	for (unsigned int i = 0; i < table.size(); i++) {
		for (unsigned int j = 0; j < varnames.size(); j++) {
			if (table.at(i).variable == varnames.at(j)) {
				//if (strstr(table.at(i).variable.c_str(), varnames.at(j).c_str())){
				row_t row = table.at(i);
				ltable.push_back(row);
			}
		}
	}

	if (merge) {
		for (unsigned int i = 0; i < ltable.size() - 1; i++) {
			for (unsigned int j = i; j < ltable.size(); j++) {
				if (i == j) {
					continue;
				}
				if (ltable.at(i).date.day == ltable.at(j).date.day &&
					ltable.at(i).date.month == ltable.at(j).date.month &&
					ltable.at(i).date.year == ltable.at(j).date.year) {
					ltable.at(i).value.v += ltable.at(j).value.v;
					ltable.at(j).id = -100;
					ltable.at(j).value.v = 0;
				}
			}
		}

		for (unsigned int i = 0; i < ltable.size(); i++) {
			if (ltable.at(i).id != -100) {
				row_t r = ltable.at(i);
				otable.push_back(r);
			}
		}
	}
	else {
		otable = ltable;
	}
#if F_MAKE_FILTER_FILES == 1
	std::ofstream file;
	std::string name = "filter_" + std::to_string(rand()) + ".txt";
	file.open(name);
	if (!file.is_open()) {
		printf("failed to write to [%s]\n", name.c_str());
		return;
	}
#endif 

	try {
		std::sort(otable.begin(), otable.end(), date_sort);
	}
	catch (const std::exception &e) {
		//std::cout << e.what() << std::endl;
		printf("%s\n", e.what());
	}

#if F_MAKE_FILTER_FILES == 1
	for (unsigned int i = 0; i < otable.size(); i++) {
		file << otable.at(i).id << " ";
		file << otable.at(i).date.month << " ";
		file << otable.at(i).date.day << " ";
		file << otable.at(i).date.year << " ";
		file << otable.at(i).variable << " ";
		file << otable.at(i).value << " ";
		file << std::endl;
	}

	file.close();
#endif
	return;
}

bool is_equal(date_t &a, date_t &b) {
	if (a.day == b.day) {
		if (a.month == b.month) {
			if (a.year == b.year) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

date_t min_date(date_t &a, date_t &b) {
	if (date_as_day(a) > date_as_day(b)) {
		return b;
	}
	else {
		return a;
	}
}

date_t max_date(date_t &a, date_t &b) {
	if (date_as_day(a) > date_as_day(b)) {
		return a;
	}
	else {
		return b;
	}
}

/*
removes corresponding dates from table by searching through list of bad dates from
bad rows selected by varname
*/
int reduce_by_var(std::vector<row_t> table, std::vector<row_t> bad_rows, std::string varname) {
	int edits = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		bool found = false;

		for (unsigned int j = 0; j < bad_rows.size(); j++) {
			if (is_equal(bad_rows.at(j).date, table.at(i).date) && table.at(i).variable == varname) {
				found = true;
			}
		}

		if (found) {
			edits++;
			table.erase(table.begin() + i);
		}
	}

	return edits;
}

void make_unique(std::vector<std::string> &vec) {
	sort(vec.begin(), vec.end());
	vec.erase(unique(vec.begin(), vec.end()), vec.end());
}

bool startsWith(std::string mainStr, std::string toMatch) {
	// std::string::find returns 0 if toMatch is found at starting
	if (mainStr.find(toMatch) == 0)
		return true;
	else
		return false;
}

std::string date_toString(date_t &a) {
	char tmp[16];
	sprintf(tmp, "%02d/%02d/%04d", a.month, a.day, a.year);
	return std::string(tmp);
}


double date_as_month(date_t &date) {
	return (date_as_day(date)) / DAYS_PER_TYP_MONTH;
}

double date_as_year(date_t &date) {
	return (date_as_day(date)) / 365;
}

