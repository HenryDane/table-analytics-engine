#pragma once
#include <vector>
#include <string>
#include "main.h"

// interpreter_memblock_t functions
/*
	sets imb into a defined state and/or resets an already configured imb
*/
bool imb_init(interpreter_memblock_t &imb);

/*
	rebuilds variable inside imb from table;
*/
bool imb_update_var_table(interpreter_memblock_t &imb);

/*
	sets up internal table of imb and creates cvar tables which are also saved to the table
	exact prodecure is:
	1. clear table
	2. copy arg(table) -> table
	3. update imb variables
	4. for each cvar in cvars create temp table with all subcomponents
	5. merge each subtable into table
*/
bool imb_build_table(interpreter_memblock_t &imb,
	std::vector<row_t> table,
	std::vector<custom_var_t> cvars);

/*
	resets current aggregation inside of imb and reaggregates table by :
	1. clear atable and ltable;
	2. copy table -> atable while filtering with period p
	3. aggregate atable via agg
	4. rebuild ltable from var
*/
bool imb_build_agg_table(interpreter_memblock_t &imb,
	agg_t agg);

/*
	resets current period of atable by:
	1. clear atable and ltable;
	2. copy table -> atable while filtering with period p
	3. aggregate atable via agg
	4. rebuild ltable from var
*/
bool imb_set_period(interpreter_memblock_t &imb, period_t p);

/*
	sets current variable of imb and then rebuilds ltable from atable
*/
bool imb_set_var(interpreter_memblock_t &imb, std::string var);

/*
	executes token on the data inside imb
*/
bool imb_enumerate_token(interpreter_memblock_t &imb, std::string token);