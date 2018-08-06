#pragma once

#include <string>
#include <tuple>

struct date_t {
	int year;
	int month;
	int day;
	int hours;
	int minuites;

	bool operator <(const date_t& x) {
		return std::tie(year, month, day, hours, minuites) < std::tie(x.year, x.month, x.day, x.hours, x.minuites);
	}

	bool operator >(const date_t& x) {
		return std::tie(year, month, day, hours, minuites) > std::tie(x.year, x.month, x.day, x.hours, x.minuites);
	}

	bool operator ==(const date_t& x) {
		return std::tie(year, month, day, hours, minuites) == std::tie(x.year, x.month, x.day, x.hours, x.minuites);
	}

	bool operator <=(const date_t& x) {
		return std::tie(year, month, day, hours, minuites) <= std::tie(x.year, x.month, x.day, x.hours, x.minuites);
	}

	bool operator >=(const date_t& x) {
		return std::tie(year, month, day, hours, minuites) >= std::tie(x.year, x.month, x.day, x.hours, x.minuites);
	}
};

struct s_double_t {
	double v;
	bool f = false;
};

struct row_t {
	int id;
	date_t date;
	std::string variable;
	s_double_t value; // reeeeeee
	std::string units;
	int edits = 1;
	int state = 0;
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

enum agg_t {
	YEARLY = 0, MONTHLY, DAILY, NONE
};

struct period_t {
	std::string name;
	agg_t aggregation;
	date_t start;
	date_t end;
};

struct macro_t {
	std::string name;
	std::vector<std::string> tokens;
	bool empty = true;
	bool valid = false;
};

struct script_flag_t {
	bool f_underscore_is_space = false;
	bool f_colors_ok = false;
	bool f_auto_reset_filters = false;
};

struct list_t {
	std::string name;
	std::vector<std::string> items;
};

#define F_CORRECT_TIES 0
#define F_MAKE_CSV 1 // leave at 1
#define F_PRINT_VARS 1
#define F_MAKE_FILTER_FILES 0
#define F_CLIP_GLOBAL 0
#define F_GEN_TS_ALL 1
#define F_DO_MONTHS 1
#define F_AGG_YEARLY 0
#define F_REBUILD_TABLE 1

#define DAYS_PER_TYP_MONTH ((double) ((31 * 7) + (30 * 4) + 28.75) / 12)
//#define UNITS "MAF"

#define FIRST_DATE ((date_t){0, 0, 0})
#define END_DATE ((date_t){9999, 99, 99})

const std::string months[13] = { "N/A", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//int merge_duplicates_by_date(std::vector<row_t> &table);
bool execute_config(std::vector<std::string> &variables, std::vector<variable_t> &rules, std::vector<custom_var_t> &cvars);
bool apply_rules(std::vector<std::string> &apply_to, std::vector<std::string> &rules_list, std::vector<std::string> &variables, std::vector<variable_t> &rules);
