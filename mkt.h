#pragma once

#include <vector>
#include "main.h"

void mann_kendall_test(mk_result_t &mkrt, std::vector<row_t> table, double alpha = 0.05, double threshold = 0.1);

double mann_kendall_tau(std::vector<row_t> table);
double mann_kendall_tau(std::vector<row_t> tablea, std::vector<row_t> tableb);