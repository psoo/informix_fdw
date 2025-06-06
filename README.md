# About this software

The PostgreSQL Informix Foreign Datawrapper (FDW) module
is a driver for accessing remote Informix table from within
PostgreSQL databases. Foreign Tables are transparently accessed
as normal PostgreSQL tables, they can be used to join remote data
against real PostgreSQL tables, import remote data and more. The FDW
interface implemented in PostgreSQL starting with version 9.1
supports the SQL/MED standard. Starting with PostgreSQL 9.3 this
FDW also supports data modify actions on remote Informix tables.

# Requirements

Informix FDW for PostgreSQL requires a complete
installation of the IBM ESQL/C Client SDK for Informix.

See <https://www.ibm.com/support/pages/informix-client-software-development-kit-client-sdk-and-informix-connect-system-requirements>

for details. Furthermore, the FDW API is available since
PostgreSQL 9.1, so at least 9.1 is required to use this
module. Informix FDW is installed as an EXTENSION, for
details see <http://www.postgresql.org/docs/current/static/sql-createextension.html>

# Compiling

```
INFORMIXDIR=/path/to/your/csdk/installation USE_PGXS=1 make install
```

# Regression tests

If you are a developer and has access to an Informix instance, you can
run the regression test suite. This currently contains two different
regression tests:

* informix_fdw - Checks core functionality
* informix_fdw_dml - Tests DML actions

The regression tests aren't defined in the Makefile itself,
since they require a running Informix database instance and you need to
import a test database into it. Also the regression dump files assume,
you run the tests with the 'informix' user. If you want a different user,
make sure you have the required rights to access the tables.

The following steps are required to prepare the regression test suite

* You need a running Informix instance and full access to it, since you
  need to import the regression database. Once you got access, you need to
  extract the dumps located in the archives from

  informix/regression.tar.bz2
  informix/regression_dml.tar.bz2

  Once extracted, you will find the new subdirectories

  regression.exp
  regression_dml.exp

  in your working directory. Those informix exports require a database with
  the name "regression" (for the regression.exp export files) or "regression_dml"
  for the DML transactional regression tests. Please note that the latter
  writes to the database and makes various changes to the table data there.

  If you can't have a "regression" or "regression_dml" database,
  but a database with a different naming, you need to rename the directory and the
  embedded SQL scripts there to match the database name you have chosen.
  If everything is prepared, import the dump into your target database:

  ```
  $ dbimport -i regression.exp regression
  $ dbimport -i regression_dml.exp regression_dml
  ```

* Prepare your private configuration for the regression tests

  There is a template configuration file located in

  `sql/regression_variables.template`

  Copy this file into  `sql/regression_variables`  and adjust all
  settings according to your Informix setup.

* Now you can run the test suite:

  ```
  $ LANG=en_US.utf8 REGRESS=informix_fdw INFORMIXDIR=$INFORMIXDIR USE_PGXS=1 make installcheck
  $ LANG=en_US.utf8 REGRESS=informix_fdw_tx INFORMIXDIR=$INFORMIXDIR USE_PGXS=1 make installcheck
  ```

  When no errors occurred due to configuration or setup errors, you should see
  the following output for each of both regression test:

```
============== dropping database "contrib_regression" ==============
DROP DATABASE
============== creating database "contrib_regression" ==============
CREATE DATABASE
ALTER DATABASE
============== running regression test queries        ==============
test informix_fdw             ... ok

=====================
 All 1 tests passed. 
=====================
```

Have a look into the the file regression.diffs if there are any errors,
and if no setup or configuration issues are involved, drop me an email
with the contents of that file attached ;)

# Example Setup

Informix database servers use different kinds of connection
methods (Shared Memory, TCP, ...). It is your responsibility to define
a proper Informix connection setup. Informix connections are named
connections via the environment variable `INFORMIXSERVER`. You don't
need to export them before connecting to PostgreSQL and using a
foreign table to your Informix server, the FDW will do all the required
stuff for you. However, you need a working `INFORMIXSERVER` setting in
your Client SDK installation (e.g. `$INFORMIXDIR/etc/sqlhosts` and
`/etc/services` are configured properly). Once you have that working,
the Informix FDW can be used as follows, assumed you have an Informix
connection named 'informix':

