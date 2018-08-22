#pragma once
#include <vector>
#include <string>
#include "util.h"

int execute_main_analysis(std::vector<row_t> table, std::vector<std::string> variables, std::vector<custom_var_t> cvars);
int execute_main_full_analysis(std::vector<row_t> table, std::vector<std::string> variables, std::vector<period_t> periods, std::vector<custom_var_t> cvars);
void execute_main_analysis_correlate(std::vector<row_t> table, std::vector<std::string> variables, std::vector<custom_var_t> cvars);