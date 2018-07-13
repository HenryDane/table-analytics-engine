#pragma once

#include <vector>
#include "main.h"

void mann_kendall_test(mk_result_t &mkrt, std::vector<row_t> &table, double alpha = 0.05, double threshold = 0.1);

// numpy functions
int np_sign(double x);
void np_unique(std::vector<row_t> &tbl_in, std::vector<row_t> &tbl_out);

// math functions
double norm_cdf(double x);
double rational_approximation(double t);
double norm_ppf(double p);