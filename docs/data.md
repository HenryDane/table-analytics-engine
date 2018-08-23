# Data formatting guide

## Format
The data presented should be formatted into eight columns:
- `Date` This column holds the date stamp for the row. This must be formatted either as `mm/dd/yyyy` or as `mm/dd/yy`. 
- `Time` This holds the time stamp for the row. This must be formatted as `hh:mm`.
- `Month` This is an integer value which holds the numeric value corresponding with the month component of the `Date` field. The valid range is 1 to 12, but other values will be tolerated.
- `Year` This is an integer value which holds the numeric value of the year corresponding with the year component of the `Date` field.
- `Var` This is a string. It must not be preceeded with `__`, as values enclosed with `__` are reserved by the system. 
- `Value` This is a floating point number, or `N/A` or `--`.
- `Units` This is a string.
- `State` This is a special integer. This must be present. This must be an integer.

### States
- `0` : see function `resolve_table_state()` in `data.cpp`.
- `65` and `17` : ignored (see above).