# `out.cfg` Formatting Guide
If something is preceed or associated with `!` it means that said item is unimplemented. Likewise `NT` means that said item is untested. Furthermore, `!!` means that said item is probably going to be removed. `P` means that the item is only partially done. `I` means that the item is buggy. `D` means that the item is deprecated.

## Notes
A simple example program would be the following:
```
~ CONFIG
# MODE 1 UNDERSCORE_IS_SPACE
# BEGIN outfile.txt
TEXT:Hello_World
STOP
```
- The `~ CONFIG` component must be present. 
- The strings `__VAR__`, `__AGG__`, `__MVR__`, `__FILE__`, `__GLOBAL__`, `__AT__`, and `__LIST_C__` can not be printed. 
- Spaces in strings should be replaced with underscores, and the `UNDERSCORE_IS_SPACE` directive should be used. 
- `__NULL__` and `__NAN__` are also reserved but may be printed. They can not be printed *literaly* however. 

### Reserved words
I `__VAR__` refers to the value of the global variable variable.\
! `__AGG__` refers to the value of the global aggregation variable (actually an enumeration).\
I `__MVR__` refers to the value of the global macro variable (an alternative to `__VAR__`). \
! `__FILE__` refers to the name of the currently open file. Changing this closes the current file and opens/creates the new filename.\
NT P `__GLOBAL__` lets certian functions (`DBLOOP` and `DBCLEAN`) know that they should operate on the entire database rather than on just one. \
! `__AT__` refers to the value of the row that the current aggregator is at. Only used with `DBLOOP` and `DBCLEAN`. \
`__LIST_C__` refers to the contents of a list.\
! `__NULL__` refers to a row that does not exist.\
`__NAN__` refers to the value `NaN`.\
! `__INF__` refers to the value `INFINITY`. 

## Mode settings
The first thing after the `# MODE` marker must be the number of settings.\
`UNDERSCORE_IS_SPACE` replaces all underscores with spaces when writing.\
! `COLORS_OK` allows color code to be written (ignored if `XML_EXCEL` is not format code).\
I D !! `AUTO_RESET_FILTERS` resets filters automatically.\
! `LOGIC_OK` signals that logic commands are to be used. Unless `OVERWRITE_DB_OK` is also present, this will result in a backup being created.\
! `OVERWRITE_DB_OK` signals that backups are not to be used, and that the database should be reopened in read-write mode.\
`DEBUG_ON` signals to print out debug information.

## Begin settings
Not yet implemented -- defaults to CSV output only.
! `CSV` declares that the output is a CSV file.\
! `XML_EXCEL` declares that the output is an Excel file described in XML.\
! `PLAIN` declares that the output is a plaintext file (`BL` does nothing)

## Instructions

### State control 
`STOP` signals to stop scanning and end interpretation.\
`EMACRO` signals to stop recording a macro.\
I !! `BMACRO` a comment-like token which signals that a macro has begun. This is only for readability, and is not actually necessary.\
`ELIST` signals to stop recording a list.

### Text formatting
`NL` Injects newline to output.\
`BL` Injects a blank (cell, space, etc) to output.

### Variable adjustment
`SYS:OUT:__VAR__:{Value}` Sets the `__VAR__` variable to {Value}.\
`SYS:OUT:__MVR__:{Vaule}` Sets the `__MVR__` variable to {Value}.\
`SYS:VAR:PUSH:{Reserved Varible}` Pushes either `__VAR__`, `__MVR__`, `__AGG__`, or `__FILE__` onto its respective stack.\
`SYS:VAR:POP:{Reserved Varible}` Pops either `__VAR__`, `__MVR__`, `__AGG__`, or `__FILE__` off of its respective stack.\
`SYS:OUT:__FILE__:{Value}` Switches the output file to {Value}. (Uses append)\
`SYS:SELF:RESET` Resets all internal lists, registries, macros, etc.\
`SYS:SELF:CRASH` Crashes the interpreter. \
`SYS:SELF:LOG:{Text}` Logs {Text} to interpreter console.
! `SYS:SELF:LOGF:{printf() syntax}` Prints {printf() syntax} to console.

