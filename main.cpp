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
#include <stdlib.h> // for random

#include "main.h"
#include "mkt.h"
#include "theilsen.h"
#include "linreg.h"
#include "correlate.h"
#include "util.h"
#include "script.h"
#include "data.h"

agg_t global_min_agg = DAILY;

int main() {
	std::vector<row_t> bad_rows;
	std::vector<std::string> variables;
	std::vector<variable_t> rules;
	srand((unsigned int)time(NULL));
	std::vector<row_t> table;
	std::vector<custom_var_t> cvars;
	char tmp[255];
	int skipped = 0;
	std::string __table_name__ = "newdb_08012018.csv";
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

	std::ofstream output;
	output.open("results\\output.txt");
	if (!output.is_open()) {
		printf("failed to load output \n");
		return -2;
	}
	for (unsigned int i = 0; i < table.size(); i++) {
		printf("Writing row %d \r", i);
		output << table.at(i).id << " ";
		output << table.at(i).date.day << " " << table.at(i).date.month << " " << table.at(i).date.year << " ";
		output << table.at(i).variable << " ";
		output << table.at(i).value.v << std::endl;
	}
	output.close();

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


#if F_REBUILD_TABLE == 1
	printf("Rebuilding table\n");
	std::vector<row_t> table_fixed;
	for (int i = 0; i < variables.size(); i++) {
		std::vector<row_t> local_table;
		std::vector<row_t> local_result_table;

		for (int j = 0; j < table.size(); j++) {
			printf("%d / %d [%d] \r", j + 1, table.size(), i + 1);
			if (variables.at(i) == table.at(j).variable) {
				local_table.push_back(table.at(j));
				table.at(j).id = -50;
			}
		}

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
		resolve_table_state(local_table);

		if (global_min_agg == DAILY) {
			for (int j = 0; j < local_table.size(); j++) {
				printf("%d / %d [%d] \r", j + 1, local_table.size(), i + 1);
				if (local_table.at(j).id == -100) continue;

				row_t r = local_table.at(j);
				local_table.at(j).id = -100;
				r.edits = 0;

				for (int k = 0; k < local_table.size(); k++) {
					//printf("%d %d (%d / %d)\r", j + 1, k + 1, (j + 1) * (k + 1), local_table.size() * local_table.size());
					if (local_table.at(k).id == -100) continue;
					if (k == j) continue;

					if (local_table.at(k).date.year == local_table.at(j).date.year &&
						local_table.at(k).date.month == local_table.at(j).date.month &&
						/*local_table.at(k).date.day == local_table.at(j).date.day &&*/
						local_table.at(k).variable == local_table.at(j).variable) {
						//r.value.v += local_table.at(k).value.v;
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

		degap_table_monthly(local_result_table);

		// fix wierd date stuff
		for (int n = 0; n < local_result_table.size(); n++) {
			table.at(n).date.day = 1;
			table.at(n).date.minuites = 0;
			table.at(n).date.hours = 0;
		}

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

		//table_fixed.reserve(table_fixed.size() + local_result_table.size());
		//table_fixed.insert(table_fixed.begin(), local_result_table.begin(), local_result_table.end());

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

	execute_config(variables, rules, cvars);
	
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
		filter_by_variable(table, t, n, true);

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
	variable_times_file << "Variable, Start Date, End Date, No. Records" << std::endl;
	for (int i = 0; i < variables.size(); i++) {
		std::vector<row_t> vtable;
		bool found = false;
		int cvars_idx = 0;
		for (; cvars_idx < cvars.size(); cvars_idx++) {
			if (cvars.at(cvars_idx).name == variables.at(i)) {
				found = true;
				break;
			}
		}
		
		if (!found) {
			for (int j = 0; j < table.size(); j++) {
				if (table.at(j).variable == variables.at(i)) {
					row_t r = table.at(j);
					vtable.push_back(r);
				}
			}
		}
		else {
			try {
				filter_by_variable(table, vtable, cvars.at(cvars_idx).pieces, true);
			}
			catch (std::exception &e) {
				printf("%s \n", e.what());
			}
		}

		std::sort(vtable.begin(), vtable.end(), date_sort);
		sdates.push_back(vtable.at(0).date);
		edates.push_back(vtable.at(vtable.size() - 1).date);

		variable_times_file << variables.at(i) << ", " << date_toString(vtable.at(0).date) << ", " << date_toString(vtable.at(vtable.size() - 1).date) << ", " << vtable.size() << std::endl;
	}


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

	printf("Executing script out.cfg\n");

	execute_script(std::string("out.cgf"), table, variables, rules, cvars);

	return 635;

	printf("Beginning tests \n");
	std::ofstream results;
#if F_MAKE_CSV == 0
	results.open("results.txt");
	if (!results.is_open()) {
		printf("Failed to open results.txt\n");
		return -1;
	}
#endif

	CreateDirectory(L".\\results", NULL);

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
		for (unsigned int h = 0; h < table.size(); h++) {
			if (table.at(h).variable == "GGT") {
				row_t r = table.at(h);
				ggt.push_back(r);
			}
		}
#if F_MAKE_CSV == 1
		results.open("results\\var_" + variables.at(i) + ".csv", std::ios::trunc);

		if (!results.is_open()) {
			std::string strtmp = "Failed to open results_" + variables.at(i) + ".txt\n";
			printf(strtmp.c_str());
			return -1;
		}
#endif
		// assemble local variable table
		std::vector<row_t> vtable;
		bool found_cv = false;
		for (unsigned int s = 0; s < cvars.size(); s++) {
			//printf("CVAME: %s\n", cvars.at(s).name.c_str());
			if (variables.at(i) == cvars.at(s).name) {
				found_cv = true;
				std::vector<std::string> names(cvars.at(s).pieces);
				filter_by_variable(table, vtable, names, true);
				break;
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
		sprintf(tmp, "DATE, Z, P, SLOPE, CL, A, B, T, GGT, MEAN, STDDEV, CV, MEDIAN\n");
		results << tmp;
#endif

		//printf("reduced by %d", reduce_by_var(ggt, bad_rows, variables.at(i)));
		mk_result_t mkrt;
		mann_kendall_test(mkrt, vtable);
		ts_result_t tsrt1;
		theil_sen(tsrt1, vtable, 2); // 2 because monthly
		linreg_result_t lrrt1;
		linear_regression(lrrt1, vtable);
		double ggt_v = correlation(vtable, ggt);

#if F_MAKE_CSV == 0
		sprintf(tmp, "Year %f %f %f[%s/mth] %f %f[%s/mth] %f[%s] %s %f %f %f %f %f\n", mkrt.z, mkrt.p, tsrt1.slope, vtable.at(0).units.c_str(), tsrt1.r_u - tsrt1.r_l, lrrt1.m, vtable.at(0).units.c_str(), lrrt1.b, vtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v, mean(vtable), std_dev(vtable), coef_var(vtable), median(vtable));
		results << tmp;
#else 
		sprintf(tmp, "Year, %f, %f, %f %s/mth, %f, %f %s/mth, %f %s, %s, %f, %f, %f, %f, %f\n", mkrt.z, mkrt.p, tsrt1.slope, vtable.at(0).units.c_str(), tsrt1.r_u - tsrt1.r_l, lrrt1.m, vtable.at(0).units.c_str(), lrrt1.b, vtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v, mean(vtable), std_dev(vtable), coef_var(vtable), median(vtable));
		results << tmp;
#endif

		// loop through months
#if F_DO_MONTHS == 1
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
			sprintf(tmp, "%s %f %f %f[%s/mth] %f %f[%s/mth] %f[%s] %s %f %f %f %f %f\n", months[j].c_str(), mkrt1.z, mkrt1.p, tsrt.slope, mtable.at(0).units.c_str(), tsrt.r_u - tsrt.r_l, lrrt.m, mtable.at(0).units.c_str(), lrrt.b, mtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v1, mean(mtable), std_dev(mtable), coef_var(mtable), median(mtable));
			results << tmp;
#else 
			sprintf(tmp, "%s, %f, %f, %f %s/yr, %f, %f %s/yr, %f %s, %s, %f, %f, %f, %f, %f\n", months[j].c_str(), mkrt1.z, mkrt1.p, tsrt.slope, mtable.at(0).units.c_str(), tsrt.r_u - tsrt.r_l, lrrt.m, mtable.at(0).units.c_str(), lrrt.b, mtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v1, mean(mtable), std_dev(mtable), coef_var(mtable), median(mtable));
			results << tmp;
#endif
			mtable.clear();
		}

		vtable.clear();

#endif

		results << std::endl;

#if F_MAKE_CSV == 1
		results.close();
#endif

		printf(" done\n");

	}

#if F_MAKE_CSV == 0
	results.close();
#endif
	printf("Generating correlation data \n");
	int cycle_num = 1;
	std::ofstream correlate_data;
	correlate_data.open("results\\correlate.csv", std::ios::trunc);

	if (!correlate_data.is_open()) {
		return -12;
	}

	correlate_data << "X, ";
	for (unsigned int i = 0; i < variables.size(); i++) {
		correlate_data << variables.at(i) << ", ";
	}
	correlate_data << std::endl;

	for (unsigned int i = 0; i < variables.size(); i++) {
#if F_DO_MONTHS == 1
		printf("%d / %d\r", cycle_num, variables.size() + 12 * variables.size());
#else 
		printf("%d / %d\r", cycle_num, variables.size());
#endif
		correlate_data << variables.at(i) << ", ";
		for (unsigned int j = 0; j < variables.size(); j++) {
			std::vector<row_t> tablea;
			std::vector<row_t> tableb;

			// create tablea
			bool found_i = false;
			for (unsigned int l = 0; l < cvars.size(); l++) {
				if (cvars.at(l).name == variables.at(i)) {
					found_i = true;
					std::vector<std::string> names(cvars.at(l).pieces);
					filter_by_variable(table, tablea, names);
					break;
				}
			}
			if (!found_i) {
				for (unsigned int l = 0; l < table.size(); l++) {
					if (table.at(l).variable == variables.at(i)) {
						row_t r = table.at(l);
						tablea.push_back(r);
					}
				}
			} 

			// create tableb
			bool found_j = false;
			for (unsigned int l = 0; l < cvars.size(); l++) {
				if (cvars.at(l).name == variables.at(j)) {
					found_j = true;
					std::vector<std::string> names(cvars.at(l).pieces);
					filter_by_variable(table, tableb, names);
					break;
				}
			}
			if (!found_j) {
				for (unsigned int l = 0; l < table.size(); l++) {
					if (table.at(l).variable == variables.at(j)) {
						row_t r = table.at(l);
						tableb.push_back(r);
					}
				}
			}

			double c = correlation(tablea, tableb);

			//printf("%d ", c);
			correlate_data << c << ", ";
		}
		correlate_data << std::endl;
		cycle_num++;
	}
	correlate_data.close();

#if F_DO_MONTHS == 1
	// months
	for (int k = 1; k <= 12; k++) {
		correlate_data.open("results\\correlate_" + months[k] + ".csv", std::ios::trunc);

		if (!correlate_data.is_open()) {
			return -k * 100;
		}
		//correlate_data << months[k] << std::endl;
		std::vector<row_t> m_table;
		for (unsigned int i = 0; i < table.size(); i++) {
			if (table.at(i).date.month == k) {
				row_t r = table.at(i);
				m_table.push_back(r);
			}
		}

		correlate_data << "[" << months[k] << "], ";
		for (unsigned int i = 0; i < variables.size(); i++) {
			correlate_data << variables.at(i) << ", ";
		}
		correlate_data << std::endl;

		for (unsigned int i = 0; i < variables.size(); i++) {
			correlate_data << variables.at(i) << ", ";
			printf("%d / %d \r", cycle_num, variables.size() + 12 * variables.size());
			for (unsigned int j = 0; j < variables.size(); j++) {
				std::vector<row_t> tablea;
				std::vector<row_t> tableb;

				// create tablea
				bool found_i = false;
				for (unsigned int l = 0; l < cvars.size(); l++) {
					if (cvars.at(l).name == variables.at(i)) {
						found_i = true;
						std::vector<std::string> names(cvars.at(l).pieces);
						filter_by_variable(m_table, tablea, names);
						break;
					}
				}
				if (!found_i) {
					for (unsigned int l = 0; l < m_table.size(); l++) {
						if (m_table.at(l).variable == variables.at(i)) {
							row_t r = m_table.at(l);
							tablea.push_back(r);
						}
					}
				}

				// create tableb
				bool found_j = false;
				for (unsigned int l = 0; l < cvars.size(); l++) {
					if (cvars.at(l).name == variables.at(j)) {
						found_j = true;
						std::vector<std::string> names(cvars.at(l).pieces);
						filter_by_variable(m_table, tableb, names);
						break;
					}
				}
				if (!found_j) {
					for (unsigned int l = 0; l < m_table.size(); l++) {
						if (m_table.at(l).variable == variables.at(j)) {
							row_t r = m_table.at(l);
							tableb.push_back(r);
						}
					}
				}

				double c = correlation(tablea, tableb);

				//printf("%d ", c);
				correlate_data << c << ", ";
			}
			correlate_data << std::endl;
			cycle_num++;
		}
		correlate_data << std::endl;
		correlate_data.close();
	}
#endif
	printf("\n");
	std::vector<row_t> t_salinity;
	std::vector<row_t> t_sac4;
	std::vector<row_t> t_sjq4;
	std::vector<row_t> t_sea_level;
	
	printf("cvars size %d\n", cvars.size());
	for (unsigned int m = 0; m < cvars.size(); m++) {
		printf("CV: %d %s %d : ", m, cvars.at(m).name.c_str(), cvars.at(m).pieces.size());
		for (unsigned int zx = 0; zx < cvars.at(m).pieces.size(); zx++) {
			printf("[%s] ", cvars.at(m).pieces.at(zx).c_str());
		}
		printf("\n");
	}
	for (unsigned int i = 0; i < table.size(); i++) {
		//printf("[%d] %s ", i, table.at(i).variable.c_str());
		if (table.at(i).variable == "GGT") {
			row_t r = table.at(i);
			t_sea_level.push_back(r);
			//printf("GGT");
		}
		//printf("\n");
	}
	std::vector<std::string> namesa(cvars.at(0).pieces);
	filter_by_variable(table, t_sac4, namesa);
	std::vector<std::string> namesb(cvars.at(1).pieces);
	filter_by_variable(table, t_sjq4, namesb);
	std::vector<std::string> namesc(cvars.at(2).pieces);
	filter_by_variable(table, t_salinity, namesc);

	//printf("\n");
	printf("%d %d %d \n", t_sac4.size(), t_sjq4.size(), t_salinity.size());
	printf("Generating partial correlation table\n");
	std::ofstream pcorfile;
	pcorfile.open("results\\pcorfile.csv");
	if (!pcorfile.is_open()) {
		printf("could not open file \n");
		return -963;
	}
	//printf("opened\n");
	pcorfile << "X, SAC4_by_Sea_Level, SJQ4_by_Sea_Level, Sea_Level_by_SAC4, Sea_Level_by_SJQ4," << std::endl;
	// SJQ4
	for (int i = 0; i < cvars.at(2).pieces.size(); i++) {
		printf("%d / %d \n", i + 1, cvars.at(2).pieces.size());
		std::vector<row_t> salinity;
		for (int j = 0; j < table.size(); j++) {
			if (table.at(j).variable == cvars.at(2).pieces.at(i)) {
				row_t r = table.at(j);
				salinity.push_back(r);
			}
		}
		sprintf(tmp, "%s, %f, %f, %f, %f\n",
			cvars.at(2).pieces.at(i).c_str(),
			partial_correlate(salinity, t_sac4, t_sea_level),
			partial_correlate(salinity, t_sjq4, t_sea_level),
			partial_correlate(salinity, t_sea_level, t_sac4),
			partial_correlate(salinity, t_sea_level, t_sjq4));
		pcorfile << tmp;
	}
	pcorfile.close();
	printf("\n");

	return 0;
}

bool execute_config(std::vector<std::string> &variables, std::vector<variable_t> &rules, std::vector<custom_var_t> &cvars) {
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
