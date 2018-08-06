#pragma once
#include <vector>
#include "main.h"
#include "util.h"

void resolve_table_state(std::vector<row_t> &table);
int old_table_agg(std::vector<row_t> &table);
void period_filter(std::vector<row_t> &table, period_t period);
bool degap_table_monthly(std::vector<row_t> &table);
int read_db(std::ifstream &data, std::vector<row_t> &table);