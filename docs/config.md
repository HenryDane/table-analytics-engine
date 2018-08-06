# `out.cfg` Formatting Guide
If something is preceed or associated with `!` it means that said item is unimplemented. Likewise `NT` means that said item is untested. 

## Notes
A simple example program would be the following:
```
~ CONFIG
# MODE 1 UNDERSCORE_IS_SPACE
# BEGIN outfile
TEXT:Hello_World
STOP
```
- The `~ CONFIG` component must be present. 
- The strings `__VAR__`, `__AGG__`, `__MVR__`, `__FILE__`, and `__LIST_C__` can not be printed. 
- Spaces in strings should be replaced with underscores, and the `UNDERSCORE_IS_SPACE` directive should be used. 
- `__NULL__` and `__NAN__` are also reserved but may be printed. They can not be printed *literaly* however. 

## Mode settings
The first thing after the `# MODE` marker must be the number of settings.\
`UNDERSCORE_IS_SPACE` replaces all underscores with spaces when writing.\
`COLORS_OK` allows color code to be written (ignored if `XML_EXCEL` is not format code).\
`AUTO_RESET_FILTERS` resets filters automatically.\
`LOGIC_OK` signals that logic commands are to be used. Unless `OVERWRITE_DB_OK` is also present, this will result in a backup being created.\
`OVERWRITE_DB_OK` signals that backups are not to be used, and that the database should be reopened in read-write mode.

## Begin settings
`CSV` declares that the output is a CSV file.\
`XM_EXCEL` declares that the output is an Excel file described in XML.\
`PLAIN` declares that the output is a plaintext file (`BL` does nothing)

## Instructions

### State control 
`STOP` signals to stop scanning and end interpretation.\
`EMACRO` signals to stop recording a macro.\
`ELIST` signals to stop recording a list.

### Text formatting
`NL` Injects newline to output.\
`BL` Injects a blank (cell, space, etc) to output.

### Variable adjustment
`SYS:OUT:__VAR__:{Value}` Sets the `__VAR__` variable to {Value}.\
`SYS:OUT:__MVR__:{Vaule}` Sets the `__MVR__` variable to {Value}.\
! `SYS:OUT:__FILE__:{Value}` Switches the output file to {Value}.\
NT `SYS:SELF:RESET` Resets all internal lists, registries, macros, etc.
NT `SYS:SELF:CRASH` Resets all internal lists, registries, macros, cvars, etc

### Registry 
`SYS:REGISTRY:PERIOD:{name}:{Y}:{M}:{D}:{Y}:{M}:{D}` defines a period.\
`SYS:REGISTRY:LIST:CREATE:{Name}` defines a list (strings) named {Name}.\
NT `SYS:REGISTRY:LIST:ADD:{Name}:{Item}` adds item {Item} to the list {Name}.\
NT `SYS:REGISTRY:LIST:MERGE:{Name 1}:{Name 2}` concatenates the two lists into the first.\
`SYS:REGISTRY:LIST:SCAN:{Name}` reads items into a list until `ELIST` is encountered.\
! `SYS:REGISTRY:LIST:FCVAR:{Name}:{Var Name}` reads items from custom var def into {Name}.

### Filter 
`SYS:FILTER:RESET` Resets all filters to raw variable.
`SYS:FILTER:PERIOD:{Value}` Sets and filters the data according to a predefined period {Value}.\
! `SYS:FILTER:VALUE:GREATER:{Value}` Sets and filters the data, keeping all rows with value above {Value}.\
! `SYS:FILTER:VALUE:LESS:{Value}` Sets and filters the data, keeping all rows with value below {Value}.\
! `SYS:FILTER:VALUE:EQUAL:{Value}` Sets and filters the data, keeping all rows with value equal to {Value}.\
! `SYS:FILTER:VALUE:NOT:{Value}` Sets and filters the data, keeping all rows with value not equal to {Value}.\
! `SYS:FILTER:MONTH:EQUAL:{Value}` Sets and filters the data, keeping all rows with month equal to {Value}.

