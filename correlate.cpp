#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include "util.h"
#include "main.h"
#include "theilsen.h"

/*
	Data must be aggregated at the same level
	ALWAYS PRINT RESULT WITH %f IN printf() CALLS FOR LESS FRUSTRATION (2.5 hrs)
*/
double mean(std::vector<row_t> &table) {
	double a = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		a += table.at(i).value.v;
	}

	return (a / table.size());
}

double std_dev(std::vector<row_t> &table) {
	double mu = mean(table);
	double a = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		a += pow(table.at(i).value.v - mu, 2);
	}

	return sqrt(a / table.size());
}

/*
	"right-hand" running average
	mov_avg will be smaller than table by windowsize
*/
void running_avg(std::vector<row_t> &table, std::vector<double> mov_avg, int windowSize) {
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

double coef_var(std::vector<row_t> &table) {
	return (std_dev(table) / mean(table));
}

inline bool IsNaN(float A)
{
	return ((*(uint32_t*)&A) & 0x7FFFFFFF) > 0x7F800000;
}

double correlation(std::vector<row_t> tablea, std::vector<row_t> tableb, bool d) {
#if F_SKIP_NULLS == 0

	// resolve NAN
	std::sort(tablea.begin(), tablea.end(), date_sort);
	std::sort(tableb.begin(), tableb.end(), date_sort);

	if (d) {
		printf("ORIGINAL: A(%s -> %s) B(%s -> %s)\n",
			date_toString(tablea.at(0).date).c_str(), date_toString(tablea.at(tablea.size() - 1).date).c_str(),
			date_toString(tableb.at(0).date).c_str(), date_toString(tableb.at(tableb.size() - 1).date).c_str());
	}

	date_t startdate = max_date(tablea.at(0).date, tableb.at(0).date);
	date_t enddate = min_date(tablea.at(tablea.size() - 1).date, tableb.at(tableb.size() - 1).date);

	// clip start date
	for (unsigned int idx = 0; idx < tablea.size(); idx++) {
		//printf("%d / %d\r", idx, tablea.size());
		if (is_equal(tablea.at(idx).date, startdate)) {
			tablea.erase(tablea.begin(), tablea.begin() + idx);
			break;
		}
	}
	for (unsigned int idx = 0; idx < tableb.size(); idx++) {
		//printf("%d / %d\r", idx, tableb.size());
		if (is_equal(tableb.at(idx).date, startdate)) {
			tableb.erase(tableb.begin(), tableb.begin() + idx);
			break;
		}
	}
	// clip end date
	for (unsigned int idx = 0; idx < tablea.size(); idx++) {
		//printf("%d / %d\r", idx, tablea.size());
		if (is_equal(tablea.at(idx).date, enddate)) {
			if (idx + 1 >= tablea.size()) break;
			tablea.erase(tablea.begin() + idx + 1, tablea.end());
			break;
		}
	}
	for (unsigned int idx = 0; idx < tableb.size(); idx++) {
		//printf("%d / %d\r", idx, tableb.size());
		if (is_equal(tableb.at(idx).date, enddate)) {
			if (idx + 1 >= tableb.size()) break;
			tableb.erase(tableb.begin() + idx + 1, tableb.end());
			break;
		}
	}

	if (d) {
		printf("FINAL: A(%s -> %s) B(%s -> %s)\n",
			date_toString(tablea.at(0).date).c_str(), date_toString(tablea.at(tablea.size() - 1).date).c_str(),
			date_toString(tableb.at(0).date).c_str(), date_toString(tableb.at(tableb.size() - 1).date).c_str());
	}

	if (tablea.size() != tableb.size()) {
		printf("wrong size! %d %d\n", tablea.size(), tableb.size());
		return INFINITY;
	}

#elif F_SKIP_NULLS == 1
#if _MSC_VER
#pragma message ("Warning : F_SKIP_NULLS=1 is unsafe for double correlation(std::vector<row_t>, std::vector<row_t>, bool )")
#elif   __GNUC__
	#warning("Warning : F_SKIP_NULLS=1 is unsafe for double correlation(std::vector<row_t>, std::vector<row_t>, bool )")
#endif
#endif

	double sum_a = 0;
	double sum_b = 0;
	double sum_ab = 0;
	double sumsq_a = 0;
	double sumsq_b = 0;

	//printf("%d == %d\n", a.size(), b.size());

	int n = tablea.size();

	for (int i = 0; i < n; i++){
		// sum of elements of array A.
		sum_a += tablea.at(i).value.v;

		// sum of elements of array B.
		sum_b += tableb.at(i).value.v;

		// sum of A[i] * B[i].
		sum_ab += tablea.at(i).value.v * tableb.at(i).value.v;

		// sum of square of array elements.
		sumsq_a += tablea.at(i).value.v * tablea.at(i).value.v;
		sumsq_b += tableb.at(i).value.v * tableb.at(i).value.v;
	}

	sum_a = sum_a / n;
	sum_b = sum_b / n;
	sumsq_a = sumsq_a / n;
	sumsq_b = sumsq_b / n;
	sum_ab = sum_ab / n;

	double corr = ((sum_ab) - (sum_a * sum_b)) / ( sqrt((sumsq_a) - (sum_a * sum_a )) * sqrt((sumsq_b) - (sum_b * sum_b)));
	//if (d) printf("%f %d (%d, %d) %f %f %f %f %f\n", corr, n, e_a - s_a, e_b - s_b, sum_a, sumsq_a, sum_b, sumsq_b, sum_ab);
	return corr;
}

// from https://www.youtube.com/watch?v=3jr_vbxajcs
double partial_correlate(std::vector<row_t> A, std::vector<row_t> B, std::vector<row_t> Z, bool d) {
	int num = -1;

	// sums
	double sum_a = 0; 
	double sum_b = 0;
	double sum_z = 0;
	double sum_ab = 0;
	double sum_bz = 0;
	double sum_az = 0;

	// squared sums
	double sum_aa = 0;
	double sum_bb = 0;
	double sum_zz = 0;

	// fastish filter
	std::sort(A.begin(), A.end(), date_sort);
	std::sort(B.begin(), B.end(), date_sort);
	std::sort(Z.begin(), Z.end(), date_sort);
	if (d) {
		printf("A(%s -> %s) B(%s -> %s) Z(%s -> %s)\n",
			date_toString(A.at(0).date).c_str(), date_toString(A.at(A.size() - 1).date).c_str(),
			date_toString(B.at(0).date).c_str(), date_toString(B.at(B.size() - 1).date).c_str(),
			date_toString(Z.at(0).date).c_str(), date_toString(Z.at(Z.size() - 1).date).c_str());
	}
	date_t end_date = min_date(A.at(A.size() - 1).date, min_date(B.at(B.size() - 1).date, Z.at(Z.size() - 1).date));
	date_t start_date = max_date(A.at(0).date, max_date(B.at(0).date, Z.at(0).date));
	//printf("clippng\n");
	// clip start date
	for (unsigned int idx = 0; idx < A.size(); idx++) {
		//printf("%d / %d\r", idx, A.size());
		if (is_equal(A.at(idx).date, start_date)) {
			A.erase(A.begin(), A.begin() + idx);
			break;
		}
	}
	for (unsigned int idx = 0; idx < B.size(); idx++) {
		//printf("%d / %d\r", idx, B.size());
		if (is_equal(B.at(idx).date, start_date)) {
			B.erase(B.begin(), B.begin() + idx);
			break;
		}
	}
	for (unsigned int idx = 0; idx < Z.size(); idx++) {
		//printf("%d / %d\r", idx, Z.size());
		if (is_equal(Z.at(idx).date, start_date)) {
			Z.erase(Z.begin(), Z.begin() + idx);
			break;
		}
	}

	// clip end date
	for (unsigned int idx = 0; idx < A.size(); idx++) {
		//printf("%d / %d\r", idx, A.size());
		if (is_equal(A.at(idx).date, end_date)) {
			if (idx + 1 >= A.size()) break;
			A.erase(A.begin() + idx + 1, A.end());
			break; 
		}
	}
	for (unsigned int idx = 0; idx < B.size(); idx++) {
		//printf("%d / %d\r", idx, B.size());
		if (is_equal(B.at(idx).date, end_date)) {
			if (idx + 1 >= B.size()) break;
			B.erase(B.begin() + idx + 1, B.end());
			break;
		}
	}
	for (unsigned int idx = 0; idx < Z.size(); idx++) {
		//printf("%d / %d\r", idx, Z.size());
		if (is_equal(Z.at(idx).date, end_date)) {
			if (idx + 1 >= Z.size()) break;
			Z.erase(Z.begin() + idx + 1, Z.end());
			break;
		}
	}
	//printf("\n");

	if (d) {
		printf("A(%s -> %s) B(%s -> %s) Z(%s -> %s)\n",
			date_toString(A.at(0).date).c_str(), date_toString(A.at(A.size() - 1).date).c_str(),
			date_toString(B.at(0).date).c_str(), date_toString(B.at(B.size() - 1).date).c_str(),
			date_toString(Z.at(0).date).c_str(), date_toString(Z.at(Z.size() - 1).date).c_str());
	}

	// data setup
	if (A.size() != B.size() || B.size() != Z.size() || A.size() != Z.size()) {
		printf("Size mismatch\n");
		return INFINITY;
	} else {
		num = A.size();
	}

	// step one & two & four: calculate sums
	for (int i = 0; i < num; i++) {
		sum_a += A.at(i).value.v;
		sum_b += B.at(i).value.v;
		sum_z += Z.at(i).value.v;

		sum_aa += (A.at(i).value.v * A.at(i).value.v);
		sum_bb += (B.at(i).value.v * B.at(i).value.v);
		sum_zz += (Z.at(i).value.v * Z.at(i).value.v);

		sum_ab += (A.at(i).value.v * B.at(i).value.v);
		sum_bz += (B.at(i).value.v * Z.at(i).value.v);
		sum_az += (A.at(i).value.v * Z.at(i).value.v);
	}

	// step three : SS_Xs
	double ss_a = sum_aa - (pow(sum_a, 2) / num);
	double ss_b = sum_bb - (pow(sum_b, 2) / num);
	double ss_z = sum_zz - (pow(sum_z, 2) / num);

	// step five : SP_XYs
	double sp_ab = sum_ab - ((sum_a * sum_b) / num);
	double sp_az = sum_az - ((sum_a * sum_z) / num);
	double sp_bz = sum_bz - ((sum_b * sum_z) / num);

	// step six : r_XYs
	double r_ab = sp_ab / sqrt(ss_a * ss_b);
	double r_az = sp_az / sqrt(ss_a * ss_z);
	double r_bz = sp_bz / sqrt(ss_b * ss_z);

	// step seven : result
	double r_ab_nz = (r_ab - (r_az * r_bz)) / sqrt((1 - pow(r_az, 2)) * (1 - pow(r_bz, 2)));

	// cant print Σ so E is used instead :[
	// also cant print ² so letter is repeated :[
	if (d) {
		printf("EA=%f EB=%f EZ=%f \n", sum_a, sum_b, sum_z);
		printf("E(AA)=%f E(BB)=%f E(ZZ)=%f \n", sum_aa, sum_bb, sum_zz);
		printf("SS_A=%f SS_B=%f SS_Z=%f \n", ss_a, ss_b, ss_z);
		printf("E(AB)=%f E(BZ)=%f E(AZ)=%f \n", sum_ab, sum_bz, sum_az);
		printf("SP_AB=%f SP_BZ=%f SP_AZ=%f \n", sp_ab, sp_bz, sp_az);
		printf("r_AB=%f r_BZ=%f r_AZ=%f \n", r_ab, r_bz, r_az);
	}

	return r_ab_nz;
} 