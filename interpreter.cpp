#include <vector>
#include <string>
#include "main.h"
#include "macros.h"
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

token_t imb_enumerate_token(interpreter_memblock_t &imb, std::string token) {
	std::vector<std::string> tokens = split(token, ':');
	std::string top = tokens.at(0);

	token_enum_t tet = EMPTY;

	if (top == "SETAGG") {
		tet = SETAGG;
	}
	else if (top == "SETPER") {
		tet = SETPER;
	}
	else if (top == "SETVAR") {
		tet = SETVAR;
	}
	else if (top == "SETMVR") {
		tet = SETMVR;
	}
	else if (top == "SETFILE") {
		tet = SETFILE;
	}
	else if (top == "MKDB") {
		tet = MKDB;
	}
	else if (top == "DELDB") {
		tet = DELDB;
	}
	else if (top == "COPYDB") {
		tet = COPYDB;
	}
	else if (top == "LOOPDB") {
		tet = __ERROR__;
	}
	else if (top == "WRITEDB") {
		tet = __ERROR__;
	}
	else if (top == "READDB") {
		tet = __ERROR__;
	}

	else if (top == "PUSH") {
		tet = PUSH;
	}
	else if (top == "POP") {
		tet = POP;
	}
	else if (top == "SWAP") {
		tet = SWAP;
	}
	else if (top == "SIFT") {
		tet = SIFT;
	}
	else if (top == "__STACKWIPE") {
		tet = __STACKWIPE;
	}

	else if (top == "MACRO") {
		tet = MACRO;
	}
	else if (top == "MACROF") {
		tet = MACROF;
	}
	else if (top == "MACROS") {
		tet = MACROS;
	}
	else if (top == "EXECUTE") {
		tet = EXECUTE;
	}
	else if (top == "MACROLOOP") {
		tet = MACROLOOP;
	}
	else if (top == "MACROTABLE") {
		tet = MACROTABLE;
	}

	else if (top == "FIRSTDATE") {
		tet = FIRSTDATE;
	}
	else if (top == "LASTDATE") {
		tet = LASTDATE;
	}
	else if (top == "MEAN" || top == "AVERAGE") {
		tet = MEAN;
	}
	else if (top == "MEDIAN") {
		tet = MEDIAN;
	}
	else if (top == "STDDEV" || top == "STDEV") {
		tet = STDDEV;
	}
	else if (top == "CV" || top == "COEFV" || top == "COEFVAR") {
		tet = COEFVAR;
	}

	else if (top == "MKP" || top == "MANNKENDALLP") {
		tet = MANNKENDALLP;
	}
	else if (top == "MKZ" || top == "MANNKENDALLZ") {
		tet = MANNKENDALLZ;
	}
	else if (top == "MKH" || top == "MANNKENDALLH") {
		tet = MANNKENDALLH;
	}
	else if (top == "MKT" || top == "MANNKENDALLT") {
		tet = MANNKENDALLT;
	}
	else if (top == "MKTAU" || top == "MANNKENDALLTAU") {
		tet = MANNKENDALLTAU;
	}

	else if (top == "TSS" || top == "THEILSENSLOPE") {
		tet = THEILSENSLOPE;
	}

	else if (top == "LINEARREGRESSIONA" || top == "LINRA") {
		tet = LINRA;
	}
	else if (top == "LINEARREGRESSIONB" || top == "LINRB") {
		tet = LINRB;
	}
	else if (top == "LINEARREGRESSIONR" || top == "LINRR") {
		tet = LINRR;
	}
	else if (top == "POLYREG") {
		tet = POLYREG;
	}
	else if (top == "EXPREG") {
		tet = EXPREG;
	}

	else if (top == "MIN") {
		tet = MIN;
	}
	else if (top == "MAX") {
		tet = MAX;
	}
	else if (top == "RANGE") {
		tet = RANGE;
	}
	else if (top == "IQR") {
		tet = IQR;
	}
	else if (top == "PERCENTILE") {
		tet = PERCENTILE;
	}
	else if (top == "RPERCENTILE") {
		tet = RPERCENTILE;
	}

	else if (top == "CORR") {
		tet = CORR;
	}
	else if (top == "PCORR") {
		tet = PCORR;
	}

	else if (top == "LINT") {
		tet = LINT;
	}
	else if (top == "POLYINT") {
		tet = POLYINT;
	}
	else if (top == "NEARINT") {
		tet = NEARINT;
	}

	else if (top == "OCLIP") {
		tet = OCLIP;
	}
	else if (top == "NORMALIZE") {
		tet = NORMALIZE;
	}

	else if (top == "__LEGACY_INT") {
		tet = __LEGACY_INT;
	}
	else if (top == "__LEGACY_EXECUTE_TOKEN") {
		tet = __LEGACY_EXECUTE_TOKEN;
	}
	else if (top == "__LEGACY_EXECUTE_SCRIPT") {
		tet = __LEGACY_EXECUTE_SCRIPT;
	}
	else if (top == "__PROTECTED_DIRECT_EXECUTE") {
		tet = __ERROR__;
	}
	else if (top == "__PROTECTED_MODIFY_TOKEN") {
		tet = __ERROR__;
	}
	else if (top == "__PROTECTED_DELETE_DIR") {
		tet = __ERROR__;
	}
	else if (top == "__PROTECTED_REBUILD_TOKENLIST") {
		tet = __ERROR__;
	}
	else {
		printf("Error while enumerating token [%s]: Token not recognized\n", top.c_str());
	}

	if (tet == __ERROR__) printf("Error while enumerating token [%s]: Forward-defined tokens are not useable\n", top.c_str());

	token_t t;
	t.token = tet;
	t.associated = tokens;

	return t;
}