### Aggregation 
`SYS:AGG:{Value}` Aggregates data to the level described by {Value} (`Yearly`, `Monthly`, `Daily` [NT], `None` [NT]).

### Text and Comments
`TEXT:{Value}` injects {Value} (no spaces allowed) into the output.\
`TEXT:__VAR__` prints value of `__VAR__` to file.\
`TEXT:__MVR__` prints value of `__MVR__` to file.\
`TEXT:__LIST_C__:{List}` prints contents of {List} to file.\
! `CONCAT:{Value}:{Value}`.\
! `CONCATM:{Value}:{Macro}`.\
! `CONCATT:{Value}:{Token}`.\
NT `C:{Value}` is a comment

### Macros
! `MACRO:LOAD:{Name}:{From}` Loads a macro named {Name} from source file {From}.\
`MACRO:EXECUTE:{Name}` Executes macro {Name}.\
`MACRO:LOOPVAR:{Name}:{Var List}` Executes macro {Name} once for each item in {Var List}, assigning each consecutive value to __MVR__.\
! `MACRO:LOADSTR:{Name}:{Num Items}` Reads {Num Items} items into macro {Name}.\
`MACRO:LOADM:{Name}` Reads items into macro {Name} until `EMACRO` instruction is encountered.

### Logic
If logic is to be used the `LOGIC_OK` flag should be used. If edits to databases (e.g. via `DBLOOP` or `DBCLEAN`) are to be saved to the original database then the flag `OVERWRITE_DB_OK` flag should be used. 

! `~:IF:{Condition}:{J/T/E/A}:{Value}` If {Condition} evaluates to true then {Value} will be evaluated according to the {J/T/E/A} switch. \
! `~:GLIF:{Condition}:{J/T/E/A}:{Value}` If {Condition} evaluates to true at anytime during script execution then {Value} will be evaluated according to the {J/T/E/A} switch. \
! `~:DBLOOP:{Var/Global}:{Condition}:{J/T/E/A}:{Value}` loops through either the variable {Var} or through the entire database {Global}, evaluating {Condition}. If true then {Value} will be evaluated according to the {J/T/E/A} switch. \
! `~:DBCLEAN:{Var/Global}:{Condition}:{DR/SV}` loops through either the variable {Var} or through the entire database {Global}, evaluating condition. If true then the record will be deleted if `DR` is used or kept if `SV` is used.\ 
! `{...}:ELSE:{J/T/E/A}:{Value}`

Result codes:
- `J` : Jump to the label described by {Value}. If the label has not been found yet, the interpreter will attempt to find it.
- `T` : Write text {Value} to output.
- `E` : Execute macro {Value}.
- `A` : Execute token {Value}.

Conditions are in the format `{EQ/LS/GR/NT}:{Var}:{DT/ID}:{Date/ID}:{Value}`. The short codes correspond to:
- `EQ` -> equals 
- `LS` -> less than
- `GR` -> greater than
- `NT` -> not equal to
`__NAN__` and `__NULL__` are only valid with `EQ` and `NT`.

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
`VARS:{Var Name}:TESTS:TS` (theil-sen test package)\
`VARS:{Var Name}:TESTS:TS:Slope` returns the slope\
`VARS:{Var Name}:TESTS:LINREG` (linear regression package)\
`VARS:{Var Name}:TESTS:LINREG:A` the slope component of the linear regression package\
`VARS:{Var Name}:TESTS:LINREG:B` the y-intecept componenent of the linear regression package

`VARS:{Var Name}:CORR:{Other Var Name}` (will auto-agg to monthly)\
`VARS:{Var Name}:PCORR:{Other Var Name}:{Another Var Name}` (will auto-agg to monthly) computer partial correlation of {Var Name} & {Other Var Name} controlled by {Another Var Name}
