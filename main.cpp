/*
Mann-Kendall-Test application

Inputs:
- data.csv : A space seperated file containing the following (not space tolerant)
DATE : A mm/dd/yyyy formatted date (monthly data only -- day is hardcoded to 1)
VARIABLE : A string with the name of a viariable
VALUE : The associated value of the variable

Outputs:
- result.txt : A text file containing results for each variable included in the data file
*/

#define _CRT_SECURE_NO_WARNINGS // so I can use sprintf

#include <stdio.h> // printf and sprintf
#include <fstream> // files
#include <string> // std::string & co.
//#include <sstream> 
#include <vector> 
#include <iostream> 
#include <algorithm> // special functions for conversions and such

#include <Windows.h> // for CreateDirectory

#include <time.h> // for random
#include <stdlib.h> // for random

#include "main.h"
#include "mkt.h"
#include "theilsen.h"
#include "linreg.h"
#include "correlate.h"

#include "wavelet2d.h"

int main() {
	std::vector<row_t> bad_rows;
	std::vector<std::string> variables;
	std::vector<variable_t> rules;
	srand((unsigned int)time(NULL));
	std::vector<row_t> table;
	std::vector<custom_var_t> cvars;
	char tmp[255];
	int skipped = 0;

	printf("Mann-Kendall Test v0.0.1 \n");
	// printf("F_SKIP_NULLS = %d \t F_CORRECT_TIES = %d \t DAYS_PER_TYP_MONTH = %f \n", F_SKIP_NULLS, F_CORRECT_TIES, DAYS_PER_TYP_MONTH);

	printf("Reading data\n");
	std::ifstream data;
	data.open("sf_flow_data_edit_v5.csv");
	if (!data.is_open()) {
		printf("Failed to load input \n");
		return -1;
	}

	std::string header;
	data >> header >> header >> header >> header >> header >> header;

	int index = 0;
	std::string line = " ";
	while (data.good()) {
		if (data.eof()) break;
		printf("Reading row %d \r", index);

		std::string date;
		std::string month;
		std::string year;
		std::string variable;
		std::string value;
		std::string units;

		if (!data.eof()) data >> date;
		if (!data.eof()) data >> month;
		if (!data.eof()) data >> year;
		if (!data.eof()) data >> variable;
		if (!data.eof()) data >> value;
		if (!data.eof()) data >> units;

		if (data.eof()) break;
		//		std::cout << "DAT: [" << date << "][" << month << "][" << year << "][" << variable << "][" << value << "][" << units << "]" << std::endl;

		row_t row;

		row.date.month = std::stoi(std::string(date.c_str(), 2));
		//row.date.month = -1;
		row.date.day = 1; // monthly data only
		row.date.year = std::stoi(year);
		row.units = units;

		if (value.c_str()[0] != 'N' && value.c_str()[1] != '-') {
			row.value = std::stod(value);
		}
		else {
#if F_SKIP_NULLS == 1
			//printf("Skipping null \n");
			skipped++;
			continue;
#elif F_SKIP_NULLS == 2
			row.value = 0;
#elif F_SKIP_NULLS == 3
			skipped++;
			row.variable = variable;
			row.id = index;
			bad_rows.push_back(row);
			continue;
#else
			row.value = NAN;
#endif
		}
		row.variable = variable;
		row.id = index;

		table.push_back(row);

		index++;

	}
	printf("\n");
	printf("Skipped %d nulls \n", skipped);
	data.close();

	std::ofstream output;
	output.open("output.txt");
	if (!output.is_open()) {
		printf("failed to load output \n");
		return -2;
	}
	for (unsigned int i = 0; i < table.size(); i++) {
		output << table.at(i).id << " ";
		output << table.at(i).date.day << " " << table.at(i).date.month << " " << table.at(i).date.year << " ";
		output << table.at(i).variable << " ";
		output << table.at(i).value << std::endl;
	}

	printf("Identifying variables \n");
	for (unsigned int i = 0; i < table.size(); i++) {
		printf("Reading row %d \r", i);

		bool found = false;
		for (unsigned int j = 0; j < variables.size(); j++) {
			if (variables.at(j) == table.at(i).variable) {
				found = true;
				break;
			}
		}
		if (!found) {
			variables.push_back(table.at(i).variable);
		}
	}
	printf("\n");

#if F_PRINT_VARS == 1
	for (unsigned int i = 0; i < variables.size(); i++) {
		printf("%s ", variables.at(i).c_str());
	}
	printf("\n");
#endif
	//return 2; // leave early
	printf("%d is c size \n", cvars.size());
	execute_config(variables, rules, cvars);
	printf("%d is c  size \n", cvars.size());
	//return 2;

	printf("Beginning tests \n");
	std::ofstream results;
#if F_MAKE_CSV == 0
	results.open("results.txt");
	if (!results.is_open()) {
		printf("Failed to open results.txt\n");
		return -1;
	}
#endif

#if F_MAKE_DIRS == 1
	CreateDirectory(L".\\results", NULL);
#endif

	time_t time_now;
	time(&time_now);
	struct tm* tm_info = localtime(&time_now);
	strftime(tmp, 50, "%Y-%m-%d %H:%M:%S \n", tm_info);
#if F_MAKE_CSV == 0
	results << "TIME: " << tmp;
	results << "DROPPED " << skipped << " RECORDS" << std::endl << std::endl;
#endif

	// analyze each variable
	for (unsigned int i = 0; i < variables.size(); i++) {
		std::vector<row_t> ggt;
		for (unsigned int i = 0; i < table.size(); i++) {
			row_t r = table.at(i);
			ggt.push_back(r);
		}
#if F_MAKE_CSV == 1
#if F_MAKE_DIRS == 1
		results.open("results\\" + variables.at(i) + ".csv", std::ios::trunc);
#else 
		results.open("results_" + variables.at(i) + ".csv", std::ios::trunc);
#endif
		if (!results.is_open()) {
			std::string strtmp = "Failed to open results_" + variables.at(i) + ".txt\n";
			printf(strtmp.c_str());
			return -1;
		}
#endif
		// assemble local variable table
		std::vector<row_t> vtable;
		bool found_cv = false;
		for (int s = 0; s < cvars.size(); s++) {
			//printf("CVAME: %s\n", cvars.at(s).name.c_str());
			if (variables.at(i) == cvars.at(s).name) {
				found_cv = true;
				std::vector<std::string> names;
				std::copy(cvars.at(s).pieces.begin(), cvars.at(s).pieces.end(), names.begin());
				//printf("%s : {", cvars.at(s).name);
				//for (int p = 0; p < cvars.at(s).pieces.size(); p++) {
				//	printf("%s ", cvars.at(s).pieces.at(p).c_str());
				//}
				//printf("}\n");
				filter_by_variable(table, vtable, names);
			}
		}
		if (!found_cv)  {
			for (unsigned int j = 0; j < table.size(); j++) {
				if (table.at(j).variable == variables.at(i)) {
					row_t row = table.at(j);
					vtable.push_back(row);
				}
			}
		}

		sprintf(tmp, "(%d / %d) Var: [%s] . . .", vtable.size(), table.size(), variables.at(i).c_str());
		//sprintf(tmp, "%d [%s] . . .", vtable.size(), variables.at(i).c_str());
		printf(tmp);
#if F_MAKE_CSV == 0
		sprintf(tmp, "VAR %s, From %d/%d/%d to %d/%d/%d using %d records of %d records total\n", variables.at(i).c_str(), min_table_date(vtable).month, min_table_date(vtable).day, min_table_date(vtable).year, max_table_date(vtable).month, max_table_date(vtable).day, max_table_date(vtable).year, vtable.size(), table.size());
		results << tmp;
		sprintf(tmp, "DATE Z P SLOPE CL A B T GGT MEAN STDDEV CV\n");
		results << tmp;
#else
		sprintf(tmp, "%s, From %d/%d/%d to %d/%d/%d,,,,,\n", variables.at(i).c_str(), min_table_date(vtable).month, min_table_date(vtable).day, min_table_date(vtable).year, max_table_date(vtable).month, max_table_date(vtable).day, max_table_date(vtable).year);
		results << tmp;
		sprintf(tmp, "DATE, Z, P, SLOPE, CL, A, B, T, GGT, MEAN, STDDEV, CV\n");
		results << tmp;
#endif

		//printf("reduced by %d", reduce_by_var(ggt, bad_rows, variables.at(i)));
		mk_result_t mkrt;
		mann_kendall_test(mkrt, vtable);
		ts_result_t tsrt1;
		theil_sen(tsrt1, vtable);
		linreg_result_t lrrt1;
		linear_regression(lrrt1, vtable);
		double ggt_v = correlation(vtable, ggt, true);

#if F_MAKE_CSV == 0
		sprintf(tmp, "Year %f %f %f[%s/mth] %f %f[%s/mth] %f[%s] %s %f %f %f %f\n", mkrt.z, mkrt.p, tsrt1.slope, vtable.at(0).units.c_str(), tsrt1.r_u - tsrt1.r_l, lrrt1.m, vtable.at(0).units.c_str(), lrrt1.b, vtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v, mean(vtable), std_dev(vtable), coef_var(vtable));
		results << tmp;
#else 
		sprintf(tmp, "Year, %f, %f, %f %s/mth, %f, %f %s/mth, %f %s, %s, %f, %f, %f, %f\n", mkrt.z, mkrt.p, tsrt1.slope, vtable.at(0).units.c_str(), tsrt1.r_u - tsrt1.r_l, lrrt1.m, vtable.at(0).units.c_str(), lrrt1.b, vtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v, mean(vtable), std_dev(vtable), coef_var(vtable));
		results << tmp;
#endif

		// loop through months
		for (int j = 1; j <= 12; j++) {
			std::vector<row_t> mtable;
			for (unsigned int k = 0; k < vtable.size(); k++) {
				if (vtable.at(k).date.month == j) {
					row_t row = vtable.at(k);
					mtable.push_back(row);
				}
			}

			std::vector<row_t> ggt_m;
			for (unsigned int k = 0; k < table.size(); k++) {
				if (table.at(k).variable == "GGT" && table.at(k).date.month == j) {
					row_t r = table.at(k);
					ggt_m.push_back(r);
				}
			}
			//printf("reduced by %d", reduce_by_var(ggt_m, bad_rows, variables.at(i)));

			mk_result_t mkrt1;
			mann_kendall_test(mkrt1, mtable);
			ts_result_t tsrt;
			theil_sen(tsrt, mtable);
			linreg_result_t lrrt;
			linear_regression(lrrt, mtable);
			double ggt_v1 = correlation(mtable, ggt_m);

#if F_MAKE_CSV == 0	
			sprintf(tmp, "%s %f %f %f[%s/mth] %f %f[%s/mth] %f[%s] %s %f %f %f %f\n", months[j].c_str(), mkrt1.z, mkrt1.p, tsrt.slope, mtable.at(0).units.c_str(), tsrt.r_u - tsrt.r_l, lrrt.m, mtable.at(0).units.c_str(), lrrt.b, mtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v1, mean(mtable), std_dev(mtable), coef_var(mtable));
			results << tmp;
#else 
			sprintf(tmp, "%s, %f, %f, %f %s/yr, %f, %f %s/yr, %f %s, %s, %f, %f, %f, %f\n", months[j].c_str(), mkrt1.z, mkrt1.p, tsrt.slope, mtable.at(0).units.c_str(), tsrt.r_u - tsrt.r_l, lrrt.m, mtable.at(0).units.c_str(), lrrt.b, mtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v1, mean(mtable), std_dev(mtable), coef_var(mtable));
			results << tmp;
#endif
			mtable.clear();
		}

		vtable.clear();

		results << std::endl;

#if F_MAKE_CSV == 1
		results.close();
#endif

		printf(" done\n");

	}

#if F_MAKE_CSV == 0
	results.close();
#endif

	return 0;
}

