# DBSyft Version 2 Guide
QAQC Codes:
- `!` Unimplmented
- `NT` Untested
- `!!` Deprecated, will be removed
- `P` Partial
- `B` One or more active bugs

## File format
All files are formatted in the following manner:
```
~DBSyft2
~MODE {number of mode settings} {mode} 
.db {main database name}
{filename}
{number of db settings} {db settings}

.alias {num alias records}
{alias name} {'<' for custom var, '=' for rename} {old name}

.result 
{output file name} 
{trunc/app}
{file format codes}

.main {number of flages} {flags}

{program goes here}

```

## Reserved words
Within the context of inputs and internal names that could potentiall conflict with database entries, the following are used internally and shuold not be used within a database for any reason.
- `&{varname}` All strings beginning with `&` refer to a variable.
- `&&{database}` All string beginning witth `&&` refer to a database.
- `__FILE__` This refers to the current filename. Printing this typically returns the filename.
- `__NAN__` This string refers to the NAN value.
- `__INF__` This string refers to infinity.

## Modes and settings
The following modes are supported:
- `UIS` aka `UNDERSCORE_IS_SPACE` aka `1000`
- `LO` aka `LOGIC_ON` aka `1001`
- `OWDB` aka `OVERWRITE_DB_OK` aka `1002`
- `DEBUG` this takes one parameter, debug level (an integer, or '*').

The following database settings are supported:
- `CSV`
- `PLAIN` (takes the delimter character as a parameter)
- `COLCFG:{Date idx}:{Val idx}:{Var idx}:{Total num idx}:{opt. Time idx}:{opt. state idx}:{opt. mth idx}`


The following are valid result settings:
- `CSV`
- `PLAIN` 

The following are valid program flags:
- `FAST_COMPILE` Tells the interpreter to compile instead and to use internal types, direct access of arrays.
- `LOGIC_PRETEST` Tells interpreter to preprocess all date handling, using internal flagging and typing. (Faster but more memory).
- `LOGIC_AS_LOCKED_PATH` Tells interpreter to make internal "macros" to optomize logic and avoid swapping. (Faster but more memory).
- `DB_SWAP_OK` Requires that program be in compiled format. Tells interpreter to generalize program for any DB.
- `FORCE_LITERAL_INTERPRET` Forces the interpreter to parse without prior enumeration or optomization.
- `NO_SAFETY` Tells interpreter to ignore typing even if it risks crashing.

## Data types
There are three distinct data types in Kevinscript.

## Syntax
DBSyft makes use of fixed-format instructions. Parameters are passed with the `:` character, while arguments are passed with ` `. 

The lookup character `@` is typically used for accessing a row (or group of rows) of a database. This can be applied in 5 ways:
- `!` Lookup by index: `{Table name}@{integer type variable}` will return all rows with the correct index
- `!` Lookup by date: `{Table name}@{string type variable}` if the given variable looks like a date, an attempt will be made to parse it. All resulting rows with the correct date will be returned.
- `!` Lookup by value: `{Table name}@@{variable}` Will return all rows whose value matches variable exactly
- `!` Lookup by value range `{Table name}@@@{variable}:{variable}` 
- `!` Lookup by variable `{Table name}@{variable}` will return all rows whose variable matches variable exactly. If the variable argument looks like a date, it will be parsed as one.

The subobject character `.` is typically used for accessing subobjects. It is only applicable to databases.
- `!` Get row: {Tbl}.r
- `!` Get value: {Tbl}.v
- `!` Get date: {Tbl}.d
- `!` Get variable: {Tbl}.a

## Conditions

## Instructions
No instructions are currently implemented.

### Text
. `TEXT`
. `WRITE`
. `DELETE`
. `BL`
. `NL`
. `CLOG` 
. `__GCLOG` Toggles whether all results should be logged to screen
. `#` Comment

### Variable manipulation
. `ALLOC` Creates a variable or database 
. `DEFPER` Defines an aggregation
`SETAGG` Sets aggregation (triggerns internal state changes) from one argument (a string version of the internal enum)
`SETPER` Sets period (triggerns internal state changes) from either one argument (name of per) or two arguments (mm-dd-yyyy-hh-mm formatted dates)
`SETVAR` Sets variable (triggerns internal state changes) from one argument
`SETMVR` Sets macro variable from one argument
`SETFILE` Sets the output file from one argument

### Database manipulation
`MKDB` Creates a database
`DELDB` Deletes a database
`COPYDB` Copies a database 
. `LOOPDB`

### Stack manipulation
`PUSH` Pushes a variable or database onto the stack (one argument)
`POP` Pops a variable or database off of the stack (one argument)
`SWAP` Swaps a variable with a variable or a database with a database (two arguments)
`SIFT` Searches stack for requested object (one argument (output object), one argument (being looked for))
`__STACKWIPE` Resets the stack

### Macro manipulation
`MACRO` Defines a macro
`MACROF` Defines a macro from a file
`MACROS` Defines a macro via scanning of current file
`EXECUTE` Executes a macro
`MACROLOOP` Loops through a list, executing a macro each time
`MACROTABLE` Loops through two lists, executing a macro each time

## List manipulation
. `LIST` Creates a list
. `LISTFS` Scans data into a list
. `LISTFG` Copies unique variable table into list
. `LISTADD` Merges two lists together

### Control
. `IF`
. `ELSE`
. `ELIF`
. `FOR`
. `WHILE`
. `LABEL`
. `GOTO`
. `STOP`
. `DBLOOP`

### Simple analysis
`FIRSTDATE` Gets the first date in a DB or a DB-variable
`LASTDATE`
`MEAN` aka `AVERAGE`
`MEDIAN`
`STDDEV` aka `STDEV`
`CV` aka `COEFV` aka `COEFVAR`

### Mann-Kendall analysis
`MKP` aka `MANNKENDALLP`
`MKZ` aka `MANNKENDALLZ`
`MKH` aka `MANNKENDALLH`
`MKT` aka `MANNKENDALLT` 
`MKTAU` aka `MANNKENDALLTAU`

### Theil-Sen analysis
`TSS` aka `THEILSENSLOPE`

### Regression analysis
`LINRA` aka `LINEARREGRESSIONA`
`LINRB` aka `LINEARREGRESSIONB`
`LINRR` aka `LINEARREGRESSIONR`
`POLYREG`
`EXPREG`

### Range analysis
`MIN` 
`MAX`
`RANGE`
`IQR`
`PERCENTILE`
`RPERCENTILE`

### Correlation analysis
`CORR`
`PCORR`

### Interpolation
`LINT`
`POLYINT`
`NEARINT`

### Data processing
`OCLIP`
`NORMALIZE`
. `LOG`
. `LOGB`

### Legacy and protected functions
`__LEGACY_INT:{#}`
`__LEGACY_EXECUTE_TOKEN` Uses older code to execute token
`__LEGACY_EXECUTE_SCRIPT` Uses older code to execute script
. `__PROTECTED_DIRECT_EXECUTE` 
. `__PROTECTED_MODIFY_TOKEN`
. `__PROTECTED_DELETE_DIR`
. `__PROTECTED_RBUILD_TOKENLIST`
