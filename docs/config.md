# `out.cfg` Formatting Guide

## Mode settings
The first thing after the `# MODE` marker must be the number of settings.\
`UNDERSCORE_IS_SPACE` replaces all underscores with spaces when writing\
`COLORS_OK` allows color code to be written (ignored if XML_EXCEL is not format code)\
`AUTO_RESET_FILTERS` resets filters automatically

## Begin settings
`CSV` declares that the output is a CSV file\
`XM_EXCEL` declares that the output is an Excel file described in XML\
`PLAIN` declares that the output is a plaintext file (BL does nothing)

## Out codes
`STOP` signals to stop scanning\
`NL` Injects newline to output\
`BL` Signals a blank space

`SYS:OUT:__VAR__:{Value}` Sets the __VAR__ variable to {Value}

`SYS:FILTER:RESET` Resets all filters\
`SYS:FILTER:PERIOD:{Value}` Sets and filters the data according to a predefined period {Value}\
`SYS:FILTER:VALUE:GREATER:{Value}` Sets and filters the data, keeping all rows with value above {Value}\
`SYS:FILTER:VALUE:LESS:{Value}` Sets and filters the data, keeping all rows with value below {Value}\
`SYS:FILTER:VALUE:EQUAL:{Value}` Sets and filters the data, keeping all rows with value equal to {Value}\
`SYS:FILTER:VALUE:NOT:{Value}` Sets and filters the data, keeping all rows with value not equal to {Value}

`SYS:AGG:{Value}` Aggregates data to the level described by {Value} (Yearly, Monthly, Daily, None)

`TEXT:{Value}` injects {Value} (no spaces allowed) into the output

`VARS:{Var Name}:INFO` (the info object)\
`VARS:{Var Name}:INFO:InitialDate` returns the smallest date in {Var Name}\
`VARS:{Var Name}:INFO:FinalDate` returns the largest date in {Var Name}\
`VARS:{Var Name}:INFO:Mean` returns the mean of {Var Name}\
`VARS:{Var Name}:INFO:Median` returns the mean of {Var Name}\
`VARS:{Var Name}:INFO:StdDev` returns the mean of {Var Name}\
`VARS:{Var Name}:INFO:CV` returns the mean of {Var Name}

`VARS:{Var Name}:TESTS` (test package)\
`VARS:{Var Name}:TESTS:MK` (mann-kendall test package)\
`VARS:{Var Name}:TESTS:MK:z` z component of MK test\
`VARS:{Var Name}:TESTS:MK:p` p component of MK test\
`VARS:{Var Name}:TESTS:MK:h` h component of MK test\
`VARS:{Var Name}:TESTS:MK:trend` trend result (string) of MK test\
`VARS:{Var Name}:TESTS:TS` (theil-sen test package)\
`VARS:{Var Name}:TESTS:TS:Slope` returns the slope\
`VARS:{Var Name}:TESTS:LINREG` (linear regression package)\
`VARS:{Var Name}:TESTS:LINREG:A` the slope component of the linear regression package\
`VARS:{Var Name}:TESTS:LINREG:B` the y-intecept componenent of the linear regression package
