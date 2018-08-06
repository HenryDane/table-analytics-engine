#pragma once

#include <vector>
#include "main.h"

void mann_kendall_test(mk_result_t &mkrt, std::vector<row_t> table, double alpha = 0.05, double threshold = 0.1);