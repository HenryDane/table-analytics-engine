#define _CRT_SECURE_NO_WARNINGS // look I like sprintf() ok?
#define NOMINMAX // use std not windows

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <windows.h>
#include "main.h"

bool is_inside(std::vector<std::string> vec, std::string str) {
	for (unsigned int i = 0; i < vec.size(); i++) {
		if (str.compare(vec.at(i)) == 0) {
			return true;
		}
	}
	return false;
}

double min_table(std::vector<row_t> &table) {
	double min_val = INFINITY;
	for (unsigned int i = 0; i < table.size(); i++) {
		if (!table.at(i).value.f) min_val = std::min(min_val, table.at(i).value.v);
	}
	return min_val;
}

double max_table(std::vector<row_t> &table) {
	double max_val = -INFINITY;
	for (unsigned int i = 0; i < table.size(); i++) {
		if (!table.at(i).value.f) max_val = std::max(max_val, table.at(i).value.v);
	}
	return max_val;
}

// returns days since 00/00/0000 as integer with leap years
int date_as_day(date_t date) {
	int day_sum = 0;
	for (int i = 1; i < date.month; i++) {
		int days_month = 0;
		if (i == 9 || i == 4 || i == 6 || i == 11) {
			// sept/aprl/june/novm
			days_month = 30;
		}
		else if (i == 2) {
			if (abs((date.year - 1600)) % 4 == 0) {
				// leap year
				days_month = 29;
			} else {
				days_month = 28;
			}
		} else {
			days_month = 31;
		}

		day_sum += days_month;
	}

	day_sum += date.year * 365;
	day_sum += date.day;
	return day_sum;
}

date_t min_table_date(std::vector<row_t> &table) {
	date_t date = { 9999, 99, 99, 99, 99 }; // YYYY MM DD
	for (unsigned int i = 0; i < table.size(); i++) {
		if (date_as_day(table.at(i).date) < date_as_day(date)) {
			date = table.at(i).date;
		}
	}

	return date;
}

date_t max_table_date(std::vector<row_t> &table) {
	date_t date = { 0000, 00, 00, 00, 00 }; // YYYY MM DD
	for (unsigned int i = 0; i < table.size(); i++) {
		if (date_as_day(table.at(i).date) > date_as_day(date)) {
			date = table.at(i).date;
		}
	}

	return date;
}

bool date_sort(row_t i, row_t j) {
	if (date_as_day(i.date) < date_as_day(j.date)) {
		return true;
	}
	return false;
	/*if (i.date.year < j.date.year) {
		return true;
	}
	else if (i.date.year > j.date.year) {
		return false;
	}
	else {
		if (i.date.month < j.date.month) {
			return true;
		}
		else if (i.date.month < j.date.month) {
			return false;
		}
		else {
			if (i.date.day < j.date.day) {
				return true;
			}
			else if (i.date.day > j.date.day) {
				return false;
			}
			else {
				return true;
			}
		}
	} */
}

bool date_sort_d(date_t i, date_t j) {
	if (date_as_day(i) < date_as_day(j)) {
		return true;
	}
	return false;
}

bool table_id_sort(row_t i, row_t j) {
	if (i.id < j.id) {
		return true;
	}
	return false;
}

bool date_sort_dr(date_t i, date_t j) {
	if (date_as_day(i) > date_as_day(j)) {
		return true;
	}
	return false;
}

