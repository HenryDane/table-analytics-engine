#include <vector>
#include <math.h> 
#include "main.h"
#include "util.h"

bool linear_regression(linreg_result_t &result, std::vector<row_t> &table) {
	double sumx = 0.0; // sum of x     
	double sumx2 = 0.0; // sum of x**2  
	double sumxy = 0.0; // sum of x * y 
	double sumy = 0.0; // sum of y     
	double sumy2 = 0.0; // sum of y**2  

	for (unsigned int i = 0; i< table.size(); i++) {
		sumx += date_as_month(table.at(i).date); //x[i];
		sumx2 += pow(date_as_month(table.at(i).date), 2);
		sumxy += date_as_month(table.at(i).date) * table.at(i).value.v;
		sumy += table.at(i).value.v; // y[i];
		sumy2 += pow(table.at(i).value.v, 2);
	}

	double denom = (table.size() * sumx2 - pow(sumx, 2));
	if (denom == 0) {
		// singular matrix. can't solve the problem.
		result.m = NAN;
		result.b = NAN;
		if (result.r) result.r = NAN;
		return false;
	}

	result.m = (table.size() * sumxy - sumx * sumy) / denom;
	result.b = (sumy * sumx2 - sumx * sumxy) / denom;
	result.r = (sumxy - sumx * sumy / table.size()) /    /* compute correlation coeff */
		sqrt((sumx2 - pow(sumx, 2) / table.size()) *
		(sumy2 - pow(sumy, 2) / table.size()));

	return true;
}