#include <vector>
#include <sstream>
#include <cmath>
#include <iostream>
#include "main.h"
#include "mkt.h"
#include "scimath.h"
#include "util.h"

void mann_kendall_test(mk_result_t &mkrt, std::vector<row_t> table, double alpha, double threshold) {
	null_shield(table);

	unsigned int n = table.size();

	// calculate S
	int s = 0;
	for (int k = 0; k < n - 1; k++) {
		for (int j = k + 1; j < n; j++) {
			s += np_sign(table.at(j).value.v - table.at(k).value.v);
		}
	}
	//printf("%d \n", s);
	//abort();

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
	//printf("\n::: %f %f %f %f | %f %f | %d %d \n", p, norm_cdf(abs(z)), abs(z), z, s, var_s, n, 0);
	//std::cout << p << " " << norm_cdf(abs(z)) << " " << abs(z) << " " << z << " " << s << " " << var_s << " " << n << " " << 0 << std::endl;
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