### Registry 
`SYS:REGISTRY:PERIOD:{name}:{Y}:{M}:{D}:{Y}:{M}:{D}` defines a period.\
`SYS:REGISTRY:LIST:CREATE:{Name}` defines a list (strings) named {Name}.\
NT `SYS:REGISTRY:LIST:ADD:{Name}:{Item}` adds item {Item} to the list {Name}.\
NT `SYS:REGISTRY:LIST:MERGE:{Name 1}:{Name 2}` concatenates the two lists into the first.\
`SYS:REGISTRY:LIST:SCAN:{Name}` reads items into a list until `ELIST` is encountered.\
!! `SYS:REGISTRY:LIST:FCVAR:{Name}:{Var Name}` reads items from custom var def into {Name}.\
`SYS:REGISTRY:LIST:FGVAR:{Name}` copies the unique variable list into {Name}.

### Filter 
`SYS:FILTER:RESET` Resets all filters to raw variable.
`SYS:FILTER:PERIOD:{Value}` Sets and filters the data according to a predefined period {Value}.\
! `SYS:FILTER:VALUE:GREATER:{Value}` Sets and filters the data, keeping all rows with value above {Value}.\
! `SYS:FILTER:VALUE:LESS:{Value}` Sets and filters the data, keeping all rows with value below {Value}.\
! `SYS:FILTER:VALUE:EQUAL:{Value}` Sets and filters the data, keeping all rows with value equal to {Value}.\
! `SYS:FILTER:VALUE:NOT:{Value}` Sets and filters the data, keeping all rows with value not equal to {Value}.\
! `SYS:FILTER:MONTH:EQUAL:{Value}` Sets and filters the data, keeping all rows with month equal to {Value}.\
! `SYS:FILTER:OUTLIER`\

### Aggregation 
`SYS:AGG:{Value}` Aggregates data to the level described by {Value} (`Yearly`, `Monthly`, `Daily` [NT], `None` [NT]).

### Text and Comments
`TEXT:{Value}` injects {Value} (no spaces allowed) into the output.\
`TEXT:__VAR__` prints value of `__VAR__` to file.\
`TEXT:__MVR__` prints value of `__MVR__` to file.\
! `TEXT:__AGG__` prints value of `__AGG__` to file.\
! `TEXT:__FILE__` prints value of `__FILE__` to file.\
! `TEXT:__LIST_C__:{List}` prints contents of {List} to file.\
NT `TEXTIF:{Condition}:{Text}` prints {Text} out if {Condition} is true.\ 
NT `CONCAT:V:{Value}:{Value}` concatenates {Value} and {Value} seperated by a space.\
NT `CONCAT:T:{Value}:{Token}` concatenates {Value} and the result of token, seperated by a space. The token must be deliminaed with '+' instead of ':'.\
`C:{Value}` is a comment

### Macros
`MACRO:LOAD:{Name}:{From}` Loads a macro named {Name} from source file {From}.\
`MACRO:LOADM:{Name}` Reads items into macro {Name} until `EMACRO` instruction is encountered.\
`MACRO:EXECUTE:{Name}` Executes macro {Name}.\
`MACRO:LOOPVAR:{Name}:{Var List}` Executes macro {Name} once for each item in {Var List}, assigning each consecutive value to `__MVR__`.\
NT `MACRO:LOADSTR:{Name}:{Num Items}` Reads {Num Items} items into macro {Name}.\
`MACRO:TABLE:{Col List}:{Row List}:{Macro}` Executes {Macro}, passing {Col List} components as `__MVR__` and {Row List} Components as `__VAR__`. Automatically pushes and pops `__MVR__` and `__VAR__`.
`MACRO:TABLEL:{Col List}:{Row List}:{Macro}` Executes {Macro}, passing {Col List} components as `__MVR__` and {Row List} Components as `__VAR__`. Automatically pushes and pops `__MVR__` and `__VAR__`. Also prints out values of `__MVR__` and `__VAR__` as labels on the columns and rows.

### Logic
If logic is to be used the `LOGIC_OK` flag should be used. If edits to databases (e.g. via `DBLOOP` or `DBCLEAN`) are to be saved to the original database then the flag `OVERWRITE_DB_OK` flag should be used. `LOGIC` may be use in place of `~`.

