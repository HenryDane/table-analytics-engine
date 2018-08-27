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
#include "macros.h"

void yearly_agg(std::vector<row_t> &tbl) {
	std::vector<row_t> table_agg;
	for (unsigned int i = 0; i < tbl.size(); i++) {
		row_t r = tbl.at(i);

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
			tbl.erase(tbl.begin() + i);
			i = std::max((unsigned int) 0, i - 5);
		}
		else {
			table_agg.push_back(r);
			tbl.erase(tbl.begin() + i);
			i = std::max((unsigned int) 0, i - 5);
		}
	}

	tbl = table_agg;
}

bool eval_condition(std::string condition_token, std::vector<row_t> table) {
	std::vector<std::string> pieces = split(condition_token, ';');

	s_double_t val;
	val.v = 0;
	val.f = false;
	s_double_t otherval;
	if (pieces.at(4) != "__NAN__") {
		//double a = std::stod(pieces.at(4)));
		otherval.v = std::stod(pieces.at(4));
		otherval.f = false;
	}
	else {
		otherval.v = 0;
		otherval.f = true;
	}

	if (pieces.at(2) == "DT") {
		std::vector<std::string> datepieces = split(pieces.at(3), '-');

		int yyyy = std::stoi(datepieces.at(0));
		int mm = std::stoi(datepieces.at(1));
		int dd = std::stoi(datepieces.at(2));
		int hh = std::stoi(datepieces.at(3));
		int min = std::stoi(datepieces.at(4));

		date_t d = { yyyy, mm, dd, hh, min };

		for (unsigned int i = 0; i < table.size(); i++) {
			if (table.at(i).date.numeric() == d.numeric()) {
				val = table.at(i).value;
				break;
			}
		}
	}
	else if (pieces.at(2) == "ID") {
		int id = std::stoi(pieces.at(3));

		for (unsigned int i = 0; i < table.size(); i++) {
			if (table.at(i).id == id) {
				val = table.at(i).value;
			}
		}
	}

	if (pieces.at(0) == "EQ") {
		if (val.f == otherval.f) {
			return true;
		}
		else {
			if (val.v == otherval.v) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else if (pieces.at(0) == "LS") {
		if (val.f || otherval.f) {
			printf("EVAL_CONDITION lookupval or literal is null!\n");
			return false;
		}
		else {
			if (val.v < otherval.v) {
				return true;
			}
			else {
				printf("%f, %f\n", val.v, otherval.v);
				return false;
			}
		}
	}
	else if (pieces.at(0) == "GR") {
		if (val.f || otherval.f) {
			printf("EVAL_CONDITION lookupval or literal is null!\n");
			return false;
		}
		else {
			if (val.v > otherval.v) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else if (pieces.at(0) == "NT") {
		if (val.f != otherval.f) {
			return true;
		}
		else {
			if (val.v != otherval.v) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else if (pieces.at(0) == "TRUE") {
		return true;
	}
	else if (pieces.at(0) == "FALSE") {
		return false;
	}

	printf("something is wrong while executing condition evaluation!");
	return false;
}

list_t get_list_by_name(std::vector<list_t> l, std::string n) {
	for (unsigned int i = 0; i < l.size(); i++) {
		if (l.at(i).name == n) {
			return l.at(i);
			break;
		}
	}

	// return garbage
	return{ "__BAD_LIST__", (std::vector<std::string>) NULL };
}

bool execute_token(std::istream &cfg,
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

	static std::vector<std::string> __file__stack;
	static std::vector<std::string> __agg__stack;
	static std::vector<std::string> __var__stack;
	static std::vector<std::string> __mvr__stack;

	static std::string __file__name = "__unset_static__";

	if (debug) {
		printf("\n\n");
		printf("+===========================================+\n");
		printf(" ltable: %d tokens: %d\n", ltable.size(), tokens.size());
		printf(" current_var: %s \n macrovar: %s\n [%s]\n", current_var.c_str(), macrovar.c_str(), this_token.c_str());
		printf(" vs: %d ms:%d\n", __var__stack.size(), __mvr__stack.size());
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
			for (unsigned int i = 0; i < lists.size(); i++) {
				if (lists.at(i).name == tokens.at(2)) {
					for (unsigned int j = 0; j < lists.at(i).items.size(); j++) {
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
	else if (tokens.at(0) == "TEXTIF") {
		if (eval_condition(tokens.at(2), table)) {
			out << tokens.at(3) << ",";
		}
	}
	else if (tokens.at(0) == "CONCAT") {
		if (tokens.at(1) == "V") {
			out << tokens.at(2) << " " << tokens.at(3) << ",";
		}
		else if (tokens.at(1) == "M") {
			out << tokens.at(2) << " ";
			execute_token(cfg, tokens.at(3), split(tokens.at(3), '+'), out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
		}
	}
	else if (tokens.at(0) == "~" || tokens.at(0) == "LOGIC") {
		if (tokens.at(1) == "IF") {
			if (eval_condition(tokens.at(2), table)) {
				if (tokens.at(3) == "J") {
					//goto_label(tokens.at(4), cfg);
				}
				else if (tokens.at(3) == "T") {
					out << tokens.at(4) << ",";
				}
				else if (tokens.at(3) == "E") {
					// execute macro
					for (unsigned int i = 0; i < macros.size(); i++) {
						if (macros.at(i).name == tokens.at(4)) {
							for (unsigned int j = 0; j < macros.at(i).tokens.size(); j++) {
								execute_token(cfg, macros.at(i).tokens.at(j), split(macros.at(i).tokens.at(j), ':'), out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
							}
						}
						break;
					}
				}
			}
		}
		else if (tokens.at(1) == "DBLOOP") {
			if (tokens.at(2) == "__GLOBAL__") {

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
					for (unsigned int i = 0; i < table.size(); i++) {
						if (table.at(i).variable == current_var) {
							ltable.push_back(table.at(i));
							count++;
						}
					}

					if (ltable.size() == 0) {
						if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
						unsigned int idx = 0;
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
				else {
					current_var = tokens.at(3);

					// make table
					int count = 0;
					ltable.clear();
					for (unsigned int i = 0; i < table.size(); i++) {
						if (table.at(i).variable == current_var) {
							ltable.push_back(table.at(i));
							count++;
						}
					}

					if (ltable.size() == 0) {
						if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
						unsigned int idx = 0;
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
			else if (tokens.at(2) == "__FILE__") {
				out.close();

				out.open(tokens.at(3));
				if (!out.is_open()) {
					printf("UNABLE TO REOPEN FILE [%s]! OUTPUT IS IN UNDEFINED STATE!\n", tokens.at(3).c_str());
				}
			}
			else {
				if (debug) printf("Err !");
			}
		}
		else if (tokens.at(1) == "VAR") {
			if (tokens.at(2) == "PUSH") {
				if (tokens.at(3) == "__MVR__") {
					__mvr__stack.push_back(macrovar);
				}
				else if (tokens.at(3) == "__VAR__") {
					__var__stack.push_back(current_var);
					//printf("NEW SIZE: %d\n", __var__stack.size());
				}
				else if (tokens.at(3) == "__AGG__") {
					__agg__stack.push_back(agg_var);
				}
				else if (tokens.at(3) == "__FILE__") {
					__file__stack.push_back(__file__name);
				}
			}
			else if (tokens.at(2) == "POP") {
				if (tokens.at(3) == "__MVR__") {
					macrovar = __mvr__stack.at(__mvr__stack.size() - 1);
					__mvr__stack.pop_back();
				}
				else if (tokens.at(3) == "__VAR__") {
					current_var = __var__stack.at(__var__stack.size() - 1);
					__var__stack.pop_back();
				}
				else if (tokens.at(3) == "__AGG__") {
					agg_var = __agg__stack.at(__agg__stack.size() - 1);
					__agg__stack.pop_back();
				}
				else if (tokens.at(3) == "__FILE__") {
					__file__name = __file__stack.at(__file__stack.size() - 1);
					__file__stack.pop_back();
				}
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
				agg_var = "__UNDEFINED__";
				__file__name = "__UNDEFINED__";
				__mvr__stack.clear();
				__agg__stack.clear();
				__var__stack.clear();
				tokens.clear();
			}
			else if (tokens.at(2) == "CRASH") {
				abort();
			}
			else if (tokens.at(2) == "LOG") {
				std::cout << tokens.at(3) << std::endl;
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
			}
			else if (tokens.at(2) == "LIST") {
				if (tokens.at(3) == "CREATE") {
					list_t l;
					l.name = tokens.at(4);
					lists.push_back(l);
					if (debug) printf("list size: %d\n", lists.size());
				}
				else if (tokens.at(3) == "ADD") {
					unsigned int i = 0;
					for (; i < lists.size(); i++) {
						if (lists.at(i).name == tokens.at(4)) {
							lists.at(i).items.push_back(tokens.at(5));
						}
					}
				}
				else if (tokens.at(3) == "FCVAR") {
					list_t l;
					l.name = tokens.at(4);

					unsigned int i = 0;
					bool found = false;
					for (; i < cvars.size(); i++) {
						if (cvars.at(i).name == tokens.at(5)) {
							found = true;
							break;
						}
					}

					std::copy(cvars.at(i).pieces.begin(), cvars.at(i).pieces.end(), std::back_inserter(l.items));
				}
				else if (tokens.at(3) == "FGVAR") {
					list_t l;
					l.name = tokens.at(4);

					for (unsigned int i = 0; i < table.size(); i++) {
						bool f = false;

						for (unsigned int j = 0; j < l.items.size(); j++) {
							if (l.items.at(j) == table.at(i).variable) {
								f = true;
							}
						}

						if (!f)
							l.items.push_back(table.at(i).variable);
					}

					printf("Ns: %d\n", l.items.size());

					lists.push_back(l);

					//std::copy(variables.begin(), variables.end(), std::back_inserter(l.items));
				}
				else if (tokens.at(3) == "MERGE") {
					unsigned int i = 0;
					unsigned int j = 0;
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
					unsigned int idx = 0;
					bool found = false;
					for (; idx < lists.size(); idx++) {
						if (lists.at(idx).name == tokens.at(4)) {
							found = true;
							break;
						}
					}

					std::vector<std::string> variables;
					for (unsigned int i = 0; i < table.size(); i++) {
						bool f = false;

						for (unsigned int j = 0; j < variables.size(); j++) {
							if (variables.at(j) == table.at(i).variable) {
								f = true;
								break;
							}
						}

						if (!f) {
							variables.push_back(table.at(i).variable);
						}
					}

					for (unsigned int m = 0; m < variables.size(); m++) {
						lists.at(idx).items.push_back(variables.at(m));
					}

					if (debug) printf("Scanned %d items into list %s at %d\n", lists.at(idx).items.size(), lists.at(idx).name.c_str(), idx);
				}
				else if (tokens.at(3) == "SCAN") {
					unsigned int i = 0;
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
								//if (debug) printf("%d\n", i);
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
					for (unsigned int i = 0; i < table.size(); i++) {
						if (table.at(i).variable == current_var) {
							ltable.push_back(table.at(i));
							//count++;
						}
					}
				}

				unsigned int idx = 0;
				bool found = false;
				for (; idx < periods.size(); idx++) {
					if (periods.at(idx).name == tokens.at(3)) {
						found = true;
						break;
					}
				}

				if (found) {
					if (debug) printf("S:F:P found %s at %d\n", tokens.at(3).c_str(), idx);
					for (unsigned int i = 0; i < ltable.size(); i++) {
						if (date_as_day(ltable.at(i).date) > date_as_day(periods.at(idx).end) ||
							date_as_day(ltable.at(i).date) < date_as_day(periods.at(idx).start)) {
							ltable.at(i).id = -100;
						}
					}

					for (unsigned int i = 0; i < ltable.size(); i++) {
						if (ltable.at(i).id == -100) {
							ltable.erase(ltable.begin() + i);
							i = std::max((unsigned int) 0, i - 2);
						}
					}
				}
				if (debug) printf("SYS:FILTER:PERIOD filtered by period [%s] successfully (%d)\n", tokens.at(3).c_str(), ltable.size());
			}
			else if (tokens.at(2) == "RESET") {
				// make table
				int count = 0;
				ltable.clear();
				for (unsigned int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				if (ltable.size() == 0) {
					if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
					unsigned int idx = 0;
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
				for (unsigned int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				if (ltable.size() == 0) {
					if (debug) printf("S:O:V returned 0 records, looking in custom vars \n");
					unsigned int idx = 0;
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
						for (unsigned int i = 0; i < table.size(); i++) {
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

				if (addl_var == "__MVR__") {
					addl_var = macrovar;
				}
				else if (addl_var == "__VAR__") {
					addl_var = current_var;
				}

				// make table
				int count = 0;
				ltable.clear();
				for (unsigned int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				if (ltable.size() == 0) {
					if (debug) printf("scanning cvars . . .\n");
					for (unsigned int i = 0; i < cvars.size(); i++) {
						if (cvars.at(i).name == current_var) {
							filter_by_variable(table, ltable, cvars.at(i).pieces, " ", false);
							break;
						}
					}
				}

				std::vector<row_t> atable;
				for (unsigned int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == addl_var) {
						atable.push_back(table.at(i));
					}
				}

				if (atable.size() == 0) {
					if (debug) printf("scanning cvars . . . .\n");
					for (unsigned int i = 0; i < cvars.size(); i++) {
						if (cvars.at(i).name == current_var) {
							filter_by_variable(table, atable, cvars.at(i).pieces, " ", false);
							break;
						}
					}
				}

				if (debug) printf("at corr. %d %d\n", ltable.size(), atable.size());
				out << correlation(ltable, atable, false) << ",";
				if (debug) printf("done corr \n");
			}
			else if (tokens.at(2) == "PCORR") {
				// rebuild table
				int count = 0;
				ltable.clear();
				for (unsigned int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == current_var) {
						ltable.push_back(table.at(i));
						count++;
					}
				}

				std::string addl_var = tokens.at(3);
				std::string addl2_var = tokens.at(4);

				std::vector<row_t> atable;
				for (unsigned int i = 0; i < table.size(); i++) {
					if (table.at(i).variable == addl_var) {
						atable.push_back(table.at(i));
					}
				}

				std::vector<row_t> btable;
				for (unsigned int i = 0; i < table.size(); i++) {
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
			if (debug) printf("MACRO:LOAD is untested\n");

			macro_t m;

			m.name = tokens.at(2);

			std::ifstream in;
			in.open(tokens.at(3));
			if (!in.is_open()) {
				printf("failed to open file %s\n", tokens.at(3).c_str());
			}
			else {
				std::string s;
				in >> s;
				if (s != "EMACRO") {
					m.tokens.push_back(s);
				}

				macros.push_back(m);
				printf("Successfully loaded macro %s with %d tokens\n", tokens.at(2).c_str(), m.tokens.size());
			}

			in.close();
		}
		else if (tokens.at(1) == "TABLE") {
			list_t l1 = get_list_by_name(lists, tokens.at(2)); // col list
			list_t l2 = get_list_by_name(lists, tokens.at(3)); // row list

			printf("List1 :%d\nList2: %d\n", l1.items.size(), l2.items.size());

			macro_t m;
			bool found = false;

			for (unsigned int i = 0; i < macros.size(); i++) {
				if (macros.at(i).name == tokens.at(4)) {
					m = macros.at(i);
					found = true;
					break;
				}
			}
			if (!found) {
				printf("Bad macro id\n");
			}

			if (l1.name == "__BAD_LIST__" || l2.name == "__BAD_LIST__") {
				printf("Bad list argument(s)\n");
			}
			else {
				__var__stack.push_back(current_var);
				__mvr__stack.push_back(macrovar);

				for (unsigned int i = 0; i < l1.items.size(); i++) {
					for (unsigned int j = 0; j < l2.items.size(); j++) {
						current_var = l2.items.at(j);
						macrovar = l1.items.at(i);

						// execute macro
						for (unsigned int k = 0; k < m.tokens.size(); k++) {
							execute_token(cfg, m.tokens.at(k), split(m.tokens.at(k), ':'), out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
						}
					}
					out << std::endl;
				}

				current_var = __mvr__stack.at(__var__stack.size() - 1);
				__var__stack.pop_back();
				macrovar = __mvr__stack.at(__mvr__stack.size() - 1);
				__mvr__stack.pop_back();
			}
		}
		else if (tokens.at(1) == "TABLEL") {
			list_t l1 = get_list_by_name(lists, tokens.at(2)); // col list
			list_t l2 = get_list_by_name(lists, tokens.at(3)); // row list

			printf("List1 :%d\nList2: %d\n", l1.items.size(), l2.items.size());

			macro_t m;
			bool found = false;

			for (unsigned int i = 0; i < macros.size(); i++) {
				if (macros.at(i).name == tokens.at(4)) {
					m = macros.at(i);
					found = true;
					break;
				}
			}
			if (!found) {
				printf("Bad macro id\n");
			}

			if (l1.name == "__BAD_LIST__" || l2.name == "__BAD_LIST__") {
				printf("Bad list argument(s)\n");
			}
			else {
				__var__stack.push_back(current_var);
				__mvr__stack.push_back(macrovar);

				out << ",";
				for (unsigned int j = 0; j < l2.items.size(); j++) {
					out << l2.items.at(j) << ",";
				}
				out << std::endl;

				for (unsigned int i = 0; i < l1.items.size(); i++) {
					out << l1.items.at(i) << ",";
					for (unsigned int j = 0; j < l2.items.size(); j++) {
						current_var = l2.items.at(j);
						macrovar = l1.items.at(i);

						// execute macro
						for (unsigned int k = 0; k < m.tokens.size(); k++) {
							execute_token(cfg, m.tokens.at(k), split(m.tokens.at(k), ':'), out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
						}
					}
					out << std::endl;
				}

				current_var = __mvr__stack.at(__var__stack.size() - 1);
				__var__stack.pop_back();
				macrovar = __mvr__stack.at(__mvr__stack.size() - 1);
				__mvr__stack.pop_back();
			}
		}
		else if (tokens.at(1) == "EXECUTE") {
			bool found = false;
			unsigned int idx = 0;

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
				for (unsigned int index = 0; index < macros.at(idx).tokens.size(); index++) {
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
			unsigned int index = 0;
			bool found = false;
			for (; index < lists.size(); index++) {
				if (lists.at(index).name == tokens.at(3)) {
					found = true;
					break;
				}
			}

			unsigned int mindex = 0;
			bool foundm = false;
			for (; mindex < macros.size(); mindex++) {
				if (macros.at(mindex).name == tokens.at(2)) {
					foundm = true;
					break;
				}
			}

			for (unsigned int i = 0; i < lists.at(index).items.size(); i++) {
				if (debug) printf("Executing list cycle %d / %d\n", i + 1, lists.at(index).items.size());
				macrovar = lists.at(index).items.at(i);
				//execute macro
				for (unsigned int j = 0; j < macros.at(mindex).tokens.size(); j++) {
					std::vector<std::string> tkns;
					tkns.clear();
					tkns = split(macros.at(mindex).tokens.at(j), ':');
					if (debug) printf("    Executing token %s\n", macros.at(mindex).tokens.at(j).c_str());
					execute_token(cfg, macros.at(mindex).tokens.at(j), tkns, out, state, current_var, ltable, table, macros, cvars, periods, flags, macrovar, lists, agg_var, debug);
				}
			}
		}
		else if (tokens.at(1) == "LOADSTR") {
			if (debug) printf("MACRO:LOADSTR is untested\n");

			macro_t m;

			m.name = tokens.at(2);

			for (unsigned int i = 0; i < std::stod(tokens.at(3)); i++) {
				std::string s;
				cfg >> s;
				m.tokens.push_back(s);
			}

			macros.push_back(m);
			printf("Successfully loaded %s with %d tokens\n", tokens.at(2).c_str(), m.tokens.size());
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
	else if (tokens.at(0) == "INT0") {
		execute_main_full_analysis(table, id_variables(table), periods, cvars);
	}
	else if (tokens.at(0) == "INT1") {
		execute_main_analysis_correlate(table, id_variables(table), cvars);
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
	cfg.open(file);
	if (!cfg.is_open()) {
		printf("Could not open file [%s] !\n", file.c_str());
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
		else if (this_token == "DEBUG_OFF") {
			flags.f_debug = false;
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

void do_console(std::vector<row_t> table, std::vector<custom_var_t> cvars, std::vector<std::string> variables, std::vector<variable_t> rules) {

	std::string str;

	std::string var;
	std::string mvr;
	std::string avar;

	int state = 0;

	script_flag_t flags;

	//std::vector<row_t> table;
	std::vector<row_t> ltable;
	std::vector<macro_t> macros;
	std::vector<list_t> lists;
	std::vector<period_t> periods;
	//std::vector<custom_var_t> cvars;

	std::ofstream out;
	out.open("consoleresults.txt", std::ios::app);

	std::cout << "Kevinscript Console v1" << std::endl;
	while (1) {
		std::cout << ">>> ";
		std::cin >> str;

		if (str == "EXIT" || str == "e") break;

		//execute_token( std::cin, str, split(str, ':'), out, state, var, ltable, table, macros, cvars, periods, flags, mvr, lists, avar, true);

		if (startsWith(str, "EXECUTE:")) {
			execute_script(split(str, ':').at(1).c_str(), table, variables, rules, cvars);
		}
	}
}