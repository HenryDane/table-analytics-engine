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
#define NOMINMAX // use std not windows

#include <stdio.h> // printf and sprintf
#include <fstream> // files
#include <string> // std::string & co.
#include <vector> // because arrays are too hard
#include <iostream> // files and std::cout
#include <algorithm> // special functions for conversions and such
#include <Windows.h> // for CreateDirectory
#include <time.h> // for random
#include <stdlib.h> // for random ( and system() )
#include "main.h"
#include "mkt.h"
#include "theilsen.h"
#include "linreg.h"
#include "correlate.h"
#include "util.h"
#include "script.h"
#include "data.h"
#include "scimath.h"
#include "macros.h"

agg_t global_min_agg = DAILY;

//#include "Python.h"

int main() {
	std::vector<row_t> bad_rows;
	std::vector<std::string> variables;
	std::vector<variable_t> rules;
	srand((unsigned int)time(NULL));
	std::vector<row_t> table;
	std::vector<custom_var_t> cvars;
	char tmp[255];
	int skipped = 0;
	std::string __table_name__ = "_compile.stxt";
	int __table_suffix__ = rand() % 1000;

	printf("Table Analytics Engine v2.1.5 \n");
	DeleteDirectory(L".\\results");
	DeleteDirectory(L".\\ts");
	CreateDirectory(L".\\results", NULL);
	CreateDirectory(L".\\ts", NULL);

	printf("Reading data\n");
	std::ifstream data;
	data.open(__table_name__);
	if (!data.is_open()) {
		printf("Failed to load input \n");
		return -1;
	}
	skipped = read_db(data, table);
	printf("Resolved %d nulls\n", skipped);
	data.close();

	printf("Identifying variables \n");
	for (unsigned int i = 0; i < table.size(); i++) {
		printf("Reading row %d \r", i);

		bool found = false;
		if (i > table.size()) i = 0;

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


#if F_REBUILD_TABLE == 1
	printf("Rebuilding table\n");
	std::vector<row_t> table_fixed;
	for (int i = 0; i < variables.size(); i++) {
		//printf("loopstart\n");
		std::vector<row_t> local_table;
		std::vector<row_t> local_result_table;

		//printf("efio\n");
		for (int j = 0; j < table.size(); j++) {
			printf("%d / %d [%d] \r", j + 1, table.size(), i + 1);
			if (variables.at(i) == table.at(j).variable) {
				local_table.push_back(table.at(j));
				table.at(j).id = -50;
			}
		}

		//printf("efji\n");
		std::sort(table.begin(), table.end(), table_id_sort);
		for (int d = 0; d < table.size(); d++) {
			if (table.at(d).id > -50) {
				if (d > 0 && d < table.size()) {
					table.erase(table.begin(), table.begin() + d - 1);
				}
				break;
			}
		}

		// fix state differences
		//printf("yeet\n");
		resolve_table_state(local_table);

		if (global_min_agg == DAILY) {
			for (int j = 0; j < local_table.size(); j++) {
				printf("%d / %d [%d] \r", j + 1, local_table.size(), i + 1);
				if (local_table.at(j).id == -100) continue;

				row_t r = local_table.at(j);
				local_table.at(j).id = -100;
				r.edits = 1;

				for (int k = 0; k < local_table.size(); k++) {
					//printf("%d %d (%d / %d)\r", j + 1, k + 1, (j + 1) * (k + 1), local_table.size() * local_table.size());
					if (local_table.at(k).id == -100) continue;
					if (k == j) continue;

					if (local_table.at(k).date.year == local_table.at(j).date.year &&
						local_table.at(k).date.month == local_table.at(j).date.month &&
						/*local_table.at(k).date.day == local_table.at(j).date.day &&*/
						local_table.at(k).variable == local_table.at(j).variable) {
						r.value = safe_add(r.value, local_table.at(k).value);
						r.edits++;
						local_table.at(k).id = -100;
					}
				}

				if (r.edits > 0)
					r.value.v = r.value.v / r.edits;

				local_result_table.push_back(r);
			}
		}

		//printf("dgm\n");
		degap_table_monthly(local_result_table);
		//printf("fixin stuff %d\n", local_result_table.size());
		// fix wierd date stuff
		for (unsigned int n = 0; n < local_result_table.size(); n++) {
			local_result_table.at(n).date.day = 1;
			local_result_table.at(n).date.minuites = 0;
			local_result_table.at(n).date.hours = 0;
			local_result_table.at(n).id++;
		}
		//printf("writin\n");
		std::ofstream outtmp;
		if (i == 0) {
			outtmp.open(__table_name__ + "." + std::to_string(__table_suffix__), std::ios::trunc);
		}
		else {
			outtmp.open(__table_name__ + "." + std::to_string(__table_suffix__), std::ios::app);
		}
		if (!outtmp.is_open()) {
			printf("err\n");
		}
		//printf("stuff2 %d \n", local_result_table.size());
		for (int m = 0; m < local_result_table.size(); m++) {
			//outtmp << local_result_table.at(m).id << " ";
			outtmp << date_toString(local_result_table.at(m).date) << " ";
			outtmp << local_result_table.at(m).date.month << " ";
			outtmp << local_result_table.at(m).date.year << " ";
			outtmp << local_result_table.at(m).variable << " ";
			outtmp << local_result_table.at(m).value.v << " ";
			outtmp << local_result_table.at(m).units << " ";
			outtmp << local_result_table.at(m).state << " ";
			outtmp << std::endl;
		}
		outtmp.close();
		
		//gen_plot(local_result_table);

		//table_fixed.reserve(table_fixed.size() + local_result_table.size());
		//table_fixed.insert(table_fixed.begin(), local_result_table.begin(), local_result_table.end());
		//printf("loopend\n");
	}

	printf("\nOld size: %d\n", table.size());
	table.clear();
	data.open(__table_name__ + "." + std::to_string(__table_suffix__));
	printf("Resolved %d nulls \n", read_db(data, table));
	data.close();
	printf("New size: %d\n", table.size());
#endif

	printf("Done cleaning data\n");

#if F_AGG_YEARLY == 1

#endif
	
	std::vector<std::string> scripts;
	execute_config(variables, rules, cvars, scripts);
	// gen cvar ts data
	std::ofstream dataout;
	dataout.open(__table_name__ + "." + std::to_string(__table_suffix__), std::ios::app);
	for (int i = 0; i < cvars.size(); i++) {
		std::vector<row_t> t;

		filter_by_variable(table, t, cvars.at(i).pieces, cvars.at(i).name, true, true);

		for (int j = 0; j < t.size(); j++) {
			dataout << date_toString(t.at(j).date) << " ";
			dataout << t.at(j).date.month << " ";
			dataout << t.at(j).date.year << " ";
			dataout << t.at(j).variable << " ";
			dataout << t.at(j).value.v << " ";
			dataout << t.at(j).units << " ";
			dataout << t.at(j).state << " ";
			dataout << std::endl;
		}
	}
	dataout.close();

	printf("\nOld size: %d\n", table.size());
	table.clear();
	data.open(__table_name__ + "." + std::to_string(__table_suffix__));
	printf("Resolved %d nulls \n", read_db(data, table));
	data.close();
	printf("New size: %d\n", table.size());

#if F_GEN_TS_ALL == 1
	CreateDirectory(L".\\ts", NULL);
	for (int i = 0; i < variables.size(); i++) {
		std::ofstream ofile;
		std::string fname = "ts\\" + variables.at(i) + ".csv";
		ofile.open(fname);
		if (!ofile.is_open()) {
			printf("Could not open %s\n", fname.c_str());
			return -234;
		}

		ofile << "'ID,DATE,MTH,YEAR,VAR,VAL,UNITS" << std::endl;
		for (int j = 0; j < table.size(); j++) {
			if (table.at(j).variable == variables.at(i)) {
				ofile << table.at(j).id << ",";
				ofile << date_toString(table.at(j).date).c_str() << ",";
				ofile << table.at(j).date.month << ",";
				ofile << table.at(j).date.year << ",";
				ofile << table.at(j).variable << ",";
				//ofile << ((table.at(j).value.f) ? NAN : table.at(j).value.v) << ",";
				if (table.at(j).value.f) {
					ofile << "NAN,";
				}
				else {
					ofile << table.at(j).value.v << ",";
				}
				ofile << table.at(j).units << std::endl;
			}
		}
		ofile.close();
	}

	
	for (int i = 0; i < cvars.size(); i++) {
		std::ofstream ofile;
		std::string fname = "ts\\" + cvars.at(i).name + ".csv";
		ofile.open(fname);
		if (!ofile.is_open()) {
			printf("Could not open %s\n", fname.c_str());
			return -234;
		}

		std::vector<std::string> n = cvars.at(i).pieces;
		std::vector<row_t> t;
		filter_by_variable(table, t, n, " ", true);

		for (int j = 0; j < t.size(); j++) {
			ofile << t.at(j).id << ",";
			ofile << date_toString(t.at(j).date).c_str() << ",";
			ofile << t.at(j).date.month << ",";
			ofile << t.at(j).date.year << ",";
			ofile << t.at(j).variable << ",";
			//ofile << ((t.at(j).value.f) ? NAN : t.at(j).value.v) << ",";
			if (t.at(j).value.f) {
				ofile << "NAN,";
			}
			else {
				ofile << t.at(j).value.v << ",";
			}
			ofile << t.at(j).units << std::endl;
		}
		
		ofile.close();
	}
#endif

	printf("Collecting variable metadata\n");
	std::vector<date_t> sdates;
	std::vector<date_t> edates;
	std::ofstream variable_times_file;
	variable_times_file.open("results\\vt_ranges.csv");
	if (!variable_times_file.is_open()) {
		printf("could not save vt_ranges.txt\n");
		return -563;
	}
	variable_times_file << "Num Vars:," << variables.size() << std::endl;
	variable_times_file << "Variable, Start Date, End Date, No. Records, Units" << std::endl;
	//printf("vs: %d\n", variables.size());
	for (int i = 0; i < variables.size(); i++) {
		//printf("i: %d\n", i);
		std::vector<row_t> vtable;
		//printf("f: %d\n", found);
		
		for (int j = 0; j < table.size(); j++) {
			if (table.at(j).variable == variables.at(i)) {
				row_t r = table.at(j);
				vtable.push_back(r);
			}
		}

		//printf("%d\n", vtable.size());

		std::sort(vtable.begin(), vtable.end(), date_sort);
		sdates.push_back(vtable.at(0).date);
		edates.push_back(vtable.at(vtable.size() - 1).date);

		variable_times_file << variables.at(i) << ", " << date_toString(vtable.at(0).date) << ", " << date_toString(vtable.at(vtable.size() - 1).date) << ", " << vtable.size() << "," << vtable.at(0).units << std::endl;
	}

	variable_times_file.close();


	std::sort(sdates.begin(), sdates.end(), date_sort_dr);
	std::sort(edates.begin(), edates.end(), date_sort_d);

#if F_CLIP_GLOBAL == 1
	printf("Current table size: %d\n", table.size());
table_cleanup_begin:
	for (int i = 0; i < table.size(); i++) {
		if (date_as_day(table.at(i).date) < date_as_day(sdates.at(0)) ||
			date_as_day(table.at(i).date) > date_as_day(edates.at(0))) {
			table.erase(table.begin() + i);
			goto table_cleanup_begin;
		}
	}
	printf("Updated table size: %d\n", table.size());
	variable_times_file << "Clipped to, " << date_toString(sdates.at(0)).c_str() << ", " << date_toString(edates.at(0)).c_str() << ", " << table.size() << std::endl;
	variable_times_file.close();
#endif 

	printf("Executing scripts\n");

	for (int i = 0; i < scripts.size(); i++) {
		printf("Executing %s ...\n", scripts.at(i).c_str());
		execute_script(scripts.at(i).c_str(), table, variables, rules, cvars);
	}

	do_console(table, cvars, variables, rules);

#if F_STOP_AT_SCRIPTS == 1
	return 0;
#endif

	//execute_main_analysis(table, variables, cvars);
	std::vector<period_t> periods;
	period_t period = { "All", YEARLY,{ 0, 0, 0, 0, 0 },{ 9999, 99, 99, 99, 99 } }; periods.push_back(period);
	//period = { "Pre_WP", YEARLY, { 0, 0, 0 }, { 1967, 9, 31 } }; periods.push_back(period);
	period = { "Post_WP", YEARLY, { 1967, 10, 0 }, { 2017, 9, 31 } }; periods.push_back(period);
	period = { "Pre_SMSCG", YEARLY, { 0, 0, 0 }, { 1987, 9, 31 } }; periods.push_back(period);
	period = { "Post_SMSCG", YEARLY, { 1987, 10, 1 }, { 9999, 99, 99 } }; periods.push_back(period);
	execute_main_full_analysis(table, variables, periods, cvars); 
	execute_main_analysis_correlate(table, variables, cvars);

	return 0;
}

bool execute_config(std::vector<std::string> &variables, 
		std::vector<variable_t> &rules, 
		std::vector<custom_var_t> &cvars, 
		std::vector<std::string> &scripts) {

	std::ifstream cfg;
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

	printf("Loaded %d custom variables\n", cvars.size());

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
				printf("Bad length value [%s]\n", e.what());
				return false;
			}
			//printf("checking length %d \n", length);

			// select all varuiables with lengfth less than X
			//printf("%d is size \n", variables.size());
			for (unsigned int j = 0; j < variables.size(); j++) {
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

			for (unsigned int j = 0; j < variables.size(); j++) {
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
	for (unsigned int i = 0; i < variables.size(); i++) {
		variable_t r;
		r.name = variables.at(i);
		for (unsigned int j = 0; j < rules.size(); j++) {
			if (variables.at(i) == rules.at(j).name) {
				r.correlations.insert(r.correlations.end(), rules.at(j).correlations.begin(), rules.at(j).correlations.end());
			}
		}

		// resolve internal duplicates
		make_unique(r.correlations);

		nrules.push_back(r);
	}

	rules = nrules;

	printf("Loaded %d correlation rules \n", rules.size());
	for (unsigned int i = 0; i < rules.size(); i++) {
		//printf("[Rule %d] %s : ", i, rules.at(i).name.c_str());
		for (unsigned int j = 0; j < rules.at(i).correlations.size(); j++) {
			//printf("%s ", rules.at(i).correlations.at(j).c_str());
		}
		//printf("\n");
	}

	cfg >> tmp >> tmp2;
	if (tmp != "#" || tmp2 != "SCRIPTS") {
		printf("bad script list [%s] [%s] \n", tmp.c_str(), tmp2.c_str());
		return false;
	}
	int n_scripts = 0;
	cfg >> n_scripts;
	for (int i = 0; i < n_scripts; i++) {
		cfg >> tmp;
		scripts.push_back(tmp);
	}
	printf("Loaded %d scripts\n", scripts.size());

	cfg.close();
	return true;
}

bool apply_rules(std::vector<std::string> &apply_to, std::vector<std::string> &rules_list, std::vector<std::string> &variables, std::vector<variable_t> &rules) {
	std::vector<std::string> accepted_variables;
	for (unsigned int j = 0; j < rules_list.size(); j++) {
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
			for (unsigned int k = 0; k < variables.size(); k++) {
				if (variables.at(k).length() <= length) {
					accepted_variables.push_back(variables.at(k));
				}
			}
		} else if (rules_list.at(j).c_str()[0] == '-') {
			std::string todel = rules_list.at(j).substr(1);
reloop: // awful hack
			for (unsigned int m = 0; m < accepted_variables.size(); m++) {
				if (startsWith(accepted_variables.at(m), todel)) {
					accepted_variables.erase(accepted_variables.begin() + m);
					goto reloop;
				}
			}
		} else if (rules_list.at(j).c_str()[0] == '&'){
			std::string sw = rules_list.at(j).substr(1);

			for (unsigned int l = 0; l < variables.size(); l++) {
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
	for (unsigned int i = 0; i < apply_to.size(); i++) {
		variable_t rule;
		std::vector<std::string> v;
		v.resize(accepted_variables.size() * 5);
		if (accepted_variables.size() == 0) {
			for (unsigned int z = 0; z < rules.size(); z++) {
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

	return true;
}