```sql
CREATE EXTENSION informix_fdw;

CREATE SERVER test_server
FOREIGN DATA WRAPPER informix_fdw
OPTIONS (informixserver 'informix', informixdir '/opt/informix/csdk');

CREATE USER MAPPING FOR CURRENT_USER
SERVER test_server
OPTIONS (username 'informix', password 'informix');

CREATE FOREIGN TABLE inttest(f1 bigint not null,
                             f2 integer,
			     f3 smallint)
SERVER test_server
OPTIONS(table 'inttest',
        client_locale 'en_US.utf8'
	db_locale 'en_US.819',
	database 'regression_dml');
```

The settings `informixdir` and `informixserver` are dependent on your
installation and configuration. `db_locale` and `client_locale` must be
specified to enable correct conversion between foreign Informix and local PostgreSQL server.

`table` identifies the table on the remote Informix server you want to access. There
is also the `query` parameter in the example above, which can be used to access
the remote data in a view-like manner. For more detailed information about options
read the FDW Options section below.

# Supported datatypes

Currently, only fundamental Informix data types are supported. There's
no support for opaque or user defined types at the moment. Implemented
datatype conversion routines for retrieval:

```
BOOLEAN
DATETIME
DATE
INTERVAL
SMALLINT
INT2
INT4
INT8
BIGINT
SERIAL
SERIAL8
VARCHAR
CHARACTER
LVARCHAR
NCHAR
NVARCHAR
BYTE
TEXT
NUMERIC
MONEY
```

The following types are currently supported for DML:

```
SERIAL, SERIAL8             => SERIAL, SERIAL8
INTEGER, BIGINT             => INT8, INTEGER
INTERVAL                    => INTERVAL
TEXT, BYTEA                 => TEXT, BYTE (LO)
VARCHAR, TEXT               => LVARCHAR, VARCHAR, NVARCHAR, TEXT
TIMESTAMPTZ, TIMESTAMP,DATE => DATETIME
DATE                        => DATE
MONEY, NUMERIC              => NUMERIC, MONEY
```

Note that Informix doesn't support time zones, thus all `TIMESTAMPTZ` values will
be converted into a timestamp without time zone.

Also Informix has a way to declare a special `DATETIME` or `DATE` value with a certain
precision, e.g. `DATE YEAR TO MONTH`, where a date value in the format
`yyyy-mm` is accepted.  This is currently not supported.

The Informix interval format has two defined ranges: `YYYY-MM` and
`DD HH24:MI:SS.FFFFF`, where the fraction can have up to five digits.
Currently the fractions are omitted when doing DML. If a PostgreSQL
interval value spans two ranges, the value is truncated to fit into the
target interval range on the remote server.

PostgreSQL interval types support true "negative" values, where each
component of an interval could be negative. The best example to show
this definition is the following:

```sql
SELECT to_char(interval '15 minutes 30 seconds ago', 'HH24:MI:SS');
  to_char
------------
 00:-15:-30
(1 row)
```

In opposite to this value (which counts 1 day in the past but still has
a positive time range):

```sql
SELECT to_char(interval '-1 day 15 minutes 30 seconds', 'DD HH24:MI:SS')
testfdw-# ;
   to_char
-------------
 -1 00:15:30
(1 row)
```

Informix also supports negative interval values, but it doesn't accept
negative values within. Thus, the first example can't be executed
successfully against Informix:

```sql
INSERT INTO interval_test(f2) VALUES(interval '15 minutes 30 seconds ago');
ERROR:  could not convert attnum 1 to informix type "14", errcode -1263
```

Since the value is passed in ANSI SQL format to Informix, it's string
representation returned by PostgreSQL is "00 00:-15:-30", which
is the correct interpretation of the datum. However, this currently
doesn't work with the Informix FDW, even if you want to consider to
set `IntervalStyle` to `sql_standard`. Since the FDW internally uses
`interval_to_char()`, it doesn't honor the GUC setting at all.

Use this with care if you plan to support DML on such foreign tables.

# FDW Options

* informixserver - required

  Specifies the Informix server identifier passed to the
  `INFORMIXSERVER` environment variable. This value is required and must
  match the identifier specified in your Informix sqlhosts file.

* informixdir - required

  Specifies the path to your Informix installation (either CSDK or locale
  server installation path). This sets the specified value to the
  `INFORMXDIR` environment variable during connection establishing.

* database - required

  The database name where the foreign table is located.

* user - required

  The Informix database username

* password - required

  The Informix user password.

* disable_predicate_pushdown

  Disables predicate pushdown infrastructure. No `WHERE` expressions are
  pushed down to the Informix server anymore (for details about predicate
  pushdown, see the sections below).

