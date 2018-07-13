#pragma once

#include <vector>
#include "main.h"

void theil_sen(ts_result_t &result, std::vector<row_t> &table, int units_as = 1, double threshold = 0.05);

// math functions
double date_as_year(date_t &date);
double date_as_month(date_t &date);
int date_as_day(date_t &date);