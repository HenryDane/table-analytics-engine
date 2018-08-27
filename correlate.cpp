#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include "util.h"
#include "main.h"
#include "theilsen.h"

/*
	Data must be aggregated at the same level
	ALWAYS PRINT RESULT WITH %f IN printf() CALLS FOR LESS FRUSTRATION (2.5 hrs)
*/

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
		for (int j = std::max(0, i - windowSize); j < i; j++){
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

double correlation(std::vector<row_t> tablea, std::vector<row_t> tableb, bool d) {
	if (std::min(tablea.size(), tableb.size()) == 0) {
		printf("one of the tables is zero size!\n");
		return INFINITY;
	}
	double rcl = clip_date(tablea, tableb, d);
	if (rcl != 0) {
		return rcl;
	}

	if (tablea.size() != tableb.size()) {
		for (unsigned int i = 0; i < std::min(tablea.size(), tableb.size()); i++) {
			if (d) printf("%s [%f] | %s [%f]\n", date_toString(tablea.at(i).date).c_str(), tablea.at(i).value.v, date_toString(tableb.at(i).date).c_str(), tableb.at(i).value.v);
		}
		if (d) printf("wrong size! %d %d\n", tablea.size(), tableb.size());
		if (d) printf("CRASHING! \n");
	}

	double sum_a = 0;
	double sum_b = 0;
	double sum_ab = 0;
	double sumsq_a = 0;
	double sumsq_b = 0;

	unsigned int n = std::min(tablea.size(), tableb.size());
	if (n == 0) {
		return INFINITY;
	}
	
	int s = 0;
	for (unsigned int i = 0; i < n; i++){
		if (tablea.at(i).value.f || tableb.at(i).value.f ||
			isnan(tablea.at(i).value.v) || isnan(tableb.at(i).value.v)) {
			//if (d) printf("I FOUND A NULL!\n");
			s++;
			continue;
		}
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

	n -= s; // correct for skipped

	if (n == 0) {
		return INFINITY;
	}

	sum_a = sum_a / n;
	sum_b = sum_b / n;
	sumsq_a = sumsq_a / n;
	sumsq_b = sumsq_b / n;
	sum_ab = sum_ab / n;

	double corr = ((sum_ab) - (sum_a * sum_b)) / ( sqrt((sumsq_a) - (sum_a * sum_a )) * sqrt((sumsq_b) - (sum_b * sum_b)));
	if (d) printf("%f %d (%d, %d) %f %f %f %f %f\n", corr, n, n, n, sum_a, sumsq_a, sum_b, sumsq_b, sum_ab);
	return corr;
}

// from https://www.youtube.com/watch?v=3jr_vbxajcs
double partial_correlate(std::vector<row_t> A, std::vector<row_t> B, std::vector<row_t> Z, bool d) {
	//d = true;
	if (d) printf(">>> %d %d %d\n", A.size(), B.size(), Z.size());
	strip_null(A, B, Z);
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
	if (d) printf("sorting %d %d %d\n", A.size(), B.size(), Z.size());
	std::sort(A.begin(), A.end(), date_sort);
	std::sort(B.begin(), B.end(), date_sort);
	std::sort(Z.begin(), Z.end(), date_sort);
	if (d) {
		printf("A(%s -> %s) B(%s -> %s) Z(%s -> %s)\n",
			date_toString(A.at(0).date).c_str(), date_toString(A.at(A.size() - 1).date).c_str(),
			date_toString(B.at(0).date).c_str(), date_toString(B.at(B.size() - 1).date).c_str(),
			date_toString(Z.at(0).date).c_str(), date_toString(Z.at(Z.size() - 1).date).c_str());
	}
	date_t end_date = { 9999, 99, 99, 99, 99 };
	try {
		end_date = min_date(A.at(A.size() - 1).date, min_date(B.at(B.size() - 1).date, Z.at(Z.size() - 1).date));
	} catch (std::exception &e) {
		printf("%d %d %d !!! [%s]\n", A.size(), B.size(), Z.size(), e.what());
		printf("%s %s\n", B.at(0).variable.c_str(), Z.at(0).variable.c_str());
	}
	date_t start_date = max_date(A.at(0).date, max_date(B.at(0).date, Z.at(0).date));
	//printf("clippng\n");

	// clip start date
	for (unsigned int idx = 0; idx < A.size(); idx++) {
		if (A.at(idx).date <= start_date /*is_equal(tablea.at(idx).date, startdate)*/) {
			A.erase(A.begin(), A.begin() + idx);
			if (A.size() == 0) return INFINITY;
			idx = std::min((unsigned int)0, idx - 1);
		}
	}
	for (unsigned int idx = 0; idx < B.size(); idx++) {
		if (B.at(idx).date <= start_date /*is_equal(tableb.at(idx).date, startdate)*/) {
			B.erase(B.begin(), B.begin() + idx);
			if (B.size() == 0) return INFINITY;
			idx = std::min((unsigned int)0, idx - 1);
		}
	}
	for (unsigned int idx = 0; idx < Z.size(); idx++) {
		if (Z.at(idx).date <= start_date /*is_equal(tableb.at(idx).date, startdate)*/) {
			Z.erase(Z.begin(), Z.begin() + idx);
			if (Z.size() == 0) return INFINITY;
			idx = std::min((unsigned int)0, idx - 1);
		}
	}
	// clip end date
	for (unsigned int idx = 0; idx < A.size(); idx++) {
		if (A.at(idx).date >= end_date/*is_equal(tablea.at(idx).date, enddate)*/) {
			//if (idx + 1 >= tablea.size()) break;
			A.erase(A.begin() + idx, A.end());
			if (A.size() == 0) return INFINITY;
			break;
		}
	}
	for (unsigned int idx = 0; idx < B.size(); idx++) {
		if (B.at(idx).date >= end_date/*is_equal(tableb.at(idx).date, enddate)*/) {
			//if (idx + 1 >= tableb.size()) break;
			B.erase(B.begin() + idx, B.end());
			if (B.size() == 0) return INFINITY;
			break;
		}
	}
	for (unsigned int idx = 0; idx < Z.size(); idx++) {
		if (Z.at(idx).date >= end_date/*is_equal(tableb.at(idx).date, enddate)*/) {
			//if (idx + 1 >= tableb.size()) break;
			Z.erase(Z.begin() + idx, Z.end());
			if (Z.size() == 0) return INFINITY;
			break;
		}
	}

	if (d) {
		printf("A(%s -> %s) B(%s -> %s) Z(%s -> %s)\n",
			date_toString(A.at(0).date).c_str(), date_toString(A.at(A.size() - 1).date).c_str(),
			date_toString(B.at(0).date).c_str(), date_toString(B.at(B.size() - 1).date).c_str(),
			date_toString(Z.at(0).date).c_str(), date_toString(Z.at(Z.size() - 1).date).c_str());
		printf("%d %d %d\n", A.size(), B.size(), Z.size());
	}

	// data setup
	if (A.size() != B.size() || B.size() != Z.size() || A.size() != Z.size()) {
		if (d) printf("Size mismatch\n");
		if (std::max(A.size(), std::max(B.size(), Z.size())) - std::min(A.size(), std::min(B.size(), Z.size())) <= 1) {
			if (d) printf("continuing ...\n");
			num = std::min(A.size(), std::min(B.size(), Z.size()));
		}
		else {
			return INFINITY;
		}
	} else {
		num = A.size();
	}

	//printf("p_c() -> A (%d) %s to %s \n", A.size(), date_toString(A.at(0).date).c_str(), date_toString(A.at(A.size() - 1).date).c_str());
	//printf("p_c() -> B (%d) %s to %s \n", B.size(), date_toString(B.at(0).date).c_str(), date_toString(B.at(B.size() - 1).date).c_str());
	//printf("p_c() -> Z (%d) %s to %s \n", Z.size(), date_toString(Z.at(0).date).c_str(), date_toString(Z.at(Z.size() - 1).date).c_str());

	// step one & two & four: calculate sums
	int nxx = 0;
	for (int i = 0; i < num; i++) {
		//printf("A: %f B: %f Z: %f\n", A[i].value.v, B[i].value.v, Z[i].value.v);
		if (A.at(i).value.f || B.at(i).value.f || Z.at(i).value.f ||
			isnan(A.at(i).value.v) || isnan(B.at(i).value.v) || isnan(Z.at(i).value.v)) {
			if (d) printf("Skipped \n");
			continue;
		}

		sum_a += A.at(i).value.v;
		sum_b += B.at(i).value.v;
		sum_z += Z.at(i).value.v;

		sum_aa += (A.at(i).value.v * A.at(i).value.v);
		sum_bb += (B.at(i).value.v * B.at(i).value.v);
		sum_zz += (Z.at(i).value.v * Z.at(i).value.v);

		sum_ab += (A.at(i).value.v * B.at(i).value.v);
		sum_bz += (B.at(i).value.v * Z.at(i).value.v);
		sum_az += (A.at(i).value.v * Z.at(i).value.v);
		//printf("EA=%f EB=%f EZ=%f \n", sum_a, sum_b, sum_z);
		//printf("E(AA)=%f E(BB)=%f E(ZZ)=%f \n", sum_aa, sum_bb, sum_zz);
		//printf("E(AB)=%f E(BZ)=%f E(AZ)=%f \n", sum_ab, sum_bz, sum_az);
		nxx++;
	}

	if (d) printf("nxx: %d\n", nxx);
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