void filter_by_variable(std::vector<row_t> table, std::vector<row_t> &otable, std::vector<std::string> varnames, bool merge) {
	std::vector<row_t> ltable;
	otable.clear();

	for (unsigned int i = 0; i < table.size(); i++) {
		for (unsigned int j = 0; j < varnames.size(); j++) {
			if (table.at(i).variable == varnames.at(j)) {
				//if (strstr(table.at(i).variable.c_str(), varnames.at(j).c_str())){
				row_t row = table.at(i);
				ltable.push_back(row);
			}
		}
	}

	if (merge) {
		for (unsigned int i = 0; i < ltable.size() - 1; i++) {
			for (unsigned int j = i; j < ltable.size(); j++) {
				if (i == j) {
					continue;
				}

				// skip bad rows
				if (table.at(i).id == -100 || table.at(j).id == -100) {
					continue;
				}

				if (ltable.at(i).date.day == ltable.at(j).date.day &&
					ltable.at(i).date.month == ltable.at(j).date.month &&
					ltable.at(i).date.year == ltable.at(j).date.year) {
					ltable.at(i).value.f = (ltable.at(i).value.f || isnan(ltable.at(i).value.v)) ? true : false;
					ltable.at(j).value.f = (ltable.at(j).value.f || isnan(ltable.at(j).value.v)) ? true : false;
					if (ltable.at(i).value.f) {
						ltable.at(i) = ltable.at(j);
						// dont incr tho because "nothing happened"
					} else if (ltable.at(j).value.f) {
						// do nothing bc j == NAN && i != NAN
					} else if (ltable.at(j).value.f && ltable.at(i).value.f) {
						// both NAN
						// do nothing
					} else {
						ltable.at(i).value.v += ltable.at(j).value.v;
						ltable.at(i).edits++; // to track divisor in avg
					}
					
					// mark J as bad
					ltable.at(j).id = -100;
					ltable.at(j).value.v = 0;

					// redo the flags
					ltable.at(i).value.f = (ltable.at(i).value.f || isnan(ltable.at(i).value.v)) ? true : false;
					ltable.at(j).value.f = (ltable.at(j).value.f || isnan(ltable.at(j).value.v)) ? true : false;
				}
			}
		}

		// fix averages (i think)
		for (int i = 0; i < ltable.size(); i++) {
			ltable.at(i).value.v = ltable.at(i).value.v / ltable.at(i).edits;
		}

		otable.reserve(ltable.size());
		for (unsigned int i = 0; i < ltable.size(); i++) {
			if (ltable.at(i).id != -100) {
				row_t r = ltable.at(i);
				otable.push_back(r);
			}
		}
	} else {
		otable.reserve(ltable.size());
		otable = ltable;
	}
#if F_MAKE_FILTER_FILES == 1
	std::ofstream file;
	std::string name = "filter_" + std::to_string(rand()) + ".txt";
	file.open(name);
	if (!file.is_open()) {
		printf("failed to write to [%s]\n", name.c_str());
		return;
	}
#endif 

	try {
		std::sort(otable.begin(), otable.end(), date_sort);
	}
	catch (const std::exception &e) {
		//std::cout << e.what() << std::endl;
		printf("%s\n", e.what());
	}

#if F_MAKE_FILTER_FILES == 1
	for (unsigned int i = 0; i < otable.size(); i++) {
		file << otable.at(i).id << " ";
		file << otable.at(i).date.month << " ";
		file << otable.at(i).date.day << " ";
		file << otable.at(i).date.year << " ";
		file << otable.at(i).variable << " ";
		file << otable.at(i).value << " ";
		file << std::endl;
	}

	file.close();
#endif
	return;
}

