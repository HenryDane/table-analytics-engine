#include <vector>
#include "main.h"

// roll-it urself functions
quantile_t scimath_quantile(std::vector<row_t> &t, double lbound, double ubound);
quantile_t scimath_quantile(double * t, int n, double lbound, double ubound);

// numpy functions
int np_sign(double x);
void np_unique(std::vector<row_t> &tbl_in, std::vector<row_t> &tbl_out);

// math functions
double norm_cdf(double x);
double rational_approximation(double t);
double norm_ppf(double p);

//scipy functions
std::vector<row_t> scipy_median_filter(std::vector<row_t> &t, int window = 7);