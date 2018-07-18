#include <vector>
#include "main.h"

// numpy functions
int np_sign(double x);
void np_unique(std::vector<row_t> &tbl_in, std::vector<row_t> &tbl_out);

// math functions
double norm_cdf(double x);
double rational_approximation(double t);
double norm_ppf(double p);