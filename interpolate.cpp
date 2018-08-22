#include <vector>
#include <algorithm>
#include "main.h"
#include "util.h"

/*
	garbage, dont use
*/
void linear_interpolate(std::vector<row_t> &t) {
	std::sort(t.begin(), t.end(), date_sort);

	if (t.size() < 4) return;

	for (int i = 0; i < t.size(); i++) {
		int offset_a = 1;
		int offset_b = 1;
		int offset_c = 2;

		if (t[i].value.f || t[i].value.i) {
			double m = 0.0;

			if (i + 1 < t.size()) {
				if (t[i + offset_a].value.f) {
					offset_a++;
				}
				m = (t[i + offset_a].value.v - t[i - offset_b].value.v) / (date_as_day(t[i + offset_a].date) - date_as_day(t[i - offset_b].date));
			}
			else {
				m = (t[i - offset_b].value.v - t[i - offset_c].value.v) / (date_as_day(t[i - offset_b].date) - date_as_day(t[i - offset_c].date));
			}

			t[i].value.v = t[i - 1].value.v + (m * (date_as_day(t[i].date) - date_as_day(t[i - 1].date)));
			t[i].value.f = isnan(t[i].value.v);

		}
	}
}