double min_table(std::vector<row_t> &table) {
	double min_val = 0;
	for (unsigned int i = 0; i < table.size(); i++) {
		min_val = min(min_val, table.at(i).value);
	}
	return min_val;
}

double max_table(std::vector<row_t> &table) {
	double max_val = 0;
	for (unsigned int i = 0; i < table.size(); i++) {
		max_val = max(max_val, table.at(i).value);
	}
	return max_val;
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

void filter_by_variable(std::vector<row_t> &table, std::vector<row_t> &otable, std::vector<std::string> varnames, bool merge) {
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
					ltable.at(i).value += ltable.at(j).value;
					ltable.at(j).id = -100;
					ltable.at(j).value = 0;
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
		std::cout << e.what() << std::endl;
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
	return ((a.day == b.day) && (a.month == b.month) && (a.year == b.year));
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

bool execute_config(std::vector<std::string> &variables, std::vector<variable_t> &rules, std::vector<custom_var_t> &cvars) {
	ifstream cfg;
	cfg.open("config.txt");

	if (!cfg.is_open()) {
		printf("Could not load config\n");
		return false;
	}

	std::string tmp;
	std::string tmp2;

	// check header
	cfg >> tmp >> tmp2;
	if (tmp != "~" || tmp2 != "CONFIG") {
		printf("Bad config header\n");
		return false;
	}

	// read custom vars
	int n_c_vars;
	tmp = " ";
	tmp2 = " ";
	// clean this up!
	cfg.clear();
	cfg >> tmp >> tmp2;
	if (tmp != "#" || tmp2 != "CUSTOM") {
		printf("bad custom header [%s] [%s]\n", tmp.c_str(), tmp2.c_str());
		return false;
	}
	cfg >> n_c_vars;
	for (int i = 0; i < n_c_vars; i++) {
		custom_var_t cvt;
		std::string cv_name;
		int n_components;

		cfg >> cv_name;
		cfg >> n_components;

		variables.push_back(cv_name);

		for (int j = 0; j < n_components; j++) {
			std::string component;
			cfg >> component;
			cvt.pieces.push_back(component);
		}

		cvt.name = cv_name;

		cvars.push_back(cvt);
	}

	printf("%d custom vars\n", cvars.size());

	// correlation
	int n_correlation_rules = 0;
	cfg >> tmp >> tmp2;
	if (tmp != "#" || tmp2 != "CORRELATE") {
		printf("Bad correlation header");
	}
	cfg >> n_correlation_rules;
	for (int i = 0; i < n_correlation_rules; i++) {
		//printf("parsing %d\n", i);
		std::string selection = " ";
		int n_assoc = -1;
		std::vector<std::string> rules_list;

		cfg >> selection;
		cfg >> n_assoc;

		for (int j = 0; j < n_assoc; j++) {
			std::string t;
			cfg >> t;
			rules_list.push_back(t);
		}

		//printf("%d is size! \n", variables.size());

		if (selection.c_str()[0] == '*') {
			// applying to all variables
			//rules.clear();
			apply_rules(variables, rules_list, variables, rules);
		} else if (selection.c_str()[0] == '_') {
			// applying to all with length less than X
			std::string len = selection.substr(1);
			std::vector<std::string> apply_to;
			int length;
			try {
				length = std::stoi(len);
			} catch (std::exception &e) {
				printf("Bad length value\n");
				return false;
			}
			//printf("checking length %d \n", length);

			// select all varuiables with lengfth less than X
			//printf("%d is size \n", variables.size());
			for (int j = 0; j < variables.size(); j++) {
			//	printf("len of %d (%s) is %d \n", j, variables.at(j).c_str(), variables.at(j).length());
				if (variables.at(j).length() <= length) {
					apply_to.push_back(variables.at(j));
				}
			}

			//printf("apply to: ");
			//for (int h = 0; h < apply_to.size(); h++) {
			//	printf("%s ", apply_to.at(h).c_str());
			//} 
			//printf("\n");
			apply_rules(apply_to, rules_list, variables, rules);
		} else if (selection.c_str()[0] == '&') {
			std::string startsw = selection.substr(1);
			std::vector<std::string> apply_to;

			for (int j = 0; j < variables.size(); j++) {
				if (startsWith(variables.at(j), startsw)) {
					apply_to.push_back(variables.at(j));
				}
			}

			apply_rules(apply_to, rules_list, variables, rules);
		} else {
			std::vector<std::string> apply_to;
			apply_to.push_back(selection);
			apply_rules(apply_to, rules_list, variables, rules);
		}
	}

	// resolve duplicates inclusively
	std::vector<variable_t> nrules;
	for (int i = 0; i < variables.size(); i++) {
		variable_t r;
		r.name = variables.at(i);
		for (int j = 0; j < rules.size(); j++) {
			if (variables.at(i) == rules.at(j).name) {
				r.correlations.insert(r.correlations.end(), rules.at(j).correlations.begin(), rules.at(j).correlations.end());
			}
		}

		// resolve internal duplicates
		make_unique(r.correlations);

		nrules.push_back(r);
	}

	rules = nrules;

	printf("%d Rules found\n", rules.size());
	for (int i = 0; i < rules.size(); i++) {
		printf("[Rule %d] %s : ", i, rules.at(i).name.c_str());
		for (int j = 0; j < rules.at(i).correlations.size(); j++) {
			printf("%s ", rules.at(i).correlations.at(j).c_str());
		}
		printf("\n");
	}

	cfg.close();
}

void make_unique(std::vector<std::string> &vec) {
	sort(vec.begin(), vec.end());
	vec.erase(unique(vec.begin(), vec.end()), vec.end());
}

bool apply_rules(std::vector<std::string> &apply_to, std::vector<std::string> &rules_list, std::vector<std::string> &variables, std::vector<variable_t> &rules) {
	std::vector<std::string> accepted_variables;
	for (int j = 0; j < rules_list.size(); j++) {
		if (rules_list.at(j).c_str()[0] == '*') {
			// select all
			std::copy(variables.begin(), variables.end(), accepted_variables.begin());
		}
		else if (rules_list.at(j).c_str()[0] == '!') {
			// clear selected
			accepted_variables.clear();
		}
		else if (rules_list.at(j).c_str()[0] == '_') {
			// up to length of X
			std::string len = rules_list.at(j).substr(1);
			int length;
			try {
				length = std::stoi(len);
			}
			catch (std::exception &e) {
				printf("Bad length value [%s]\n", e.what());
				return false;
			}

			// search variables
			for (int k = 0; k < variables.size(); k++) {
				if (variables.at(k).length() <= length) {
					accepted_variables.push_back(variables.at(k));
				}
			}
		} else if (rules_list.at(j).c_str()[0] == '-') {
			std::string todel = rules_list.at(j).substr(1);
reloop: // awful hack
			for (int m = 0; m < accepted_variables.size(); m++) {
				if (startsWith(accepted_variables.at(m), todel)) {
					accepted_variables.erase(accepted_variables.begin() + m);
					goto reloop;
				}
			}
		} else if (rules_list.at(j).c_str()[0] == '&'){
			std::string sw = rules_list.at(j).substr(1);

			for (int l = 0; l < variables.size(); l++) {
				if (startsWith(variables.at(l), sw)) {
					accepted_variables.push_back(variables.at(l));
				}
			}
		} else {
			// it is a literal
			accepted_variables.push_back(rules_list.at(j));
		}
	}

	// clean up uniques
	make_unique(accepted_variables);

	// apply changes
	for (int i = 0; i < apply_to.size(); i++) {
		variable_t rule;
		std::vector<std::string> v;
		v.resize(accepted_variables.size() * 5);
		if (accepted_variables.size() == 0) {
			for (int z = 0; z < rules.size(); z++) {
				if (rules.at(z).name == apply_to.at(i)) {
					rules.erase(rules.begin() + z);
				}
			}
			//printf("nothing allowd\n");
			break;
		}
		try {
			std::copy(accepted_variables.begin(), accepted_variables.end(), v.begin());
		} catch (std::exception &e) {
			printf("%s\n", e.what());
			return false;
		}
		rule.correlations = v;
		rule.name = apply_to.at(i);
		rules.push_back(rule);
	}
}

bool startsWith(std::string mainStr, std::string toMatch){
	// std::string::find returns 0 if toMatch is found at starting
	if (mainStr.find(toMatch) == 0)
		return true;
	else
		return false;
}