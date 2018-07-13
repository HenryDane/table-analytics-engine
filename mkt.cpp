#include <vector>
#include <sstream>
#include <cmath>
#include "main.h"
#include "mkt.h"

void mann_kendall_test(mk_result_t &mkrt, std::vector<row_t> &table, double alpha, double threshold) {
	int n = table.size();

	// calculate S
	int s = 0;
	for (int k = 0; k < n - 1; k++) {
		for (int j = k + 1; j < n; j++) {
			s += np_sign(table.at(j).value - table.at(k).value);
		}
	}

#if F_CORRECT_TIES == 1
	// calculate the unique data
	std::vector<row_t> unique_x;
	np_unique(table, unique_x);
	int g = unique_x.size();
#endif

	// calculate the var(s)
	double var_s;
#if F_CORRECT_TIES == 1
	if (n == g) {
#endif
		// there is no tie
		var_s = (n * (n - 1) * (2 * n + 5)) / 18; 
#if F_CORRECT_TIES == 1
	} else {
		// tp = np.zeros(unique_x.shape)
		int G = unique_x.size();
		double * tp = new double[G];
		for (int i = 0; i < G; i++) {
			// sum (x == unique_x[i])
			double sum = 0;
			for (int j = 0; j < table.size(); j++) {
				if (table.at(j).value == unique_x.at(i).value) {
					sum += table.at(j).value;
				}
			}

			tp[i] = sum;
		}

		// np.sum(tp*(tp-1)*(2*tp+5))
		double sum = 0;
		for (int i = 0; i < unique_x.size(); i++) {
			sum += tp[i] * (tp[i] - 1) * (2 * tp[i] + 5);
		}

		var_s = (n*(n - 1)*(2 * n + 5) - sum) / 18;

		delete[] tp;
	}
#endif

	double z;
	if (s > 0) {
		z = (s - 1) / sqrt(var_s);
	} else if (s < 0) {
		z = (s + 1) / sqrt(var_s);
	} else {
		z = 0;
	}

	double p = 2 * (1 - norm_cdf(abs(z)));
	bool h = abs(z) > norm_ppf(1 - alpha / 2);

	std::string trend;
	if (z < 0 && h) {
		trend = "decreasing";
	} else if (z > 0 && h) {
		trend = "increasing";
	} else {
		trend = "no_trend";
	}

	if (p > threshold) {
		trend = "not_significant";
	}

	// set values
	mkrt.h = h;
	mkrt.p = p;
	mkrt.z = z;
	mkrt.trend = trend;

	// debug values
	mkrt.n = n;
#if F_CORRECT_TIES == 1
	mkrt.g = g;
#else
	mkrt.g = 0;
#endif
	mkrt.s = s;
	mkrt.var_s = var_s;

	return;
}

int np_sign(double x) {
	return (x < 0) ? -1 : ((x == 0) ? 0 : 1);
}

void np_unique(std::vector<row_t> &tbl_in, std::vector<row_t> &tbl_out) {
	for (unsigned int i = 0; i < tbl_in.size(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < tbl_out.size(); j++) {
			if (tbl_in.at(i).value == tbl_out.at(j).value) {
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

double rational_approximation (double t) {
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
	if (p <= 0.0 || p >= 1.0){
		std::stringstream os;
		os << "Invalid input argument (" << p
			<< "); must be larger than 0 but less than 1.";
		throw std::invalid_argument(os.str());
	}

	// See article above for explanation of this section.
	if (p < 0.5){
		// F^-1(p) = - G^-1(p)
		return -rational_approximation(sqrt(-2.0*log(p)));
	} else {
		// F^-1(p) = G^-1(1-p)
		return rational_approximation(sqrt(-2.0*log(1 - p)));
	}
}