* gl_datetime

  Sets the date/time format transmitted from the Informix server.
  This effectively sets `GL_DATETIME` environment variable to the given
  format. If not specified, the FDW uses "%iY-%m-%d %H:%M:%S" per default.

  **_NOTE_**: This format *must* not be a format not understood by the
  PostgreSQL server. If conversion to a PostgreSQL type is requested by
  a date/time value incompatible with any format understood by
  PostgreSQL, an error will occur.

* gl_date

  Sets the date format transmitted from the Informix server.
  This effectively sets the GL_DATE environment variable to the given
  format. If not specified, the FDW uses "%iY-%m-%d" per default.

  **_NOTE_**: This format *must* not be a format not understood by the
  PostgreSQL server. If conversion to a PostgreSQL type is requested by
  a date/time value incompatible with any format understood by
  PostgreSQL, an error will occur.

  Even if specified somewhere in the code, the Informix FDW doesn't set
  DBDATE at the moment. Current Informix versions prefer the GL_DATE
  environment variable in favor of DBDATE.

* client_locale - required

  Sets the `CLIENT_LOCALE` environment variable to specify the locale
  settings the client uses. Please note that the *client* setting in
  this regard is the PostgreSQL backend, so this settings is required to
  match the current locale settings used in your PostgreSQL connection.
  This setting is required for each foreign table.

  **_NOTE_**: This setting depends on the available client locales
  installed with your Informix installations. The name to be passed
  differs from the setting actually taken from PostgreSQL, since
  PostgreSQL relies on the operating system locale, where Informix uses
  its own. This can lead to some confusion to find the correct setting
  and might cause compatibility problems (incompatible string
  comparisons et al.). The FDW for example currently allows strings
  expressions embedded in query predicates to be pushed down to the
  Informix server.  Take care in this case, since the mentioned
  incompatibilities could lead to wrong results. I don't want to
  restrict this setting for now and leave it up to the user to make
  sure, the settings actually work in their environment.

* db_locale

  Specifies the locale settings passed to the `DB_LOCALE` environment variable.
  This value must be compatible with the `CLIENT_LOCALE` setting chosen with
  the client_locale FDW setting described above. Ideally, this setting reflect
  the locale settings chosen on the Informix database.

  **_NOTE_**: The Informix FDW will raise an error with incompatible settings.

* enable_blobs

  This variable sets a hint to the Informix FDW that the foreign table
  has BLOBs. Set it to enable_blobs = '1' to enable BLOB support or omit
  this option to disable it (it doesn't matter which value you pass
  to enable_blobs, it only needs to be present).

  **_NOTE:_** If you have a foreign table with BLOBs but don't want to
  select any of these, you can safely omit this setting. However, you
  have to make sure, that you don't query the columns at all (pass your
  own query string to the query variable in this case). If you select
  BLOB columns without having set enable_blobs, an error will be raised.

  Also consider that selecting foreign tables with BLOBs isn't safe
  with non-logging Informix databases under certain conditions. The FDW
  will raise a WARNING if you encounter such a situation.

  The reason for this restriction is that the Informix FDW uses
  a `SCROLL` cursor internally per default. However, Informix doesn't
  support `SCROLL` cursors in case someone is selecting BLOBs from
  a table. We switch to `NO SCROLL` in case `enable_blobs` is specified,
  but this leaves us with the last restriction below, where you can
  get inconsistent reads when having an Informix database without
  logging.

* query

  The foreign table will issue the specified query to the Informix server to
  materialize the result set.

* table

  The foreign table will build its own query against the given table on the
  Informix server.

  **NOTE**: Either table or query is required for a foreign table.

* disable_rowid

  Normally the Informix FDW uses a `ROWID` to identify tuples on the remote
  server when updating or deleting data. This doesn't work in Informix for
  all tables, fragmented tables for example doesn't support ROWIDs per default.
  If it's not possible to enforce the `ROWID` on the remote Informix server,
  disable_rowid can be used to switch to an updatable cursor for `UPDATE` and
  `DELETE` actions. However, this has the disadvantage that the foreign table
  isn't not safe to be used in `UPDATE...FROM` and `DELETE...USING` statements
  anymore, where the join is performed with a hash join for example
  (see below for details). A normal `DELETE` or `UPDATE` without a join is
  usable without any restrictions though.

# Predicate Pushdown

The Informix FDW is able to pushdown query predicates which meet the following
conditions:

* The expression is of type `VAR OP CONST`, where `VAR` is a column reference to
  the foreign table and `CONST` a constant value
