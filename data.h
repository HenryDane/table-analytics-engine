#pragma once
#include <vector>
#include "main.h"
#include "util.h"

bool filter_bounds(std::vector<row_t> &table, double lbound, double ubound);
bool filter_norepeats(std::vector<row_t> &table, unsigned int threshold);
bool filter_med_outliers(std::vector<row_t> &table,
	bool secfilt = true,
	double level = 3.0,
	double scale = NAN,
	int filter_len = 7,
	double lquantile = .25,
	double uquantile = .75,
	double seclevel = 3.0,
	double secscale = NAN,
	int secfilt_len = 241,
	double seclquantile = .25,
	double secuquantile = .75);
bool filter_delta_limit(std::vector<row_t> &table, double delta_limit);
bool clean_data(std::vector<row_t> &table);

void resolve_table_state(std::vector<row_t> &table);
int old_table_agg(std::vector<row_t> &table);
void period_filter(std::vector<row_t> &table, period_t period);
bool degap_table_monthly(std::vector<row_t> &table);
int read_db(std::ifstream &data, std::vector<row_t> &table);

std::vector<row_t> normalize_table(std::vector<row_t> t, std::vector<std::string> vars);