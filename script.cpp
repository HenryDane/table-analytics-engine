#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>

#include "main.h"
#include "util.h"
#include "correlate.h"
#include "mkt.h"
#include "theilsen.h"
#include "linreg.h"

void yearly_agg(std::vector<row_t> &tbl) {
	std::vector<row_t> table_agg;
	for (int i = 0; i < tbl.size(); i++) {
		row_t r = tbl.at(i);

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
			tbl.erase(tbl.begin() + i);
			i = std::max(0, i - 5);
		}
		else {
			table_agg.push_back(r);
			tbl.erase(tbl.begin() + i);
			i = std::max(0, i - 5);
		}
	}

	tbl = table_agg;
}

bool execute_token(std::ifstream &cfg,
	std::string &this_token,
	std::vector<std::string> &tokens,
	std::ofstream &out,
	int &state,
	std::string &current_var,
	std::vector<row_t> &ltable,
	std::vector<row_t> table,
	std::vector<macro_t> &macros,
	std::vector<custom_var_t> &cvars,
	std::vector<period_t> &periods,
	script_flag_t flags,
	std::string &macrovar,
	std::vector<list_t> &lists,
	std::string &agg_var,
	bool debug) {

	if (debug) {
		printf("\n\n");
		printf("+===========================================+\n");
		printf(" ltable: %d tokens: %d\n", ltable.size(), tokens.size());
		printf(" current_var: %s macrovar: %s\n [%s]\n", current_var.c_str(), macrovar.c_str(), this_token.c_str());
		printf("+===========================================+\n");
	}

	if (tokens.at(0) == "NL") {
		out << std::endl;
	}
	else if (tokens.at(0) == "BL") {
		out << ",";
	}
	else if (tokens.at(0) == "STOP") {
		state = -255;
		printf("STOP called, exiting\n");
		return false;
	}
	else if (tokens.at(0) == "C") {
		if (debug) printf("Found comment\n");
	}
	else if (tokens.at(0) == "TEXT") {
		if (tokens.at(1) == "__VAR__") {
			out << current_var << ",";
		}
		else if (tokens.at(1) == "__MVR__") {
			out << macrovar << ",";
		}
		else if (tokens.at(1) == "__LIST_C__") {
			for (int i = 0; i < lists.size(); i++) {
				if (lists.at(i).name == tokens.at(2)) {
					for (int j = 0; j < lists.at(i).items.size(); j++) {
						out << lists.at(i).items.at(j) << ",";
					}
					break;
				}
			}
		}
		else {
			if (!flags.f_underscore_is_space) {
				out << tokens.at(1) << ",";
			}
			else {
				replaceAll(tokens.at(1), std::string("_"), std::string(" "));
				out << tokens.at(1) << ",";
			}
		}
	}
	else if (tokens.at(0) == "SYS") {
		if (tokens.at(1) == "OUT") {
			if (tokens.at(2) == "__VAR__") {
				if (tokens.at(3) == "__MVR__") {
					current_var = macrovar;

					// make table
					int count = 0;
					ltable.clear();
					for (int i = 0; i < table.size(); i++) {
						if (table.at(i).variable == current_var) {
							ltable.push_back(table.at(i));
							count++;
						}
					}

					if (ltable.size() == 0) {
						if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
						int idx = 0;
						bool found = false;

						for (; idx < cvars.size(); idx++) {
							if (cvars.at(idx).name == current_var) {
								found = true;
								break;
							}
						}

						if (found) {
							if (debug) printf("S:O:V found %s!\n", current_var.c_str());
							filter_by_variable(table, ltable, cvars.at(idx).pieces);
						}
					}
					if (debug) printf("SYS:OUT:__VAR__ loaded %d records of [%s]\n", ltable.size(), current_var.c_str());
				} else {
					current_var = tokens.at(3);

					// make table
					int count = 0;
					ltable.clear();
					for (int i = 0; i < table.size(); i++) {
						if (table.at(i).variable == current_var) {
							ltable.push_back(table.at(i));
							count++;
						}
					}

					if (ltable.size() == 0) {
						if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
						int idx = 0;
						bool found = false;

						for (; idx < cvars.size(); idx++) {
							if (cvars.at(idx).name == current_var) {
								found = true;
								break;
							}
						}

						if (found) {
							if (debug) printf("S:O:V found %s!\n", current_var.c_str());
							filter_by_variable(table, ltable, cvars.at(idx).pieces);
						}
					}
					if (debug) printf("SYS:OUT:__VAR__ loaded %d records of [%s]\n", ltable.size(), current_var.c_str());
				}
			}
			else if (tokens.at(2) == "__AGG__") {
				agg_var = tokens.at(3);
			}
			else if (tokens.at(2) == "__MVR__") {
				if (tokens.at(3) == "__VAR__") {
					macrovar = current_var;
				}
				else {
					macrovar = tokens.at(3);
				}
			}
			else {
				if (debug) printf("Err !");
			}
		}
		else if (tokens.at(1) == "SELF") {
			if (tokens.at(2) == "RESET") {
				lists.clear();
				macros.clear();
				periods.clear();
				ltable.clear();
				current_var = "__UNDEFINED__";
				macrovar = "__UNDEFINED__";
				tokens.clear();
			}
			else if (tokens.at(2) == "CRASH") {
				abort();
			}
		}
		else if (tokens.at(1) == "REGISTRY") {
			if (tokens.at(2) == "PERIOD") {
				period_t p;
				p.name = tokens.at(3);

				p.aggregation = NONE;

				p.start.year = std::stoi(tokens.at(4));
				p.start.month = std::stoi(tokens.at(5));
				p.start.day = std::stoi(tokens.at(6));

				p.end.year = std::stoi(tokens.at(7));
				p.end.month = std::stoi(tokens.at(8));
				p.end.day = std::stoi(tokens.at(9));
				periods.push_back(p);
				if (debug) printf("SYS:REGISTRY:PERIOD loaded %s successfully\n", tokens.at(3).c_str());
			} else if (tokens.at(2) == "LIST") {
				if (tokens.at(3) == "CREATE") {
					list_t l;
					l.name = tokens.at(4);
					lists.push_back(l);
					if (debug) printf("list size: %d\n", lists.size());
				}
				else if (tokens.at(3) == "ADD") {
					int i = 0;
					for (; i < lists.size(); i++) {
						if (lists.at(i).name == tokens.at(4)) {
							lists.at(i).items.push_back(tokens.at(5));
						}
					}
				}
				else if (tokens.at(3) == "FCVAR") {
					list_t l;
					l.name = tokens.at(4);
					
					int i = 0;
					bool found = false;
					for (; i < cvars.size(); i++) {
						if (cvars.at(i).name == tokens.at(5)) {
							found = true;
							break;
						}
					}

					std::copy(cvars.at(i).pieces.begin(), cvars.at(i).pieces.end(), std::back_inserter(l.items));
				}
				else if (tokens.at(3) == "MERGE") {
					int i = 0;
					int j = 0;
					bool foundi = false;
					bool foundj = false;
					for (; i < lists.size(); i++) {
						if (lists.at(i).name == tokens.at(4)) {
							foundi = true;
							break;
						}
					}
					for (; j < lists.size(); j++) {
						if (lists.at(j).name == tokens.at(5)) {
							foundj = true;
							break;
						}
					}

					// reserve size in both vectors
					lists.at(i).items.reserve(lists.at(i).items.size() + lists.at(j).items.size());
					//lists.at(j).items.reserve(lists.at(i).items.size() + lists.at(j).items.size());

					lists.at(i).items.insert(lists.at(i).items.end(), lists.at(j).items.begin(), lists.at(j).items.end());
				}
				else if (tokens.at(3) == "ADDALL") {
					int idx = 0;
					bool found = false;
					for (; idx < lists.size(); idx++) {
						if (lists.at(idx).name == tokens.at(4)) {
							found = true;
							break;
						}
					}
					
					std::vector<std::string> variables;
					for (int i = 0; i < table.size(); i++) {
						bool f = false;

						for (int j = 0; j < variables.size(); j++) {
							if (variables.at(j) == table.at(i).variable) {
								f = true;
								break;
							}
						}

						if (!f) {
							variables.push_back(table.at(i).variable);
						}
					}

					for (int m = 0; m < variables.size(); m++) {
						lists.at(idx).items.push_back(variables.at(m));
					}

					if (debug) printf("Scanned %d items into list %s at %d\n", lists.at(idx).items.size(), lists.at(idx).name.c_str(), idx);
				}
				else if (tokens.at(3) == "SCAN") {
					int i = 0;
					bool found = false;
					for (; i < lists.size(); i++) {
						if (lists.at(i).name == tokens.at(4)) {
							found = true;
							break;
						}
					}

					if (found) {
						std::vector<std::string> st;
						while (1) {
							std::string s;
							cfg >> s;
							if (s != "ELIST") {
								if (debug) printf("%d\n", i);
								lists.at(i).items.push_back(s);
							}
							else {
								break;
							}
						}
					}
					else {
						if (debug) printf("could not find list \n");
					}

					if (debug) printf("Scanned %d items into list %s\n", lists.at(i).items.size(), lists.at(i).name.c_str());
				}
			}
		}
		else if (tokens.at(1) == "FILTER") {
			if (tokens.at(2) == "PERIOD") {
				//printf("SYS:FILTER:PERIOD [%s] is unsupported\n", tokens.at(3).c_str());

				if (flags.f_auto_reset_filters) {
					if (debug) printf("SYS:FILTER:PERIOD resetting table\n");
					if (debug) printf("S:F:P unsupported in conjunction with auto_reset\n");
					abort();
					// reset table
					//int count = 0;
					ltable.clear();
					for (int i = 0; i < table.size(); i++) {
						if (table.at(i).variable == current_var) {
							ltable.push_back(table.at(i));
							//count++;
						}
					}
				}

				int idx = 0;
				bool found = false;
				for (; idx < periods.size(); idx++) {
					if (periods.at(idx).name == tokens.at(3)) {
						found = true;
						break;
					}
				}

				if (found) {
					if (debug) printf("S:F:P found %s at %d\n", tokens.at(3).c_str(), idx);
					for (int i = 0; i < ltable.size(); i++) {
						if (date_as_day(ltable.at(i).date) > date_as_day(periods.at(idx).end) ||
							date_as_day(ltable.at(i).date) < date_as_day(periods.at(idx).start)) {
							ltable.at(i).id = -100;
						}
					}

					for (int i = 0; i < ltable.size(); i++) {
						if (ltable.at(i).id == -100) {
							ltable.erase(ltable.begin() + i);
							i = std::max(0, i - 2);
						}
					}
				}
				if (debug) printf("SYS:FILTER:PERIOD filtered by period [%s] successfully (%d)\n", tokens.at(3).c_str(), ltable.size());
			}
			else if (tokens.at(2) == "RESET") {
				// make table
				int count = 0;
				ltable.clear();
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				if (ltable.size() == 0) {
					if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
					int idx = 0;
					bool found = false;

					for (; idx < cvars.size(); idx++) {
						if (cvars.at(idx).name == current_var) {
							found = true;
							break;
						}
					}

					if (found) {
						if (debug) printf("S:O:V found %s!\n", current_var.c_str());
						filter_by_variable(table, ltable, cvars.at(idx).pieces);
					}
				}
				if (debug) printf("SYS:FILTER:RESET reloaded %d records\n", ltable.size());
			}
			else {
				if (debug) printf("SYS:FILTER [%s] not recognized\n", tokens.at(2).c_str());
			}
		}
		else if (tokens.at(1) == "AGG") {
			//printf("SYS:AGG [%s] is not yet supported\n", tokens.at(2).c_str());
			if (tokens.at(2) == "__AGG__") {
				tokens.at(2) = agg_var;
			}

			if (tokens.at(2) == "Yearly") {
				if (debug) printf("SYS:AGG:YEAR Initial:%d ", ltable.size());
				yearly_agg(ltable);
				if (debug) printf("Final:%d\n", ltable.size());
			}
			else if (tokens.at(2) == "Monthly") {
				// make table
				int count = 0;
				ltable.clear();
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				if (ltable.size() == 0) {
					if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
					int idx = 0;
					bool found = false;

					for (; idx < cvars.size(); idx++) {
						if (cvars.at(idx).name == current_var) {
							found = true;
							break;
						}
					}

					if (found) {
						if (debug) printf("S:O:V found %s!\n", current_var.c_str());
						filter_by_variable(table, ltable, cvars.at(idx).pieces);
					}
				}
				if (debug) printf("SYS:FILTER:RESET reloaded %d records\n", ltable.size());
				if (debug) printf("SYS:AGG:MONTH -> SYS:FILTER:RESET reloaded %d records\n", count);
			}
		}
	}
	else if (tokens.at(0) == "VARS") {
		if (tokens.at(1) == "__VAR__" || tokens.at(1) == ".") {
			if (tokens.at(2) == "INFO") {
				if (tokens.at(3) == "InitialDate") {
					out << date_toString(min_table_date(ltable)) << ",";
				}
				else if (tokens.at(3) == "FinalDate") {
					out << date_toString(max_table_date(ltable)) << ",";
				}
				else if (tokens.at(3) == "Mean") {
					out << mean(ltable) << ",";
				}
				else if (tokens.at(3) == "Median") {
					out << median(ltable) << ",";
				}
				else if (tokens.at(3) == "StdDev") {
					out << std_dev(ltable) << ",";
				}
				else if (tokens.at(3) == "CV") {
					out << coef_var(ltable) << ",";
				}
				else if (tokens.at(3) == "N") {
					out << ltable.size() << ",";
				}
			}
			else if (tokens.at(2) == "TESTS") {
				if (tokens.at(3) == "MK") {
					if (tokens.at(4) == "p") {
						mk_result_t mkrt;
						mann_kendall_test(mkrt, ltable);
						out << mkrt.p << ",";
					}
					else if (tokens.at(4) == "z") {
						mk_result_t mkrt;
						mann_kendall_test(mkrt, ltable);
						out << mkrt.z << ",";
					}
					else if (tokens.at(4) == "h") {
						mk_result_t mkrt;
						mann_kendall_test(mkrt, ltable);
						out << mkrt.h << ",";
					}
					else if (tokens.at(4) == "trend") {
						mk_result_t mkrt;
						mann_kendall_test(mkrt, ltable);
						out << mkrt.trend << ",";
					}
					else if (tokens.at(4) == "tau") {
						out << mann_kendall_tau(ltable) << ",";
					}
					else if (tokens.at(4) == "mtau") {
						std::vector<row_t> atable;
						for (int i = 0; i < table.size(); i++) {
							if (table.at(i).variable == tokens.at(5)) {
								atable.push_back(table.at(i));
							}
						}
						out << mann_kendall_tau(ltable, atable) << ",";
					}
				}
				else if (tokens.at(3) == "TS") {
					if (tokens.at(4) == "Slope") {
						ts_result_t tsrt;
						theil_sen(tsrt, ltable);
						out << tsrt.slope << ",";
					}
				}
				else if (tokens.at(3) == "LINREG") {
					if (tokens.at(4) == "A") {
						linreg_result_t lnrt;
						linear_regression(lnrt, ltable);
						out << lnrt.m << ",";
					}
					else if (tokens.at(4) == "B") {
						linreg_result_t lnrt;
						linear_regression(lnrt, ltable);
						out << lnrt.b << ",";
					}
				}
			}
			else if (tokens.at(2) == "CORR") {
				std::string addl_var = tokens.at(3);

				// make table
				int count = 0;
				ltable.clear();
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				std::vector<row_t> atable;
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == addl_var) {
						atable.push_back(table.at(i));
					}
				}

				out << correlation(ltable, atable) << ",";
			}
			else if (tokens.at(2) == "PCORR") {
				// rebuild table
				int count = 0;
				ltable.clear();
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				std::string addl_var = tokens.at(3);
				std::string addl2_var = tokens.at(4);

				std::vector<row_t> atable;
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == addl_var) {
						atable.push_back(table.at(i));
					}
				}

				std::vector<row_t> btable;
				for (int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == addl2_var) {
						btable.push_back(table.at(i));
					}
				}

				out << partial_correlate(ltable, atable, btable) << ",";
			}
		}
	}
	else if (tokens.at(0) == "MACRO") {
		if (tokens.at(1) == "LOAD") {
			if (debug) printf("MACRO:LOAD is unsupported\n");
		}
		else if (tokens.at(1) == "EXECUTE") {
			bool found = false;
			int idx = 0;

			for (; idx < macros.size(); idx++) {
				if (macros.at(idx).name == tokens.at(2)) {
					found = true;
					break;
				}
			}

			if (!found) {
				if (debug) printf("Macro %s does not exist", tokens.at(2).c_str());
			}
			else {
				for (int index = 0; index < macros.at(idx).tokens.size(); index++) {
					std::vector<std::string> tkns;
					tkns.clear();
					tkns = split(macros.at(idx).tokens.at(index), ':');
					if (debug) printf("Executing token %s\n", macros.at(idx).tokens.at(index).c_str());
					//execute_token(cfg, macros.at(idx).tokens.at(index), tkns, out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var);
					execute_token(cfg, macros.at(idx).tokens.at(index), tkns, out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
				}
			}
		}
		else if (tokens.at(1) == "LOOPVAR") {
			int index = 0;
			bool found = false;
			for (; index < lists.size(); index++) {
				if (lists.at(index).name == tokens.at(3)) {
					found = true;
					break;
				}
			}

			int mindex = 0;
			bool foundm = false;
			for (; mindex < macros.size(); mindex++) {
				if (macros.at(mindex).name == tokens.at(2)) {
					foundm = true;
					break;
				}
			}

			for (int i = 0; i < lists.at(index).items.size(); i++) {
				if (debug) printf("Executing list cycle %d / %d\n", i + 1, lists.at(index).items.size());
				macrovar = lists.at(index).items.at(i);
				//execute macro
				for (int j = 0; j < macros.at(mindex).tokens.size(); j++) {
					std::vector<std::string> tkns;
					tkns.clear();
					tkns = split(macros.at(mindex).tokens.at(j), ':');
					if (debug) printf("    Executing token %s\n", macros.at(mindex).tokens.at(j).c_str());
					execute_token(cfg, macros.at(mindex).tokens.at(j), tkns, out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
				}
			}
		}
		else if (tokens.at(1) == "LOADSTR") {
			if (debug) printf("MACRO:LOADSTR is unsupported\n");
		}
		else if (tokens.at(1) == "LOADM") {
			if (debug) printf("Recording macro . . . \n");
			std::string s;
			macro_t m;
			m.name = tokens.at(2);
			int size = 0;

			while (1) {
				cfg >> s;
				if (s == "EMACRO") {
					break;
				}
				else {
					m.tokens.push_back(s);
					size++;
				}
			}

			if (size > 0) {
				m.empty = false;
				m.valid = true;
			}

			macros.push_back(m);
			if (debug) printf("Loaded maro with %d items\n", size);
		}
		else {
			if (debug) printf("Bad macro keyword %s \n", tokens.at(1).c_str());
		}
	}
	else {
		if (debug) printf("did not recognize token [%s]\n", this_token.c_str());
	}

	return true;
}

void execute_script(std::string file, std::vector<row_t> table,
	std::vector<std::string> variables,
	std::vector<variable_t> rules,
	std::vector<custom_var_t> cvars) {

	// variables
	std::string this_token;
	std::string current_var = "__UNASSIGNED__";
	std::string agg_var = "__UNASSIGNED__";
	std::vector<row_t> ltable;
	std::string stmp;
	std::ifstream cfg;
	std::ofstream out;
	int state = 0;
	script_flag_t flags;

	// open the script
	cfg.open("out.cfg");
	if (!cfg.is_open()) {
		printf("Could not open file!\n");
		return;
	}

	// read config header
	cfg >> stmp >> this_token;
	if (stmp != "~" || this_token != "CONFIG") {
		printf("bad header\n");
		return;
	}

	// read mode
	int n_mode_opts = 0;
	cfg >> stmp >> this_token >> n_mode_opts;
	if (stmp != "#" || this_token != "MODE") {
		printf("failed to read mode info\n");
		return;
	}
	for (int i = 0; i < n_mode_opts; i++) {
		cfg >> this_token;
		if (this_token == "UNDERSCORE_IS_SPACE") {
			flags.f_underscore_is_space = true;
		}
		else if (this_token == "COLORS_OK") {
			flags.f_colors_ok = true;
		}
		else if (this_token == "AUTO_RESET_FILTERS") {
			flags.f_auto_reset_filters = true;
		}
		else if (this_token == "DEBUG_ON") {
			flags.f_debug = true;
		}
		else {
			printf("found unknown option [%s]\n", this_token.c_str());
		}
	}

	// read file settings
	cfg >> stmp >> this_token;
	if (stmp != "#" || this_token != "BEGIN") {
		printf("bad file setting\n");
		return;
	}
	cfg >> this_token;
	out.open(this_token.c_str(), std::ios::trunc);

	std::vector<period_t> periods;
	std::vector<macro_t> macros;
	std::vector<list_t> lists;

	std::string macrovar = "__UNDEFINED__";

	while (state >= 0) {
		std::vector<std::string> tokens;
		cfg >> this_token;

		tokens.clear();
		tokens = split(this_token, ':');

		execute_token(cfg, this_token, tokens, out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, flags.f_debug);
	}

	out.close();
	cfg.close();
}