! `~:LABEL:{Label Name}` Defines a label named {Label Name}.\
NT P `~:IF:{Condition}:{J/T/E/A}:{Value}` If {Condition} evaluates to true then {Value} will be evaluated according to the {J/T/E/A} switch. \
! `~:GLIF:{Condition}:{J/T/E/A}:{Value}` If {Condition} evaluates to true at anytime during script execution then {Value} will be evaluated according to the {J/T/E/A} switch. \
! `~:DBLOOP:{Var/__GLOBAL__}:{Condition}:{J/T/E/A}:{Value}` loops through either the variable {Var} or through the entire database `__GLOBAL__`, evaluating {Condition}. If true then {Value} will be evaluated according to the {J/T/E/A} switch. \
! `~:DBCLEAN:{Var/Global}:{Condition}:{DR/SV/ED}:{Value}` loops through either the variable {Var} or through the entire database {Global}, evaluating condition. If true then the record will be deleted if `DR` is used, kept if `SV` is used, or changed to {Value} if `ED` is used.\ 
! `{...}:ELSE:{J/T/E/A}:{Value}` added onto end of if loop (maybe).

Result codes:
- ! `J` : Jump to the label described by {Value}. If the label has not been found yet, the interpreter will attempt to find it.
- NT `T` : Write text {Value} to output.
- ! `E` : Execute macro {Value}.
- !! `A` : Execute token {Value}.

Conditions are in the format `{EQ/LS/GR/NT};{Var};{DT/ID};{Date/ID};{Value}`. Note the use of semicolons. The short codes correspond to:
- `EQ` -> equals 
- `LS` -> less than
- `GR` -> greater than
- `NT` -> not equal to
- `TRUE` -> always evalueates condition to true
- `FALSE` -> always evaluated condition to false
`__NAN__` are only valid with `EQ` and `NT`.\
! `__INF__` is only valid with `NT`.\

The date must be formatted as YYYY-MM-DD-HH-MM. 

Value is always a literal number. 

### Variable Stats & Info
`VARS:{Var Name}:INFO` (the info object) (do not enter package names without a full request)\
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
`VARS:{Var Name}:TESTS:MK:tau` computes the mann kendall tau of {Var Name}\
`VARS:{Var Name}:TESTS:MK:mtau:{Other Var}` computes the mann kendall tau of {Var Name} and {Other Var}\
`VARS:{Var Name}:TESTS:TS` (theil-sen test package)\
`VARS:{Var Name}:TESTS:TS:Slope` returns the slope\
`VARS:{Var Name}:TESTS:LINREG` (linear regression package)\
`VARS:{Var Name}:TESTS:LINREG:A` the slope component of the linear regression package\
`VARS:{Var Name}:TESTS:LINREG:B` the y-intecept componenent of the linear regression package

! `VARS:{Var Name}:SCI:iqr` inter quartile range of {Var Name}.\
! `VARS:{Var Name}:SCI:percentile:{Percentile}:{L/U}` returns the value that is the nth {percentile}.\
! `VARS:{Var Name}:SCI:rpercentile:{Value}:{L/U}` returns the percentile associated with {Value}.

`VARS:{Var Name}:CORR:{Other Var Name}` (will auto-agg to monthly)\
`VARS:{Var Name}:PCORR:{Other Var Name}:{Another Var Name}` (will auto-agg to monthly) computer partial correlation of {Var Name} & {Other Var Name} controlled by {Another Var Name}

! `VARS:{Var Name}:LINT:{A/N/H/D/M/Y}` Linearly interpolates {Var Name} to be continuous based on the {A/N/H/D/M/Y} switch.\
! `VARS:{Var Name}:POLYINT:{A/N/H/D/M/Y}`\
! `VARS:{Var Name}:NEARINT:{A/N/H/D/M/Y}`

! `VARS:{Var Name}:OCLIP` Clips outliers out of {Var Name}.

! `VARS:{Var Name}:NORMALIZE` Converts all values from [min, max] to [0, 1].

### Internally-defined macros
See individual function docs for more infornmation.

NT `INT0` runs analysis on all variables.\

NT `INT1` generates 13 correlation tables for all variables.