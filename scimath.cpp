#include <vector>
#include <sstream>
#include "main.h"

int np_sign(double x) {
	return (x < 0) ? -1 : ((x == 0) ? 0 : 1);
}

void np_unique(std::vector<row_t> &tbl_in, std::vector<row_t> &tbl_out) {
	for (unsigned int i = 0; i < tbl_in.size(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < tbl_out.size(); j++) {
			if (tbl_in.at(i).value.v == tbl_out.at(j).value.v) {
				found = true;
			}
		}

		if (!found) {
			tbl_out.push_back(tbl_in.at(i));
		}
	}
}

/*
Hacks from the web(tm): https://www.johndcook.com/blog/cpp_phi/
*/
double norm_cdf(double x) {
	// constants
	double a1 = 0.254829592;
	double a2 = -0.284496736;
	double a3 = 1.421413741;
	double a4 = -1.453152027;
	double a5 = 1.061405429;
	double p = 0.3275911;

	// Save the sign of x
	int sign = 1;
	if (x < 0)
		sign = -1;
	x = fabs(x) / sqrt(2.0);

	// A&S formula 7.1.26
	double t = 1.0 / (1.0 + p*x);
	double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

	return 0.5*(1.0 + sign*y);
}

double rational_approximation(double t) {
	// Abramowitz and Stegun formula 26.2.23.
	// The absolute value of the error should be less than 4.5 e-4.
	double c[] = { 2.515517, 0.802853, 0.010328 };
	double d[] = { 1.432788, 0.189269, 0.001308 };
	return t - ((c[2] * t + c[1])*t + c[0]) /
		(((d[2] * t + d[1])*t + d[0])*t + 1.0);
}

/*
Hacks from the web(tm): https://www.johndcook.com/blog/cpp_phi_inverse/
*/
double norm_ppf(double p) {
	if (p <= 0.0 || p >= 1.0) {
		std::stringstream os;
		os << "Invalid input argument (" << p
			<< "); must be larger than 0 but less than 1.";
		throw std::invalid_argument(os.str());
	}

	// See article above for explanation of this section.
	if (p < 0.5) {
		// F^-1(p) = - G^-1(p)
		return -rational_approximation(sqrt(-2.0*log(p)));
	}
	else {
		// F^-1(p) = G^-1(1-p)
		return rational_approximation(sqrt(-2.0*log(1 - p)));
	}
}
