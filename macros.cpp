#include <vector>
#include <string>
#include <fstream>
#include <time.h> // for random
#include <stdlib.h> // for random ( and system() )
#include <Windows.h>
#include <algorithm>
#include "main.h"
#include "correlate.h"
#include "theilsen.h"
#include "mkt.h"
#include "linreg.h"
#include "util.h"

int execute_main_analysis(std::vector<row_t> table, std::vector<std::string> variables, std::vector<custom_var_t> cvars) {
	char tmp[80];
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
		std::vector<row_t> ndo;
		for (unsigned int h = 0; h < table.size(); h++) {
			if (table.at(h).variable == "Mean_Tide") {
				row_t r = table.at(h);
				ggt.push_back(r);
			}
			else if (table.at(h).variable == "NDO") {
				row_t r = table.at(h);
				ndo.push_back(r);
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
				filter_by_variable(table, vtable, names, " ", true);
				break;
			}
		}
		if (!found_cv) {
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
		sprintf(tmp, "DATE Z P SLOPE CL A B T GGT MEAN STDDEV CV NDO\n");
		results << tmp;
#else
		sprintf(tmp, "%s, From %d/%d/%d to %d/%d/%d,,,,,\n", variables.at(i).c_str(), min_table_date(vtable).month, min_table_date(vtable).day, min_table_date(vtable).year, max_table_date(vtable).month, max_table_date(vtable).day, max_table_date(vtable).year);
		results << tmp;
		sprintf(tmp, "DATE, Z, P, SLOPE, CL, A, B, T, GGT, MEAN, STDDEV, CV, MEDIAN, NDO\n");
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
		double ndo_v = correlation(vtable, ndo);

#if F_MAKE_CSV == 0
		sprintf(tmp, "Year %f %f %f[%s/mth] %f %f[%s/mth] %f[%s] %s %f %f %f %f %f %f\n", mkrt.z, mkrt.p, tsrt1.slope, vtable.at(0).units.c_str(), tsrt1.r_u - tsrt1.r_l, lrrt1.m, vtable.at(0).units.c_str(), lrrt1.b, vtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v, mean(vtable), std_dev(vtable), coef_var(vtable), median(vtable));
		results << tmp;
#else 
		sprintf(tmp, "Year, %f, %f, %f %s/mth, %f, %f %s/mth, %f %s, %s, %f, %f, %f, %f, %f, %f\n", mkrt.z, mkrt.p, tsrt1.slope, vtable.at(0).units.c_str(), tsrt1.r_u - tsrt1.r_l, lrrt1.m, vtable.at(0).units.c_str(), lrrt1.b, vtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v, mean(vtable), std_dev(vtable), coef_var(vtable), median(vtable), ndo_v);
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
			std::vector<row_t> ndo_m;
			for (unsigned int k = 0; k < table.size(); k++) {
				if (table.at(k).variable == "Mean_Tide" && table.at(k).date.month == j) {
					row_t r = table.at(k);
					ggt_m.push_back(r);
				}
				else if (table.at(k).variable == "NDO" && table.at(k).date.month == j) {
					row_t r = table.at(k);
					ndo_m.push_back(r);
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
			double ndo_v1 = correlation(mtable, ndo_m);

#if F_MAKE_CSV == 0	
			sprintf(tmp, "%s %f %f %f[%s/mth] %f %f[%s/mth] %f[%s] %s %f %f %f %f %f %f\n", months[j].c_str(), mkrt1.z, mkrt1.p, tsrt.slope, mtable.at(0).units.c_str(), tsrt.r_u - tsrt.r_l, lrrt.m, mtable.at(0).units.c_str(), lrrt.b, mtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v1, mean(mtable), std_dev(mtable), coef_var(mtable), median(mtable), ndo_v1);
			results << tmp;
#else 
			sprintf(tmp, "%s, %f, %f, %f %s/yr, %f, %f %s/yr, %f %s, %s, %f, %f, %f, %f, %f, %f\n", months[j].c_str(), mkrt1.z, mkrt1.p, tsrt.slope, mtable.at(0).units.c_str(), tsrt.r_u - tsrt.r_l, lrrt.m, mtable.at(0).units.c_str(), lrrt.b, mtable.at(0).units.c_str(), mkrt.trend.c_str(), ggt_v1, mean(mtable), std_dev(mtable), coef_var(mtable), median(mtable), ndo_v1);
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

				double c;

				try {
					c = correlation(tablea, tableb);
				}
				catch (std::exception &e) {
					printf("Near fatal error while computing correlation between\n    [%s] with %d records and\n    [%s] with %d records\n", tablea.at(0).variable.c_str(), tablea.size(), tableb.at(0).variable.c_str(), tableb.size());
					printf("    Error is [%s]\n", e.what());
				}

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
		if (table.at(i).variable == "Mean_Tide") {
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
	for (unsigned int i = 0; i < cvars.at(2).pieces.size(); i++) {
		printf("%d / %d \r", i + 1, cvars.at(2).pieces.size());
		std::vector<row_t> salinity;
		for (unsigned int j = 0; j < table.size(); j++) {
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

	printf("Done\n");
}

std::vector<row_t> filter_table(std::vector<row_t> t, period_t p, std::string v, int mth) {
	std::vector<row_t> r;
	r.clear();

	for (row_t s : t) {
		if (date_in_period(s.date, p) &&
			s.variable == v &&
			s.date.month == ((mth == 0) ? s.date.month : mth)) {
			r.push_back(s);
		}
	}

	return r;
}

std::vector<row_t> make_tables(std::vector<row_t> table, std::string variable, int mth, period_t p, std::vector<custom_var_t> cvars) {
	std::vector<row_t> ltable = filter_table(table, p, variable, mth);

	if (ltable.size() == 0) {
		// make custom var
		for (custom_var_t cvt : cvars) {
			if (cvt.name == variable) {
				filter_by_variable(table, ltable, cvt.pieces, cvt.name, true, true);

				// flag out of bounds
				for (row_t r : ltable) {
					if (!date_in_period(r.date, p)) {
						r.id = -1;
					}
				}

				// sort by id with lowest first
				std::sort(ltable.begin(), ltable.end(), table_id_sort);

				// erase all neg values
				for (unsigned int i = 0; i < ltable.size(); i++) {
					if (ltable[i].id >= 0) {
						ltable.erase(ltable.begin(), ltable.begin() + i);
					}
				}
			}
		}
	}

	return ltable;
}

int execute_main_full_analysis(std::vector<row_t> table, std::vector<std::string> variables, std::vector<period_t> periods, std::vector<custom_var_t> cvars) {
	printf("Beginning tests\n");

	CreateDirectory(L".\\results_full", NULL);

	for (unsigned int i = 0; i < variables.size(); i++) { // loop for each variable
		std::ofstream results;
		results.open(".\\results_full\\var_" + variables[i] + ".csv");
		if (!results.is_open()) {
			printf("Unable to open results file\n");
			return -1;
		}
		
		results << "DATE,Z,P,T,Slope,A,B,R,Mean,CV,Median,NDO,Mean Tide" << std::endl;

		for (unsigned int j = 0; j < periods.size(); j++) { // loop for each period
			results << std::endl << "Period: " << periods[j].toString() << std::endl;
			for (int k = 0; k <= 12; k++) {
				// print progress
				printf("%d / %d ; %d / %d ; %d / %d        \r", i + 1, variables.size(), j + 1, periods.size(), k, 12);
				
				// make tables
				std::vector<row_t> ltable = filter_table(table, periods[j], variables[i], k);
				if (ltable.size() == 0) {
					// make custom var
					for (custom_var_t cvt : cvars) {
						if (cvt.name == variables[i]) {
							filter_by_variable(table, ltable, cvt.pieces, cvt.name, true, true);
							
							// flag out of bounds
							for (row_t r : ltable) {
								if (!date_in_period(r.date, periods[j])) {
									r.id = -1;
								}
							}

							// sort by id with lowest first
							std::sort(ltable.begin(), ltable.end(), table_id_sort);

							// erase all neg values
							for (unsigned int i = 0; i < ltable.size(); i++) {
								if (ltable[i].id >= 0) {
									ltable.erase(ltable.begin(), ltable.begin() + i);
								}
							}
						}
					}
				}

				// write header
				if (ltable.size() <= 0) {
					results << "Failed to analyze period " << periods.at(j).toString() << std::endl;
					continue;
				}
				else {
					results << ((k == 0) ? "All Months" : months[k] + " only") << ",";
				}
				std::vector<row_t> ggt = filter_table(table, periods[j], "Mean_Tide", k);
				std::vector<row_t> ndo = filter_table(table, periods[j], "NDO", k);

				// run tests
				mk_result_t mkrt;
				ts_result_t tsrt;
				linreg_result_t lnrt;
				mann_kendall_test(mkrt, ltable);
				theil_sen(tsrt, ltable);
				linear_regression(lnrt, ltable);
				double c_ggt = correlation(ltable, ggt);
				double c_ndo = correlation(ltable, ndo);

				// write results
				//results << "Period analyzed: " << periods[j].toString() << std::endl;
				results << mkrt.z << "," << mkrt.p << "," << mkrt.trend << ",";
				results << tsrt.slope << ",";
				results << lnrt.m << "," << lnrt.b << "," << lnrt.r << ",";
				results << mean(ltable) << "," << coef_var(ltable) << "," << median(ltable) << ",";
				results << c_ndo << "," << c_ggt << std::endl;
				//results << std::endl;

			}
		}

		results.close();
	}

	return 0;
}

void execute_main_analysis_correlate(std::vector<row_t> table, std::vector<std::string> variables, std::vector<custom_var_t> cvars) {
	period_t p =  { "*", agg_t::NONE, { 0, 0, 0, 0, 0 }, { 9999, 99, 99, 99, 99 }};

	int idx = 0;

	for (int i = 0; i <= 12; i++) {
		std::ofstream results(".\\results_full\\corr_" + ((i == 0) ? "yearly" : months[i]) + ".csv");
		
		for (std::string v : variables)
			results << "," << v;
		results << std::endl;

		for (std::string v : variables) {
			results << v << ",";
			for (std::string w : variables) {
				printf("%d / %d      \r", idx, 13 * variables.size() * variables.size());
				std::vector<row_t> tv = make_tables(table, v, i, p, cvars);
				std::vector<row_t> tw = make_tables(table, w, i, p, cvars);

				results << correlation(tv, tw) << ",";
				idx++;
			}

			results << std::endl;
		}

		results.close();
	}
}