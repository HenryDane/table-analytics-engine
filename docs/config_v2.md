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
~ MODE {number of mode settings} {mode} 
.db {main database name}
{filename}
{number of db settings} {db settings}

.alias {num alias records}
{alias name} {old name}

.result 
{output file name} 
{trunc/app}
{file format codes}

.main

{program goes here}

```

## Reserved words
Within the context of inputs and internal names that could potentiall conflict with database entries, the following are used internally and shuold not be used within a database for any reason.
- `&{varname}` All strings beginning with `&` refer to a variable.
- `&&{database}` All string beginning witth `&&` refer to a database.
- `__FILE__` This refers to the current filename. Printing this typically returns the filename.
- `__NAN__` This string refers to the NAN value.
- `__INF__` This string refers to infinity.

## Modes
The following modes are supported:
- `UIS` aka `UNDERSCORE_IS_SPACE` aka `1000`
- `LO` aka `LOGIC_ON` aka `1001`
- `OWDB` aka `OVERWRITE_DB_OK` aka `1002`
- `DEBUG`

## Data types
There are three distinct data types in Kevinscript.

## Instructions
No instructions are currently implemented.

### Text
. `TEXT`
. `WRITE`
. `DELETE`
. `BL`
. `NL`

### Variable manipulation
`SETAGG` Sets aggregation (triggerns internal state changes)
`SETPER` Sets period (triggerns internal state changes)
`SETVAR` Sets variable (triggerns internal state changes)
`SETMVR` Sets macro variable 
`SETFILE` Sets the output file

## Database manipulation
`MKDB` Creates a database
`DELDB` Deletes a database
`COPYDB` Copies a database 
. `LOOPDB`

### Stack manipulation
`PUSH` Pushes a variable or database onto the stack
`POP` Pops a variable or database off of the stack
`SWAP` Swaps a variable with a variable or a database with a database
`SIFT` Searches stack for requested object
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

### Legacy and protected functions
`__LEGACY_INT:{#}`
`__LEGACY_EXECUTE_TOKEN` Uses older code to execute token
`__LEGACY_EXECUTE_SCRIPT` Uses older code to execute script
. `__PROTECTED_DIRECT_EXECUTE` 
. `__PROTECTED_MODIFY_TOKEN`
. `__PROTECTED_DELETE_DIR`
. `__PROTECTED_RBUILD_TOKENLIST`