bool is_equal(date_t &a, date_t &b) {
	if (a.day == b.day) {
		if (a.month == b.month) {
			if (a.year == b.year) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

// ignores time comparisons
date_t min_date(date_t &a, date_t &b) {
	if (a.year < b.year) {
		return a;
	}
	else if (b.year < a.year) {
		return b;
	}
	else {
		if (a.month < b.month) {
			return a;
		}
		else if (b.month < a.month) {
			return b;
		}
		else {
			if (a.day < b.day) {
				return a;
			}
			else if (b.day < a.day) {
				return b;
			}
			else {
				return a;
			}
		}
	}
}

date_t max_date(date_t &a, date_t &b) {
	if (a.year > b.year) {
		return a;
	}
	else if (b.year > a.year) {
		return b;
	}
	else {
		if (a.month > b.month) {
			return a;
		}
		else if (b.month > a.month) {
			return b;
		}
		else {
			if (a.day > b.day) {
				return a;
			}
			else if (b.day > a.day) {
				return b;
			}
			else {
				return a;
			}
		}
	}
}

/*
removes corresponding dates from table by searching through list of bad dates from
bad rows selected by varname
*/
int reduce_by_var(std::vector<row_t> table, std::vector<row_t> bad_rows, std::string varname) {
	int edits = 0;

	for (unsigned int i = 0; i < table.size(); i++) {
		bool found = false;

		for (unsigned int j = 0; j < bad_rows.size(); j++) {
			if (is_equal(bad_rows.at(j).date, table.at(i).date) && table.at(i).variable == varname) {
				found = true;
			}
		}

		if (found) {
			edits++;
			table.erase(table.begin() + i);
		}
	}

	return edits;
}

void make_unique(std::vector<std::string> &vec) {
	sort(vec.begin(), vec.end());
	vec.erase(unique(vec.begin(), vec.end()), vec.end());
}

bool startsWith(std::string mainStr, std::string toMatch) {
	// std::string::find returns 0 if toMatch is found at starting
	if (mainStr.find(toMatch) == 0)
		return true;
	else
		return false;
}

std::string date_toString(date_t &a) {
	char tmp[80];
	sprintf(tmp, "%02d/%02d/%04d %02d:%02d", a.month, a.day, a.year, a.hours, a.minuites);
	return std::string(tmp);
}


double date_as_month(date_t &date) {
	return (date_as_day(date)) / DAYS_PER_TYP_MONTH;
}

double date_as_year(date_t &date) {
	return (date_as_day(date)) / 365;
}

bool edit_sort(row_t i, row_t j) {
	if (i.edits < j.edits) {
		return true;
	}
	return false;
}

/*
 does not work
*/
void aggregate_table(std::vector<row_t> &table, agg_t agg) {
	int idx = 0;
	std::ofstream ofs;

	int cycle = 0;
	switch (agg) {
	case YEARLY:
		std::sort(table.begin(), table.end(), date_sort);
		for (int i = 0; i < table.size(); i++) {
			for (int j = i; j < table.size(); j++) {
				printf("%d \r", cycle);
				if (table.at(j).edits < 0 || table.at(i).edits  < 0) continue;

				if (table.at(i).date.year == table.at(j).date.year && table.at(i).variable == table.at(j).variable) {
					if (table.at(i).value.f) {
						if (table.at(j).value.f) {
							table.at(i).value.v = NAN;
						} else {
							table.at(i).value.v = table.at(j).value.v;
							table.at(i).value.f = false; // reset flag
						}
					} else {
						if (!table.at(j).value.f) {
							table.at(i).value.v += table.at(j).value.v;
						}
					}
					table.at(i).edits++;
					table.at(i).edits += (table.at(j).edits > 0) ? table.at(j).edits : 0;
					table.at(j).edits = -100; // flag as done
					cycle++;
					//i = 0; j = 0;
				}
			}
		}

		printf("sort\n");
		std::sort(table.begin(), table.end(), edit_sort);
		for (; idx < table.size(); idx++) {
			if (table.at(idx).edits > -100) {
				break;
			}
		}
		printf("%d %d\n", idx, table.size());
		//table.erase(table.begin(), table.begin() + idx);

		std::sort(table.begin(), table.end(), date_sort);

		ofs.open("m.txt", std::ios::trunc);
		if (!ofs.is_open()) {
			printf("err\n");
		}
		for (int i = 0; i < table.size(); i++) {
			ofs << table.at(i).id << " " << date_toString(table.at(i).date) << " " << table.at(i).value.v << " " << table.at(i).edits << std::endl;
		}
		ofs.close();
		return;
	default:
		printf("undefined aggregation\n");
		return;
	}
}

bool date_in_period(date_t d, period_t p) {
	if (date_as_day(d) >= date_as_day(p.start) && date_as_day(d) <= date_as_day(p.end)) {
		return true;
	} else {
		return false;
	}
}

int DeleteDirectory(const std::wstring &refcstrRootDirectory,
	bool              bDeleteSubdirectories = true)
{
	bool            bSubdirectory = false;       // Flag, indicating whether
												 // subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	std::wstring     strFilePath;                 // Filepath
	std::wstring     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = refcstrRootDirectory + L"\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
					else
						bSubdirectory = true;
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributes(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFile(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bSubdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributes(refcstrRootDirectory.c_str(),
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
					return ::GetLastError();
			}
		}
	}

	return 0;
}

bool table_value_sort(row_t i, row_t j) {
	if (i.value.v < j.value.v) {
		return true;
	}

	return false;
}

void strip_null(std::vector<row_t> &tablea, std::vector<row_t> &tableb) {
	// printf("old: %d %d ", tablea.size(), tableb.size());
	for (int i = 0; i < tablea.size(); i++) {
		for (int j = 0; j < tableb.size(); j++) {
			if (is_equal(tablea.at(i).date, tableb.at(j).date) &&
			   (tablea.at(i).value.f || tableb.at(j).value.f)) {
				tablea.at(i).id = -100;
				tableb.at(j).id = -100;
			}
		}
	}

	for (int i = 0; i < tablea.size(); i++) {
		if (tablea.at(i).id == -100) {
			tablea.erase(tablea.begin() + i);
			i = std::max(0, i - 2); // go back to valid memory
		}
	}

	for (int i = 0; i < tableb.size(); i++) {
		if (tableb.at(i).id == -100) {
			tableb.erase(tableb.begin() + i);
			i = std::max(0, i - 2); // go back to valid memory
		}
	}
	//printf("new: %d %d\n", tablea.size(), tableb.size());
}

void strip_null(std::vector<row_t> &a, std::vector<row_t> &b, std::vector<row_t> &c) {
	std::vector<date_t> forbidden_dates;

	// write
	for (int i = 0; i < a.size(); i++) {
		if (a[i].value.f || isnan(a[i].value.v)) {
			a[i].id = -10;
			forbidden_dates.push_back(a[i].date);
		}
	}
	printf("F.D.: %d\n", forbidden_dates.size());
	for (int i = 0; i < b.size(); i++) {
		if (b[i].value.f || isnan(b[i].value.v)) {
			b[i].id = -10;
			forbidden_dates.push_back(b[i].date);
		}
	}
	printf("F.D.: %d\n", forbidden_dates.size());
	for (int i = 0; i < c.size(); i++) {
		if (c[i].value.f || isnan(c[i].value.v)) {
			c[i].id = -10;
			forbidden_dates.push_back(c[i].date);
		}
	}
	printf("F.D.: %d\n", forbidden_dates.size());

	// read 
	for (int i = 0; i < a.size(); i++) {
		for (int j = 0; j < forbidden_dates.size(); j++) {
			if (a[i].date == forbidden_dates[j]) {
				a[i].id = -10;
			}
		}
	}
	for (int i = 0; i < b.size(); i++) {
		for (int j = 0; j < forbidden_dates.size(); j++) {
			if (b[i].date == forbidden_dates[j]) {
				b[i].id = -10;
			}
		}
	}
	for (int i = 0; i < c.size(); i++) {
		for (int j = 0; j < forbidden_dates.size(); j++) {
			if (c[i].date == forbidden_dates[j]) {
				c[i].id = -10;
			}
		}
	}

	// sort
	std::sort(a.begin(), a.end(), table_id_sort);
	std::sort(b.begin(), b.end(), table_id_sort);
	std::sort(c.begin(), c.end(), table_id_sort);

	// erase
	for (int i = 0; i < a.size(); i++) {
		if (a[i].id > 0 && i > 0) {
			a.erase(a.begin(), a.begin() + i - 1);
			printf("A: %d\n", i);
			break;
		}
	}
	for (int i = 0; i < b.size(); i++) {
		if (b[i].id > 0 && i > 0) {
			b.erase(b.begin(), b.begin() + i - 1);
			printf("B: %d\n", i);
			break;
		}
	}
	for (int i = 0; i < c.size(); i++) {
		if (c[i].id > 0 && i > 0) {
			c.erase(c.begin(), c.begin() + i - 1);
			printf("C: %d\n", i);
			break;
		}
	}
}

void null_shield(std::vector<row_t> &table) {
	std::vector<row_t> t = table;
	//printf("null_shield() -> inital: %d [%d]", t.size(), table.size());
	table.clear();
	for (int i = 0; i < t.size(); i++) {
		if (!isnan(t.at(i).value.v)) {
			table.push_back(t.at(i));
		}
	}
	//printf("final: %d\n", table.size());
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}


void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

// ret a + b but prevets nan seepage
s_double_t safe_add(s_double_t &a, s_double_t &b) {
	s_double_t r;

	// update
	a.f = (a.f || isnan(a.v)) ? true : false;
	b.f = (b.f || isnan(b.v)) ? true : false;

	if (a.f && (!b.f)) {
		r.v = b.v;
		r.f = false;
	} else if ((!a.f) && b.f) {
		r.v = a.v;
		r.f = false;
	} else if ((!a.f) && (!b.f)) {
		r.v = a.v + b.v;
		r.f = false;
	} else {
		r.v = NAN;
		r.f = true;
	}

	return r;
}