bool imb_enumerate_file(interpreter_memblock_t &imb, std::ifstream &i, std::vector<token_t> &tokens) {
	if (!i.is_open()) {
		printf("Failed to open file for enumeration\n");
		return false;
	}

	while (1) {
		std::string s;
		if (!i.eof()) {
			i >> s;
			tokens.push_back(imb_enumerate_token(imb, s));
		}
		else {
			break;
		}
	}

	return true;
}

bool imb_execute_token(interpreter_memblock_t &imb, token_t t) {
	switch (t.token) {
	case EMPTY:
		printf("Skipped empty token\n"); break;
	case __LEGACY_INT:
		try {
			int id = std::stoi(t.associated.at(1));
			switch (id) {
			case 0:
				execute_main_analysis_correlate(imb.table, imb.variables, std::vector<custom_var_t>());
				break;
			default:
				break;
			}
		}
		catch (std::exception &e) {
			printf("Failed to execute internal legacy macro\n    [%s]", e.what());
		}
		break;
	default:
		printf("Behavior for token %d is undefined\n", t.token);
		break;
	}

	return true;
}

bool imb_save_enumerated_file(interpreter_memblock_t &imb, std::ofstream &o) {
	/*
		TODO:
		- write basic header
		- write DB save
		- write tokens with links to associated 
		- write associated
		- write footer
	*/
}

bool imb_open_enumerated_file(interpreter_memblock_t &imb, std::ifstream &i, std::vector<token_t> &toks) {
	/*
		TODO:
		- read in all data in loop until hit footer
		- validate via header
	*/
}

bool imb_execute_script(interpreter_memblock_t &imb, std::vector<token_t> toks) {
	for (token_t t : toks)
		imb_execute_token(imb, t);

	return true;
}

bool imb_execute_script(interpreter_memblock_t &imb, std::ifstream &i) {
	std::string s;
	std::vector<token_t> toks;

	if (!i.is_open()) {
		printf("Internal error while attempting to execute script\n");
		return false;
	}

	i >> s;
	if (s == "~DBSyft2") {
		// TODO Handle header and pre-script here
		imb_enumerate_file(imb, i, toks);
	}
	else if (s == "DBSYFTENUMSRP") {
		// TODO Handle header and pre-script here
		imb_open_enumerated_file(imb, i, toks);
	}
	else {
		printf("Unable to read script for executing\n    Perhaps it is a legacy script?\n");
		return false;
	}

	imb_execute_script(imb, toks);

	return true;
}