#pragma once

#include <vector>
#include "main.h"

void theil_sen(ts_result_t &result, std::vector<row_t> table, int units_as = 0, double threshold = 0.05);