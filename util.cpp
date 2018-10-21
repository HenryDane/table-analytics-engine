#define _CRT_SECURE_NO_WARNINGS // look I like sprintf() ok?
#define NOMINMAX // use std not windows

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <iomanip>
#include "main.h"

std::vector<std::string> id_variables(std::vector<row_t> t) {
	std::vector<std::string> variables;
	for (unsigned int i = 0; i < t.size(); i++) {
		printf("Reading row %d \r", i);

		bool found = false;
		if (i > t.size()) i = 0;

		for (unsigned int j = 0; j < variables.size(); j++) {
			if (variables.at(j) == t.at(i).variable) {
				found = true;
				break;
			}
		}
		if (!found) {
			variables.push_back(t.at(i).variable);
		}
	}
	return variables;
}

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
[[deprecated]] int date_as_day(date_t date) {
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
		/*if (date_as_day(table.at(i).date) < date_as_day(date)) {*/
		if (table.at(i).date.numeric() < date.numeric()){
			date = table.at(i).date;
		}
	}

	return date;
}

date_t max_table_date(std::vector<row_t> &table) {
	date_t date = { 0000, 00, 00, 00, 00 }; // YYYY MM DD
	for (unsigned int i = 0; i < table.size(); i++) {
		/*if (date_as_day(table.at(i).date) > date_as_day(date)) {*/
		if (table.at(i).date.numeric() > date.numeric()){
			date = table.at(i).date;
		}
	}

	return date;
}

bool date_sort(row_t i, row_t j) {
	/*if (date_as_day(i.date) < date_as_day(j.date)) {*/
	if (i.date.numeric() < j.date.numeric()){
		return true;
	}
	return false;
}

bool date_sort_d(date_t i, date_t j) {
	/*if (date_as_day(i) < date_as_day(j)) {*/
	if (i.numeric() < j.numeric()) {
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
	/*if (date_as_day(i) > date_as_day(j)) {*/
	if (i.numeric() > j.numeric()) {
		return true;
	}
	return false;
}

void filter_by_variable(std::vector<row_t> table,
		std::vector<row_t> &otable,
		std::vector<std::string> varnames,
		std::string rename,
		bool merge,
		bool f_rename) {
	std::vector<row_t> ltable;
	otable.clear();

	for (unsigned int i = 0; i < table.size(); i++) {
		for (unsigned int j = 0; j < varnames.size(); j++) {
			if (table.at(i).variable == varnames.at(j)) {
				//if (strstr(table.at(i).variable.c_str(), varnames.at(j).c_str())){
				row_t row = table.at(i);

				if (f_rename && !rename.empty()) row.variable = rename;

				ltable.push_back(row);
			}
		}
	}

	//printf("collected %d records\n", ltable.size());

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
					ltable.at(i).value.f = (ltable.at(i).value.f || std::isnan(ltable.at(i).value.v)) ? true : false;
					ltable.at(j).value.f = (ltable.at(j).value.f || std::isnan(ltable.at(j).value.v)) ? true : false;
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
					ltable.at(i).value.f = (ltable.at(i).value.f || std::isnan(ltable.at(i).value.v)) ? true : false;
					ltable.at(j).value.f = (ltable.at(j).value.f || std::isnan(ltable.at(j).value.v)) ? true : false;
				}
			}
		}

		// fix averages (i think)
		for (unsigned int i = 0; i < ltable.size(); i++) {
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
	//return (date_as_day(date)) / DAYS_PER_TYP_MONTH;
	return date.numeric() / DAYS_PER_TYP_MONTH;
}

double date_as_year(date_t &date) {
	//return (date_as_day(date)) / 365;
	return date.numeric() / 365;
}

bool edit_sort(row_t i, row_t j) {
	if (i.edits < j.edits) {
		return true;
	}
	return false;
}

void print_table_c(std::vector<row_t> &t) {
	for (unsigned int i = 0; i < t.size(); i++) {
		printf("%s\n", t.at(i).toString().c_str());
	}
}

void reflag_table(std::vector<row_t> &t) {
	for (unsigned int i = 0; i < t.size(); i++)
		t[i].value.f = t[i].value.f || std::isnan(t[i].value.v);
}

