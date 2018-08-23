#define _CRT_SECURE_NO_WARNINGS // so I can use sprintf
#pragma once

#include <string>
#include <tuple>
#include <fstream>

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

	double numeric() {
		double day_sum = 0;
		day_sum += (double)((double) hours / 24);
		day_sum += (double)((double) minuites / 60) / 24;
		for (int i = 1; i < this->month; i++) {
			int days_month = 0;
			if (i == 9 || i == 4 || i == 6 || i == 11) {
				// sept/aprl/june/novm
				days_month = 30;
			}
			else if (i == 2) {
				if (abs((this->year - 1600)) % 4 == 0) {
					// leap year
					days_month = 29;
				}
				else {
					days_month = 28;
				}
			}
			else {
				days_month = 31;
			}

			day_sum += days_month;
		}

		day_sum += this->year * 365;
		day_sum += this->day;
		
		return day_sum;
	}

	std::string toString() {
		char tmp[80];
		sprintf(tmp, "%02d/%02d/%04d %02d:%02d", month, day, year, hours, minuites);
		return std::string(tmp);
	}
};

struct s_double_t {
	double v;
	bool f = false; // nan flag (true = NAN)
	bool i = false; // interpolate flag (true = interpolate)
};

struct row_t {
	int id;
	date_t date;
	std::string variable;
	s_double_t value; // reeeeeee
	std::string units;
	int edits = 1;
	int state = 0;

	std::string toString() {
		char tmp[160];
		sprintf(tmp, "[%d] [%s] %s : %f (%s)", id, variable.c_str(), date.toString().c_str(), value.v, units.c_str());
		return std::string(tmp);
	}
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
	YEARLY = 0, MONTHLY, DAILY, HOURLY, NONE
};

struct period_t {
	std::string name;
	agg_t aggregation;
	date_t start;
	date_t end;

	std::string toString() {
		char tmp[160];
		sprintf(tmp, "%s to %s", start.toString().c_str(), end.toString().c_str());
		return std::string(tmp);
	}
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
	bool f_logic_ok = false;
	bool f_overwrite_db = false;
	bool f_debug = false;
};

struct list_t {
	std::string name;
	std::vector<std::string> items;
};

struct quantile_t {
	double h;
	double l;
};

enum token_enum_t {
	TEXT, WRITE, DELETE, BL, NL,
	SETAGG, SETPER, SETVAR, SETMVR, SETFILE,
	MKDB, DELDB, COPYDB,
	PUSH, POP, SWAP, SIFT, __STACKWIPE,
	MACRO, MACROF, MACROS, EXECUTE, MACROLOOP, MACROTABLE,
	FIRSTDATE, LASTDATE, MEAN, MEDIAN, STDDEV, COEFVAR, 
	MANNKENDALLP, MANNKENDALLZ, MANNKENDALLH, MANNKENDALLT, MANNKENDALLTAU,
	THEILSENSLOPE,
	LINRA, LINRB, LINRR, POLYREG, EXPREG,
	MIN, MAX, RANGE, IQR, PERCENTILE, RPERCENTILE,
	CORR, PCORR,
	LINT, POLYINT, NEARINT,
	OCLIP,
	NORMALIZE,
	__LEGACY_INT, __LEGACY_EXECUTE_TOKEN, __LEGACY_EXECUTE_SCRIPT,
	
	__FILEv__, __VAR__, __MVR__, __AGG__, __PER__,
	__LITERAL__, __DB_REF__, __VAR_REF__, __TABLE_REF__,
	__EXTERN_REF__, 
	__INVALID__, __ERROR__, __COMMENT__, 

	NONE
};

struct token_t {
	token_enum_t token;
	std::vector<int> associated;
};

struct interpreter_memblock_t {
	// state
	int id;
	int state;
	script_flag_t flags;
	period_t active_period;
	agg_t active_aggregation;

	// files
	std::ifstream cfg;
	std::ofstream out;

	// vars
	std::string file;
	std::string agg;
	std::string var;
	std::string mvr;

	// databases
	std::vector<row_t> table; // top-level table (cvars build-in)
	std::vector<row_t> atable; // aggregated table
	std::vector<row_t> ltable; // local table

	// other
	std::vector<macro_t> macros;
	std::vector<list_t> lists;
	std::vector<std::string> variables;
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
#define F_STOP_AT_SCRIPTS 0

#define DAYS_PER_TYP_MONTH ((double) ((31 * 7) + (30 * 4) + 28.75) / 12)

#define GNUPLOT_PATH "C:\\Users\\holling\\Downloads\\gnuplot-53pl0w64\\gnuplot\\bin\\gnuplot.exe"

const date_t FIRST_DATE = { 0, 0, 0, 0, 0 };
const date_t LAST_DATE = { 9999, 99, 99, 99, 99 };

const std::string months[13] = { "N/A", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//int merge_duplicates_by_date(std::vector<row_t> &table);
bool execute_config(std::vector<std::string> &variables, std::vector<variable_t> &rules, std::vector<custom_var_t> &cvars, std::vector<std::string> &scripts);
bool apply_rules(std::vector<std::string> &apply_to, std::vector<std::string> &rules_list, std::vector<std::string> &variables, std::vector<variable_t> &rules);
