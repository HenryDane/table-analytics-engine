#pragma once

#include <string>

struct date_t {
	int year;
	int month;
	int day;
};

struct s_double_t {
	double v;
	bool f = false;
};

struct row_t {
	int id;
	date_t date;
	std::string variable;
//	double value;
	s_double_t value; // reeeeeee
	std::string units;
};

struct mk_result_t {
	std::string trend;
	bool h;
	double p;
	double z;

	double var_s;
	double s;
	int n;
	int g;
};

struct ts_result_t {
	double slope;
	double r_u;
	double r_l;
};

struct linreg_result_t {
	double m;
	double b;
	double r;
};

struct data_t {
	double x;
	double y;
	double v;
};

struct rgb_t {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct custom_var_t {
	std::string name;
	std::vector<std::string> pieces;
};

struct variable_t {
	std::string name;
	std::vector<std::string> correlations;
};

#define F_SKIP_NULLS 0 // 0 : N/A -> NAN, 1 : N/A -> continue, 2 : N/A -> 0
#define F_CORRECT_TIES 0
#define F_MAKE_CSV 1 // leave at 1
#define F_MAKE_DIRS 1
#define F_PRINT_VARS 1
#define F_MAKE_FILTER_FILES 0

#define DAYS_PER_TYP_MONTH (double) ((31 * 7) + (30 * 4) + 28.75) / 12
#define UNITS "MAF"

const std::string months[13] = { "N/A", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//int merge_duplicates_by_date(std::vector<row_t> &table);
bool execute_config(std::vector<std::string> &variables, std::vector<variable_t> &rules, std::vector<custom_var_t> &cvars);
bool apply_rules(std::vector<std::string> &apply_to, std::vector<std::string> &rules_list, std::vector<std::string> &variables, std::vector<variable_t> &rules);