bool date_in_period(date_t d, period_t p) {
	/*if (date_as_day(d) >= date_as_day(p.start) && date_as_day(d) <= date_as_day(p.end)) {*/
	if (d.numeric() >= p.start.numeric() && d.numeric() <= p.end.numeric()) {
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
	for (unsigned int i = 0; i < tablea.size(); i++) {
		for (unsigned int j = 0; j < tableb.size(); j++) {
			if (is_equal(tablea.at(i).date, tableb.at(j).date) &&
			   (tablea.at(i).value.f || tableb.at(j).value.f)) {
				tablea.at(i).id = -100;
				tableb.at(j).id = -100;
			}
		}
	}

	for (unsigned int i = 0; i < tablea.size(); i++) {
		if (tablea.at(i).id == -100) {
			tablea.erase(tablea.begin() + i);
			i = std::max((unsigned int) 0, i - 2); // go back to valid memory
		}
	}

	for (unsigned int i = 0; i < tableb.size(); i++) {
		if (tableb.at(i).id == -100) {
			tableb.erase(tableb.begin() + i);
			i = std::max((unsigned int) 0, i - 2); // go back to valid memory
		}
	}
	//printf("new: %d %d\n", tablea.size(), tableb.size());
}

void strip_null(std::vector<row_t> &a, std::vector<row_t> &b, std::vector<row_t> &c) {
	std::vector<date_t> forbidden_dates;

	// write
	for (unsigned int i = 0; i < a.size(); i++) {
		if (a[i].value.f || std::isnan(a[i].value.v)) {
			a[i].id = -10;
			forbidden_dates.push_back(a[i].date);
		}
	}
	//printf("F.D.: %d\n", forbidden_dates.size());
	for (unsigned int i = 0; i < b.size(); i++) {
		if (b[i].value.f || std::isnan(b[i].value.v)) {
			b[i].id = -10;
			forbidden_dates.push_back(b[i].date);
		}
	}
	//printf("F.D.: %d\n", forbidden_dates.size());
	for (unsigned int i = 0; i < c.size(); i++) {
		if (c[i].value.f || std::isnan(c[i].value.v)) {
			c[i].id = -10;
			forbidden_dates.push_back(c[i].date);
		}
	}
	//printf("F.D.: %d\n", forbidden_dates.size());

	// read
	for (unsigned int i = 0; i < a.size(); i++) {
		for (unsigned int j = 0; j < forbidden_dates.size(); j++) {
			if (a[i].date == forbidden_dates[j]) {
				a[i].id = -10;
			}
		}
	}
	for (unsigned int i = 0; i < b.size(); i++) {
		for (unsigned int j = 0; j < forbidden_dates.size(); j++) {
			if (b[i].date == forbidden_dates[j]) {
				b[i].id = -10;
			}
		}
	}
	for (unsigned int i = 0; i < c.size(); i++) {
		for (unsigned int j = 0; j < forbidden_dates.size(); j++) {
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
	for (unsigned int i = 0; i < a.size(); i++) {
		if (a[i].id > 0 && i > 0) {
			a.erase(a.begin(), a.begin() + i - 1);
			//printf("A: %d\n", i);
			break;
		}
	}
	for (unsigned int i = 0; i < b.size(); i++) {
		if (b[i].id > 0 && i > 0) {
			b.erase(b.begin(), b.begin() + i - 1);
			//printf("B: %d\n", i);
			break;
		}
	}
	for (unsigned int i = 0; i < c.size(); i++) {
		if (c[i].id > 0 && i > 0) {
			c.erase(c.begin(), c.begin() + i - 1);
			//printf("C: %d\n", i);
			break;
		}
	}
}

void null_shield(std::vector<row_t> &table) {
	std::vector<row_t> t = table;
	//printf("null_shield() -> inital: %d [%d]", t.size(), table.size());
	table.clear();
	for (unsigned int i = 0; i < t.size(); i++) {
		if (!std::isnan(t.at(i).value.v)) {
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
	a.f = (a.f || std::isnan(a.v)) ? true : false;
	b.f = (b.f || std::isnan(b.v)) ? true : false;

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

double clip_date(std::vector<row_t> &tablea, std::vector<row_t> &tableb, bool d) {
	d = false;
	// resolve NAN
	std::sort(tablea.begin(), tablea.end(), date_sort);
	std::sort(tableb.begin(), tableb.end(), date_sort);

	// delete rows which contain 1 or more null
	strip_null(tablea, tableb);

	std::sort(tablea.begin(), tablea.end(), date_sort);
	std::sort(tableb.begin(), tableb.end(), date_sort);

	if (d) {
		printf("ORIGINAL: A(%s -> %s) B(%s -> %s)\n",
			date_toString(tablea.at(0).date).c_str(),
			date_toString(tablea.at(tablea.size() - 1).date).c_str(),
			date_toString(tableb.at(0).date).c_str(),
			date_toString(tableb.at(tableb.size() - 1).date).c_str());
	}

	date_t startdate = max_date(tablea.at(0).date, tableb.at(0).date);
	date_t enddate = min_date(tablea.at(tablea.size() - 1).date, tableb.at(tableb.size() - 1).date);

	// clip start date
	for (unsigned int idx = 0; idx < tablea.size(); idx++) {
		if (tablea.at(idx).date <= startdate /*is_equal(tablea.at(idx).date, startdate)*/) {
			tablea.erase(tablea.begin(), tablea.begin() + idx);
			if (tablea.size() == 0) return INFINITY;
			idx = std::min((unsigned int)0, idx - 1);
		}
	}
	for (unsigned int idx = 0; idx < tableb.size(); idx++) {
		if (tableb.at(idx).date <= startdate /*is_equal(tableb.at(idx).date, startdate)*/) {
			tableb.erase(tableb.begin(), tableb.begin() + idx);
			if (tableb.size() == 0) return INFINITY;
			idx = std::min((unsigned int)0, idx - 1);
		}
	}
	// clip end date
	for (unsigned int idx = 0; idx < tablea.size(); idx++) {
		if (tablea.at(idx).date >= enddate/*is_equal(tablea.at(idx).date, enddate)*/) {
			//if (idx + 1 >= tablea.size()) break;
			tablea.erase(tablea.begin() + idx, tablea.end());
			if (tablea.size() == 0) return INFINITY;
			break;
		}
	}
	for (unsigned int idx = 0; idx < tableb.size(); idx++) {
		if (tableb.at(idx).date >= enddate/*is_equal(tableb.at(idx).date, enddate)*/) {
			//if (idx + 1 >= tableb.size()) break;
			tableb.erase(tableb.begin() + idx, tableb.end());
			if (tableb.size() == 0) return INFINITY;
			break;
		}
	}

	if (d) {
		printf("FINAL: A(%s -> %s) B(%s -> %s)\n",
			date_toString(tablea.at(0).date).c_str(), date_toString(tablea.at(tablea.size() - 1).date).c_str(),
			date_toString(tableb.at(0).date).c_str(), date_toString(tableb.at(tableb.size() - 1).date).c_str());
	}

	return 0;
}

void gen_plot(std::vector<row_t> t) {
int randint = rand() % 10000;

// generate tmp file
std::ofstream f;
f.open(t.at(0).variable + std::to_string(randint) + ".dat");
if (!f.is_open()) {
	printf("could not open\n");
	return;
}

// write tmp data
f << "# tmp_plot.dat" << std::endl;
for (unsigned int i = 0; i < t.size(); i++) {
	if (!t[i].value.f) {
		f << std::setprecision(std::numeric_limits<long double>::digits10);
		f << t[i].date.toString() << " " << t[i].value.v << " " << std::endl;
	}
	else {
		f << std::endl;
	}
}
f << std::endl;
f.close();

// generate tmp plot file
f.open(t.at(0).variable + std::to_string(randint) + ".plot");
if (!f.is_open()) {
	printf("could not open\n");
	return;
}

// write tmp plot file
f << "# set terminal svg size 1000 1000 dynamic" << std::endl;
f << "# set output '" << t.at(0).variable << std::to_string(randint) << ".svg'" << std::endl;
f << "set style line 1 \\" << std::endl;
f << "    linecolor rgb '#0060ad' \\" << std::endl;
f << "    linetype 1 linewidth 2 \\" << std::endl;
f << "    pointtype 7 pointsize .3" << std::endl;
f << std::endl;
f << "set xdata time" << std::endl;
f << "set timefmt '%m/%d/%Y %H:%M'" << std::endl;
f << "set format x '%m/%d/%Y %H:%M'" << std::endl;
f << "set xrange ['" << min_table_date(t).toString() << "':'" << max_table_date(t).toString() << "']" << std::endl;
f << std::endl;
f << "set title \"" << t.at(0).variable << "\"" << std::endl;
f << std::endl;
f << "plot '" << t.at(0).variable << std::to_string(randint) << ".dat' using 1:3 with linespoints linestyle 1" << std::endl;
//f << "pause -1 \"Press [Return] to continue\"" << std::endl;
f << "pause mouse" << std::endl;
f << std::endl;
f.close();

// call gnuplot
std::string p = std::string(GNUPLOT_PATH) + " " + t.at(0).variable + std::to_string(randint) + ".plot";
system(p.c_str());

// clean up files
std::string s = t.at(0).variable + std::to_string(randint) + ".plot";
remove(s.c_str());
s = t.at(0).variable + std::to_string(randint) + ".dat";
remove(s.c_str());
}

/*
void build_custom_vars(std::vector<row_t> &t, std::vector<custom_var_t> cvars) {
	std::vector<row_t> result;

	for (custom_var_t cvt : cvars) {
		std::vector<row_t> cvar_table; //holds unaggregated pieces

		for (row_t r : t)
			if (is_inside(cvt.pieces, r.variable))
				cvar_table.push_back(r);


		for (row_t r : cvar_table) {
			r.variable = cvt.name;

			if (r.id < 0) continue;

			for (row_t s : cvar_table) {
				if (s.id < 0) continue;

				if (s.date == r.date) {
					s.id = -100;
					r.value = safe_add(r.value, s.value);
					r.edits++;
				}
			}

			r.value.v = r.value.v / r.edits;
			result.push_back(r);
		}
		printf("SIZE -> %d\n", result.size());
	}

	for (row_t r : result)
		t.push_back(r);
}*/

bool name_pair_sort(name_pair_t left, name_pair_t right) {
	return left.name.compare(right.name);
}

void print_wide_table(std::vector<row_t> table){
	std::ofstream ooo("ooooo.csv");

	// list of all dates in the list
	std::vector<date_t> dates;

	for (row_t r : table) {
		if (r.date.year < 1900) continue;
		bool found = false;
		for (date_t d : dates) {
			if (d == r.date) {
				found = true;
				break;
			}
		}
		if (!found) {
			dates.push_back(r.date);
		}
	}

	printf("SIZE -> %d\n", dates.size());

	std::vector<wide_table_t> wtable;

	int idx = 0;
	for (date_t d : dates) {
		if (d.year < 1900) continue;
		wide_table_t wt;
		wt.id = idx++;
		wt.date = d;

		for (row_t r : table) {
			if (/*r.date == d*/ r.date.year == d.year &&
				r.date.month == d.month &&
				r.date.day == d.day) {
				// row is correct for this day, is it already in there for some reason?
				bool found = false;
				for (name_pair_t np : wt.data) {
					if (np.name == r.variable) {
						found = true;
					}
				}

				// its not in the list already
				if (!found) {
					name_pair_t npt;
					npt.name = r.variable;
					npt.value = r.value.v;
					wt.data.push_back(npt);
				}
			}
		}

		wtable.push_back(wt);
	}

	for (unsigned int i = 0; i < wtable.size(); i++) {
		std::sort(wtable[i].data.begin(), wtable[i].data.end(), name_pair_sort);
	}

	ooo << "DATE,";
	for (name_pair_t npt : wtable[0].data) {
		ooo << npt.name << ",";
	}
	ooo << std::endl;

	for (unsigned int i = 0; i < wtable.size(); i++) {
		ooo << wtable[i].date.toString() << ",";
		for (name_pair_t npt : wtable[0].data) {
			ooo << (std::isnan(npt.value) ? std::string("NAN") : std::to_string(npt.value)) << ",";
		}
		ooo << std::endl;
	}


	ooo.close();
}
