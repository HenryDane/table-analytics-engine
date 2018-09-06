#pragma once
#include <string>
#include <vector>
#include "main.h"

// string functions
bool startsWith(std::string mainStr, std::string toMatch);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> split(const std::string& s, char delimiter);
void print_table_c(std::vector<row_t> &t);

// table functions
bool is_inside(std::vector<std::string> vec, std::string str);
void filter_by_variable(std::vector<row_t> table, std::vector<row_t> &otable, std::vector<std::string> varnames, std::string rename = "__renamed__", bool merge = true, bool f_rename = false);
int reduce_by_var(std::vector<row_t> table, std::vector<row_t> bad_rows, std::string varname);
void make_unique(std::vector<std::string> &vec);
void aggregate_table(std::vector<row_t> &table, agg_t agg);
void strip_null(std::vector<row_t> &tablea, std::vector<row_t> &tableb);
void strip_null(std::vector<row_t> &a, std::vector<row_t> &b, std::vector<row_t> &c);
void null_shield(std::vector<row_t> &table);
double clip_date(std::vector<row_t> &tablea, std::vector<row_t> &tableb, bool d = false);
void reflag_table(std::vector<row_t> &t);
void gen_plot(std::vector<row_t> t);
std::vector<std::string> id_variables(std::vector<row_t> t);
//void build_custom_vars(std::vector<row_t> &t, std::vector<custom_var_t> cvars);

// sort functions
bool date_sort(row_t i, row_t j);
bool date_sort_d(date_t i, date_t j);
bool date_sort_dr(date_t i, date_t j);
bool edit_sort(row_t i, row_t j);
bool table_value_sort(row_t i, row_t j);
bool table_id_sort(row_t i, row_t j);

// date functions
std::string date_toString(date_t &a);
double date_as_year(date_t &date);
double date_as_month(date_t &date);
int date_as_day(date_t date);
bool is_equal(date_t &a, date_t &b);
date_t min_table_date(std::vector<row_t> &table);
date_t max_table_date(std::vector<row_t> &table);
double min_table(std::vector<row_t> &table);
double max_table(std::vector<row_t> &table);
date_t min_date(date_t &a, date_t &b);
date_t max_date(date_t &a, date_t &b);

// period functions
bool date_in_period(date_t d, period_t p);

// windows functions
int DeleteDirectory(const std::wstring &refcstrRootDirectory, bool bDeleteSubdirectories = true);

// s_double_t functions
s_double_t safe_add(s_double_t &a, s_double_t &b);

void print_wide_table(std::vector<row_t> table);