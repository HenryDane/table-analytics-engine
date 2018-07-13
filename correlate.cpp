#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include "main.h"
#include "theilsen.h"

/*
	Data must be aggregated at the same level
	ALWAYS PRINT RESULT WITH %f IN printf() CALLS FOR LESS FRUSTRATION (2.5 hrs)
*/
double mean(std::vector<row_t> &table) {
	double a = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		a += table.at(i).value;
	}

	return (a / table.size());
}

double std_dev(std::vector<row_t> &table) {
	double mu = mean(table);
	double a = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		a += pow(table.at(i).value - mu, 2);
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
	for (unsigned int i = 0; i < NUMBERS_SIZE; i++) {
		numbers[i] = table.at(i).value;
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

double correlation(std::vector<row_t> &tablea, std::vector<row_t> &tableb, bool d) {
	date_t startdate = max_date(min_table_date(tablea), min_table_date(tableb));
	date_t enddate = min_date(max_table_date(tablea), max_table_date(tableb));

	int s_a = -1;
	int e_a = -1;
	int s_b = -1;
	int e_b = -1;
	for (unsigned int i = 0; i < tablea.size(); i++) {
		if (date_as_day(tablea.at(i).date) == date_as_day(startdate)) {
			if (s_a == -1) s_a = i;
		}
		if (date_as_day(tablea.at(i).date) == date_as_day(enddate)) {
			if (e_a == -1) e_a = i;
		}
	}
	if (s_a == -1 || e_a == -1) {
		printf("\n[WARN] Failed to lookup correct start/end values for tablea (%d, %d)\n ", s_a, e_a);
	}

	for (unsigned int i = 0; i < tableb.size(); i++) {
		if (date_as_day(tableb.at(i).date) == date_as_day(startdate)) {
			if (s_b == -1) s_b = i;
		}
		if (date_as_day(tableb.at(i).date) == date_as_day(enddate)) {
			if (e_b == -1) e_b = i;
		}
	}
	if (s_b == -1 || e_b == -1) {
		printf("\n[WARN] Failed to lookup correct start/end values for tableb (%d, %d)\n ", s_b, e_b);
	}

	//printf("TA [%d -> %d] [%d]\nTB [%d - > %d] [%d]\n(%d/%d/%d) -> (%d/%d/%d)\n", s_a, e_a, e_a - s_a, s_b, e_b, e_b - s_b,startdate.month, startdate.day, startdate.year, enddate.month, enddate.day, enddate.year);

	std::vector<double> a;
	std::vector<double> b;
	std::ofstream file;
	/*int qpoe = rand();
	std::string name = "tablea_" + std::to_string(qpoe) + ".txt";
	file.open(name, std::ios::trunc);
	if (!file.is_open()) {
		printf("could not save results (a) \n");
	} */

	for (int i = s_a; i < e_a; i++) {
		a.push_back(tablea.at(i).value);
		//printf("%f ", tablea.at(i).value);
		//file << tablea.at(i).date.month << "/" << tablea.at(i).date.day << "/" << tablea.at(i).date.year << " " << tablea.at(i).variable << " " << tablea.at(i).value << std::endl;
	}
	//printf("\n");
	/*file.close();
	name = "tableb_" + std::to_string(qpoe) + ".txt";
	file.open("tableb.txt", std::ios::trunc);
	if (!file.is_open()) {
		printf("could not save results (b) \n");
	} */
	for (int i = s_b; i < e_b; i++) {
		b.push_back(tableb.at(i).value);
		//file << tableb.at(i).date.month << "/" << tableb.at(i).date.day << "/" << tableb.at(i).date.year << " " << tableb.at(i).variable << " " << tableb.at(i).value << std::endl;
		//printf("%f ", tableb.at(i).value);
	}
	file.close(); 

	double sum_a = 0;
	double sum_b = 0;
	double sum_ab = 0;
	double sumsq_a = 0;
	double sumsq_b = 0;

	//printf("%d == %d\n", a.size(), b.size());

	int n = std::min(e_a - s_a, e_b - s_b);

	for (int i = 0; i < n; i++){
		// sum of elements of array A.
		sum_a += a.at(i);

		// sum of elements of array B.
		sum_b += b.at(i);

		// sum of A[i] * B[i].
		sum_ab += a.at(i) * b.at(i);

		// sum of square of array elements.
		sumsq_a += a.at(i) * a.at(i);
		sumsq_b += b.at(i) * b.at(i);
	}

	sum_a = sum_a / n;
	sum_b = sum_b / n;
	sumsq_a = sumsq_a / n;
	sumsq_b = sumsq_b / n;
	sum_ab = sum_ab / n;

	double corr = ((sum_ab) - (sum_a * sum_b)) / ( sqrt((sumsq_a) - (sum_a * sum_a )) * sqrt((sumsq_b) - (sum_b * sum_b)));
	//printf("%f %d (%d, %d) %f %f %f %f %f\n", corr, n, e_a - s_a, e_b - s_b, sum_a, sumsq_a, sum_b, sumsq_b, sum_ab);
	return corr;
}