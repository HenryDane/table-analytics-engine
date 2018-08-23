#include <vector>
#include <string>
#include "main.h"
#include "util.h"

bool imb_init(interpreter_memblock_t &imb) {
	period_t p = { "ALL", NONE, { 0, 0, 0, 0, 0 }, { 9999, 99, 99, 99, 99 } };
	imb.active_period = p;
	int state = 0;

	imb.agg = "__UNDEFINED__";
	imb.var = "__UNDEFINED__";
	imb.file = "__UNDEFINED__";
	imb.mvr = "__UNDEFINED__";

	imb.cfg.close();
	imb.out.close();

	imb.atable.clear();
	imb.ltable.clear();
	imb.table.clear();

	imb.macros.clear();
	imb.lists.clear();
	imb.variables.clear();

	return false;
}

bool imb_update_var_table(interpreter_memblock_t &imb) {
	imb.variables.clear();

	if (imb.table.size() < 0) return false;

	for (unsigned int i = 0; i < imb.table.size(); i++) {
		printf("Reading row %d \r", i);

		bool found = false;
		if (i > imb.table.size()) i = 0;

		for (unsigned int j = 0; j < imb.variables.size(); j++) {
			if (imb.variables.at(j) == imb.table.at(i).variable) {
				found = true;
				break;
			}
		}
		if (!found) {
			imb.variables.push_back(imb.table.at(i).variable);
		}
	}

	return (imb.table.size() > 0);
}

bool imb_set_var(interpreter_memblock_t &imb, std::string var) {
	imb.ltable.clear();
	for (row_t r : imb.atable)
		if (r.variable == imb.var)
			imb.ltable.push_back(r);

	return (imb.variables.size() > 0);
}

bool imb_build_table(interpreter_memblock_t &imb,
	std::vector<row_t> table,
	std::vector<custom_var_t> cvars) {

	imb.table.clear();

	std::copy(table.begin(), table.end(), imb.table.begin());

	imb_update_var_table(imb);

	for (custom_var_t cvr : cvars) {
		std::vector<row_t> ctable;
		filter_by_variable(imb.table, ctable, imb.variables, cvr.name, true, true);
		imb.table.insert(imb.table.begin(), ctable.begin(), ctable.end());
	}
	
	return true;
}

bool imb_build_agg_table(interpreter_memblock_t &imb,
	agg_t agg) {

	// always remake
	// if (agg == imb.active_aggregation) return true;

	imb.atable.clear();
	imb.ltable.clear();

	for (row_t r : imb.table) {
		if (r.id < 0) r.id = r.id * -1;
		if (r.edits <= 0) r.edits = 1;
		if (date_in_period(r.date, imb.active_period))
			imb.atable.push_back(r);
	}

	for (row_t r : imb.table) {
		if (r.id < 0) continue;

		for (row_t s : imb.table) {
			if (s.id < 0) continue;

			bool ok = false;
			switch (imb.active_aggregation) {
			case HOURLY:
				if (r.date.year == s.date.year &&
					r.date.month == s.date.month &&
					r.date.day == s.date.day &&
					r.date.hours == s.date.hours)
					ok = true;
				break;
			case DAILY:
				if (r.date.year == s.date.year &&
					r.date.month == s.date.month &&
					r.date.day == s.date.day) 
					ok = true;
				break;
			case MONTHLY:
				if (r.date.year == s.date.year &&
					r.date.month == s.date.month) 
					ok = true;
				break;
			case YEARLY:
				if (r.date.year == s.date.year)
					ok = true;
				break;
			}

			if (ok) {
				r.value = safe_add(r.value, s.value);
				r.edits++;
				s.id = -100;
			}
		}
	}

	// remake vars
	imb_set_var(imb, imb.var);

	return (imb.variables.size() > 0 && imb.atable.size() > 0);
}

bool imb_set_period(interpreter_memblock_t &imb, period_t p) {
	if (imb.active_period.start == p.start &&
		imb.active_period.end == p.end) 
		return true;

	imb.active_period = p;

	return imb_build_agg_table(imb, imb.active_aggregation);
}

bool imb_enumerate_token(interpreter_memblock_t &imb, std::string token) {
	std::vector<std::string> tokens = split(token, ':');
	std::string top = tokens.at(0);
	
	if (top == "SETAGG") {

	}
	else if (top == "SETPER") {
		
	}
	else if (top == "SETVAR") {

	}
	else if (top == "SETMVR") {

	}
	else if (top == "SETFILE") {

	}
	else if (top == "MKDB") {

	}
	else if (top == "DELDB") {

	}
	else if (top == "COPYDB") {

	}
}

bool imb_enumerate_file(interpreter_memblock_t &imb, std::ifstream &i) {

}

bool imb_execute_token(interpreter_memblock_t &imb, token_t t) {

}