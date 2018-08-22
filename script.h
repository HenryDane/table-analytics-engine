#pragma once
#include <vector>
#include <string>
#include "main.h"

void execute_script(std::string file, std::vector<row_t> table,
	std::vector<std::string> variables,
	std::vector<variable_t> rules,
	std::vector<custom_var_t> cvars);

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
	bool debug = false);

void do_console(std::vector<row_t> table, std::vector<custom_var_t> cvars, std::vector<std::string> variables, std::vector<variable_t> rules);

bool eval_condition(std::string condition_token, std::vector<row_t> table);

void yearly_agg(std::vector<row_t> &tbl);

list_t get_list_by_name(std::vector<list_t> l, std::string n);
