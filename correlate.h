#pragma once

#include <vector>
#include "main.h"

void running_avg(std::vector<row_t> &table, std::vector<double> mov_avg, int windowsize);
double mean(std::vector<row_t> &table);
double std_dev(std::vector<row_t> &table);
double coef_var(std::vector<row_t> &table);
double correlation(std::vector<row_t> &tablea, std::vector<row_t> &tableb, bool d = false);