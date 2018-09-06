#include <vector>
#include <sstream>
#include <algorithm>
#include "main.h"
//#include "correlate.h"
#include "util.h"


double median(std::vector<row_t> table) {
	null_shield(table);
	std::sort(table.begin(), table.end(), table_value_sort);
	//printf("\n\n");
	//print_table_c(table);
	//printf("\n\n");
	//printf("median() -> start: %f middle: %f, end: %f", table.at(0).value.v, table.at((table.size() / 2)).value.v, table.at(table.size() - 1).value.v);
	if (table.size() % 2) {
		return table.at((table.size() - 1) / 2).value.v; // odd
	}
	else {
		try {
			return (table.at((table.size() / 2) - 1).value.v + table.at(table.size() / 2).value.v) / 2; //even
		}
		catch (std::exception &e) {
			(void)e;
			//std::cout << e.what() << std::endl;
			//std::cout << table.size() << std::endl;
			return NAN;
		}
	}
}

double mean(std::vector<row_t> table) {
	null_shield(table);
	double a = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		a += table.at(i).value.v;
	}
	//printf("mean() -> a: %f size: %d mean: %f\n", a, table.size(), a / table.size());
	return (a / table.size());
}

double std_dev(std::vector<row_t> table) {
	null_shield(table);
	double mu = mean(table);
	double a = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		a += pow(table.at(i).value.v - mu, 2);
	}

	//printf("std_dev() -> a: %f mu: %f tbl: %d inner: %f, result: %f\n", a, mu, table.size(), a / table.size(), sqrt(a/ table.size()));
	return sqrt(a / table.size());
}

/*
"right-hand" running average
mov_avg will be smaller than table by windowsize
*/
void running_avg(std::vector<row_t> table, std::vector<double> &mov_avg, int windowSize) {
	int NUMBERS_SIZE = table.size();
	double * numbers = new double[NUMBERS_SIZE];
	for (int i = 0; i < NUMBERS_SIZE; i++) {
		numbers[i] = table.at(i).value.v;
	}

	mov_avg.clear();

	double sum = 0.0;
	double movingAverage = 0.0;


	// Loop through nums in list, excluding any at the end that will be covered by the nested for-loop.
	for (int i = 0; i <= (NUMBERS_SIZE - windowSize); i++) {

		sum = 0.0; // Reinitialize sum back to zero.
				   //printf("For numbers "); // Output message.

				   // Loop through x numbers from current i position, where x = windowSize.
		for (int j = i; j < i + windowSize; j++) {
			sum += numbers[j]; // Increment sum.
							   //printf("%f ", numbers[j]);
		}

		// Calculate moving average and display.
		movingAverage = sum / windowSize;
		//printf("\nMoving Avg: %f\n\n", movingAverage);
		mov_avg.push_back(movingAverage);
	}

	delete[] numbers;
}

/*
computes a running average "from the left"
will do sum / 1, sum / 2, until it hits window size

untested

NAN intolerant
*/
void running_avg_left(std::vector<row_t> table, std::vector<double> &mov_avg, int windowSize) {
	int NUMBERS_SIZE = table.size();

	// allocated new array
	double * numbers = new double[NUMBERS_SIZE];

	// copy values to array
	for (int i = 0; i < NUMBERS_SIZE; i++) {
		numbers[i] = table.at(i).value.v;
	}

	// clean result
	mov_avg.clear();

	double sum = 0.0;
	double movingAverage = 0.0;

	// Loop through nums in list, excluding any at the end that will be covered by the nested for-loop.
	for (int i = 0; i <= NUMBERS_SIZE; i++) {

		sum = 0.0; // Reinitialize sum back to zero.
				   //printf("For numbers "); // Output message.

				   // Loop through x numbers from current i position, where x = windowSize.
				   /*for (int j = i; j < i + windowSize; j++) {*/
		for (int j = std::max(0, i - windowSize); j < i; j++) {
			sum += numbers[j]; // Increment sum.
							   //printf("%f ", numbers[j]);
		}

		// Calculate moving average and display.
		if (i - windowSize == 0) {
			movingAverage = sum / windowSize;
		}
		else {
			movingAverage = sum / i;
		}
		//printf("\nMoving Avg: %f\n\n", movingAverage);
		mov_avg.push_back(movingAverage);
	}

	// lawful good
	delete[] numbers;
}

double coef_var(std::vector<row_t> table) {
	return (std_dev(table) / mean(table));
}

inline bool IsNaN(float A)
{
	return ((*(uint32_t*)&A) & 0x7FFFFFFF) > 0x7F800000;
}

quantile_t scimath_quantile(std::vector<row_t> &t, double lbound, double ubound) {
	quantile_t q;

	std::sort(t.begin(), t.end(), table_value_sort);

	int n = t.size();

	if (n <= 1) {
		return { NAN, NAN };
	}

	double lq = lbound * n;
	double uq = ubound * n;

	if (floor(lq) == lq) {
		q.l = t[(int) lq - 1].value.v;
	}
	else {
		q.l = (t[(int) floor(lq) - 1].value.v + t[(int) floor(lq)].value.v) / 2;
	}

	if (floor(uq) == uq) {
		q.h = t[(int) uq - 1].value.v;
	}
	else {
		q.h = (t[(int) floor(uq) - 1].value.v + t[(int) floor(uq)].value.v) / 2;
	}

	return q;
}

quantile_t scimath_quantile(double * t, int n, double lbound, double ubound) {
	quantile_t q;

	std::sort(t, t + n);

	if (n <= 1) {
		return{ NAN, NAN };
	}

	double lq = lbound * n;
	double uq = ubound * n;

	if (floor(lq) == lq) {
		q.l = t[(int) lq - 1];
	}
	else {
		q.l = (t[(int) floor(lq) - 1] + t[(int) floor(lq)]) / 2;
	}

	if (floor(uq) == uq) {
		q.h = t[(int) uq - 1];
	}
	else {
		q.h = (t[(int) floor(uq) - 1] + t[(int) floor(uq)]) / 2;
	}

	return q;
}

std::vector<row_t> scipy_median_filter(std::vector<row_t> &t, int window) {
	//null_shield(t);
	std::vector<row_t> b;

	for (unsigned int i = 0; i < t.size(); i++) {
		std::vector<row_t> a;
		int n = t.size();
		//printf("smf %d / %d \r", i + 1, n);
		for (unsigned int j = i - window; j <= i + window; j++) {
			try {
				a.push_back(t.at(j));
			}
			catch (std::exception &e) {	
				(void)e;
			}
		}

		if (a.size() == 0) continue;

		row_t r = t[i];
		if (a.size() == 0) {
			r.value.v = NAN;
			r.value.f = true;
		}
		else {
			r.value.v = median(a);
			r.value.f = isnan(r.value.v);
		}

		b.push_back(r);
	}

	return b;
}

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

/*
	wavelet sources
	http://www.jhuapl.edu/techdigest/views/pdfs/V15_N4_1994/V15_N4_1994_Sadowsky.pdf
	https://people.cs.kuleuven.be/~adhemar.bultheel/WWW/WAVE/print4.pdf
	https://en.wikipedia.org/wiki/Continuous_wavelet_transform
*/