* OP must be one of the following operators:
  `<, >, =, <=, >=, LIKE`
* Matching of column references is done on a per-name basis: that means even
  you can name a column in a foreign table different than on your remote Informix
  table, it cannot be successfully pushded down and will throw an error (in that case
  you need to turn of predicate pushdown).
* Currently, the FDW allows to push down predicates with `<, <=, >, >=` on text/varchar
  columns as well. This might lead to incorrect results, when the selected locale
  settings doesn't match. However, it seems far to conservative to restrict this at all,
  but be careful when using such predicates and check your results carefully.

# GLS Support

Informix GLS support is provided through the `CLIENT_LOCALE` and `DB_LOCALE`
database connection parameters. At least, each foreign server is required
to define a valid `CLIENT_LOCALE`, which should match the server_encoding
of the PostgreSQL database.

# Helper functions

To get a list of cached Informix database connections in a PostgreSQL
session, use the `ifx_fdw_get_connections()` procedure:

```sql
#= SELECT * FROM ifx_fdw_get_connections();
-[ RECORD 1 ]--------+----------------------------
connection_name      | informixtestol_informix1170
established_by_relid | 230262
servername           | ol_informix1170
informixdir          | /Applications/IBM/informix
database             | test
username             | informix
usage                | 2
db_locale            | en_us.819
client_locale        | en_US.utf8
```

It is possible to close a remote connection manually, if required. This
can be done by `ifx_fdw_close_connection()`:

```sql
#= SELECT * FROM ifx_fdw_close_connection('informixtxtestol_informix1210');
 ifx_fdw_close_connection
--------------------------

(1 row)
```

The function throws an error if the existing connection does not exist or
couldn't be disconnected. If no Informix connections were already used in a session,
the connection cache isn't initialized yet, which is treated as an error, too.

# Transaction control

The Informix FDW is able to coordinate transactions with local PostgreSQL
transactions and savepoints. For example, parent transactions start also
a new transaction on the remote server (if not already in progress). If
a `SAVEPOINT` is started locally on the PostgreSQL server, there will also be a
`SAVEPOINT` on the remote Informix server. `SAVEPOINT`s are stacked, so that each
`ROLLBACK` TO command will `ROLLBACK` and `RELEASE` the corresponding
`SAVEPOINT` on the remote Informix server.

# Caveats

* Even if not currently enforced, the Informix FDW heavily relies
  on the fact, that a foreign table has the same name like its counterpart
  on the remote Informix server.

* The predicate pushdown feature allows to push down predicates of the
  following type:

  `<Var> [<,<=,>,>=] 'string literal'`

  This could lead to problems in case of incompatible locale and|or collation
  definitions on the PostgreSQL and Informix side.

* Take care if you are using the `MONEY` type locally in a PostgreSQL server
  and map them to `MONEY` on the Informix server. Since this type is locale
  dependent, you should configure at least `LC_MONETARY` to match the locale
  setting on the Informix server. Otherwise you might get confusing results.

  It is possible to specify the `DBMONEY` environment variable through the Informix
  FDW options, however, since the `MONEY` value is read in through its type input
  function locally it still evaluates always against the local currency settings.

## Restrictions to DML

* `UPDATE` and `DELETE` cannot be part of an `UPDATE FROM` or `DELETE FROM` clause
  if the `disable_rowid` parameter is set.

  In this case the Informix FDW employs an updatable cursor which interacts with the
  associated foreign scan. It's possible that the cursor gets out of sync when
  the PostgreSQL planner decides to use some specific join strategies, based on
  the command (e.g. `UPDATE remote_table SET ... FROM local_table WHERE...`)

  The FDW will issue an error in this case.

## Datatype conversion issues

* Interval ranges are restricted in Informix to "YYYY-MM" and "DD HH24:MI:SS.FFFFF".
  If used in a DML transactions, those values are truncated when inserted from
  PostgreSQL.

* Certain interval values currently aren't handled very well by the Informix FDW.
  Candidates are especially negative interval values, since they get decorated
  with explicits signs in the time part, which aren't allowed by the Informix
  INTERVAL datatype. If you get a -1263 error when using DML with INTERVAL, please
  check if you use negative values (see examples above).

* `TEXT` in Informix is known to have encoding issues that doesn't work properly with
  PostgreSQL text and varchar types. You might find it better to use bytea instead, however,
  this format isn't very suitable to work with. The Informix FDW supports conversion from
  `TEXT` to bytea nevertheless.

# ToDo

- Improve usage of planner/local foreign table statistics.
