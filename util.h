#pragma once
#include <string>
#include <vector>
#include "main.h"

// string functions
bool startsWith(std::string mainStr, std::string toMatch);

// table functions
bool is_inside(std::vector<std::string> vec, std::string str);
void filter_by_variable(std::vector<row_t> table, std::vector<row_t> &otable, std::vector<std::string> varnames, bool merge = true);
int reduce_by_var(std::vector<row_t> table, std::vector<row_t> bad_rows, std::string varname);
void make_unique(std::vector<std::string> &vec);

// date util functions
std::string date_toString(date_t &a);
bool date_sort(row_t i, row_t j);
double date_as_year(date_t &date);
double date_as_month(date_t &date);
int date_as_day(date_t &date);
bool is_equal(date_t &a, date_t &b);
date_t min_table_date(std::vector<row_t> &table);
date_t max_table_date(std::vector<row_t> &table);
double min_table(std::vector<row_t> &table);
double min_table(std::vector<row_t> &table);
date_t min_date(date_t &a, date_t &b);
date_t max_date(date_t &a, date_t &b);