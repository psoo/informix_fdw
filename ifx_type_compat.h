/*-------------------------------------------------------------------------
 *
 * ifx_type_compat.h
 *		  foreign-data wrapper for INFORMIX(tm) databases
 *
 * Describes the API for accessing the INFORMIX module without
 * the need to include PostgreSQL-related header files to avoid
 * conflicts.
 *
 * Copyright (c) 2012, credativ GmbH
 *
 * IDENTIFICATION
 *		  informix_fdw/ifx_fdw.c
 *
 *-------------------------------------------------------------------------
 */

#ifndef HAVE_IFX_TYPE_COMPAT_H
#define HAVE_IFX_TYPE_COMPAT_H

/*
 * Max length of an identifier and connection name.
 *
 * Informix allows up to 128 Bytes per identifier in newer
 * releases, so leave room for the user/server identifier.
 */
#define IFX_IDENT_MAX_LEN 128
#define IFX_CONNAME_LEN (2 * IFX_IDENT_MAX_LEN)

/*
 * Fixed size INFORMIX value are always treated to
 * have length -1.
 */
#define IFX_FIXED_SIZE_VALUE -1

/*
 * Maximum length of a signed int8 value
 * in its character representation.
 */
#define IFX_INT8_CHAR_LEN 21

/*
 * Maximum length for decimal
 * character representation (without
 * null byte).
 */
#define IFX_DECIMAL_BUF_LEN 30

/*
 * Maximum number of FLOAT digits, excluding
 * null byte.
 *
 * NOTE: This should match MAXDOUBLEWIDTH in PostgreSQL.
 *       See src/backend/utils/adt/float.c for details.
 */
#define IFX_MAX_FLOAT_DIGITS 128

/*
 * Indicates special conversion codes
 * during PostgreSQL -> Informix datatype conversion.
 */
#define	IFX_CONVERSION_OVERFLOW -255
#define IFX_CONVERSION_UNDEFINED -254
#define	IFX_CONVERSION_OK 0

/*
 * Flags to identify current state
 * of informix calls.
 */
#define IFX_STACK_EMPTY    0
#define IFX_STACK_PREPARE  1
#define IFX_STACK_DECLARE  2
#define IFX_STACK_ALLOCATE 4
#define IFX_STACK_DESCRIBE 8
#define IFX_STACK_OPEN     16

/*
 * Special flags set during DESCRIBE phase to
 * identify special columns (e.g. BLOBS)
 */
#define IFX_NO_SPECIAL_COLS 0
#define IFX_HAS_BLOBS       1
#define IFX_HAS_OPAQUE      2

/*
 * IS8601 compatible DATE and DATETIME
 * output formats for Informix.
 */
#define IFX_DBDATE_FORMAT "Y4MD-"
#define IFX_ISO_DATE "%iY-%m-%d"
#define IFX_ISO_TIMESTAMP "%iY-%m-%d %H:%M:%S"
#define IFX_ISO_TIMESTAMP_FRAC5 "%iY-%m-%d %H:%M:%S.%FFFFF"

/*
 * Binary size of informix DATE value
 */
#define IFX_DATE_BINARY_SIZE 4

/*
 * Default buffer length for
 * DATE character strings.
 */
#define IFX_DATE_BUFFER_LEN 20

/*
 * Default buffer length for
 * DATETIME and INTERVAL character strings.
 */
#define IFX_DATETIME_BUFFER_LEN 30

/*
 * Maximum length of an Informix lvarchar type
 */
#define IFX_MAX_LVARCHAR_LEN 32739

/*
 * Maximum length of an Informix varchar type
 */
#define IFX_MAX_VARCHAR_LEN 255

/*
 * Maximum length of CHAR and NCHAR types
 */
#define IFX_MAX_NCHAR_LEN 32767

/*
 * Which kind of CURSOR to use.
 */
typedef enum IfxCursorUsage
{
	IFX_DEFAULT_CURSOR,
	IFX_UPDATE_CURSOR, /* NO SCROLL cursor with FOR UPDATE */
	IFX_INSERT_CURSOR,
	IFX_SCROLL_CURSOR,
	IFX_NO_CURSOR
} IfxCursorUsage;

/*
 * Informix SQLSTATE classes.
 */
typedef enum IfxSqlStateClass
{
	IFX_RT_ERROR = -1,
	IFX_SUCCESS = 0,
	IFX_WARNING = 1,
	IFX_ERROR   = 2,

	IFX_NOT_FOUND = 100,

	IFX_CONNECTION_OK = 200,
	IFX_CONNECTION_WARN = 201,
	IFX_CONNECTION_ERROR = 202,

	IFX_STATE_UNKNOWN = 1000,

	/* specific SQL object errors */
	IFX_ERROR_TABLE_NOT_FOUND = 2000,
	IFX_ERROR_INVALID_NAME    = 2001,
} IfxSqlStateClass;

/*
 * IfxSqlStateMessage
 *
 * Message from a SQLSTATE exception
 */
typedef struct IfxSqlStateMessage
{
	int   id;
	int   len;
	char  sqlcode;
	char  sqlstate[6];
	char  text[255];
	char  class_origin[255];
	char  subclass_origin[255];
} IfxSqlStateMessage;

/*
 * Define supported source types which can
 * be converted into a specific PostgreSQL type.
 *
 * This table is translated from INFORMIXDIR/incl/sqltypes.h. We
 * can't use it directly, since Informix redefines int2 and int4
 * in ifxtypes.h, which is included by sqltypes.h implicitely. So
 * maintain our own lookup types for compatibility.
 */

typedef enum IfxSourceType
{

	IFX_CHARACTER = 0,
	IFX_SMALLINT  = 1,
	IFX_INTEGER   = 2,
	IFX_FLOAT     = 3,
	IFX_SMFLOAT   = 4,
	IFX_DECIMAL   = 5,
	IFX_SERIAL    = 6,
	IFX_DATE      = 7,
	IFX_MONEY     = 8,
	IFX_NULL      = 9,
	IFX_DTIME     = 10,
	IFX_BYTES     = 11,
	IFX_TEXT      = 12,
	IFX_VCHAR     = 13,
	IFX_INTERVAL  = 14,
	IFX_NCHAR     = 15,
	IFX_NVCHAR    = 16,
	IFX_INT8      = 17,
	IFX_SERIAL8   = 18,
	IFX_SET       = 19,
	IFX_MULTISET  = 20,
	IFX_LIST      = 21,
	IFX_ROW       = 22,
	IFX_COLLECTION = 23,
	IFX_ROWREF     = 24,

	/*
	 * User defined opaque types
	 *
	 * Might also reference built-in opaque types.
	 */
	IFX_UDTVAR    = 40,
	IFX_UDTFIXED  = 41,
	IFX_REFSER8   = 42,

	/*
	 * Special ESQL/C types.
	 */
	IFX_LVARCHAR  = 43,
	IFX_BOOLEAN   = 45,
	IFX_INFX_INT8 = 52,
	IFX_BIGSERIAL = 53

} IfxSourceType;

/*
 * Defines extended Informix types.
 *
 * According to $INFORMIXDIR/incl/esql/sqltypes.h, those
 * are encoding in the sysxtdtypes system catalog. Usually the
 * ESQL/C API won't get in touch with them, but when querying the
 * system catalogs directly, conversion routines might have to
 * deal with them.
 */
typedef enum IfxExtendedType
{
	IFX_XTD_LVARCHAR  = 1,
	IFX_XTD_SENDRECV  = 2,
	IFX_XTD_IMPEXP    = 3,
	IFX_XTD_IMPEXPBIN = 4,
	IFX_XTD_BOOLEAN   = 5,
	IFX_XTD_POINTER   = 6,
	IFX_XTD_INDEXKEYARRAY = 7,
	IFX_XTD_RTNPARAMTYPES = 8,
	IFX_XTD_SELFUNCARGS   = 9,
	IFX_XTD_BLOB          = 10,
	IFX_XTD_CLOB          = 11,
	IFX_XTD_LOLIST        = 12,
	IFX_XTD_IFX_LO_SPEC	  = 13,
	IFX_XTD_IFX_LO_STAT	  = 14,
	IFX_XTD_STAT          = 15,
	IFX_XTD_CLIENTBINVAL  = 16,
	IFX_XTD_UDTMODIFIERS  = 17,
	IFX_XTD_AGGMODIFIERS  = 18,
	IFX_XTD_UDRMODIFIERS  = 19,
	IFX_XTD_GUID          = 20,
	IFX_XTD_DBSENDRECV    = 21,
	IFX_XTD_SRVSENDRECV   = 22,
	IFX_XTD_FUNCARG       = 23

} IfxExtendedType;

/*
 * Defines Informix qualifier identifier
 *
 * We redefine them here, since we don't want to make
 * them accessible directly through the PostgreSQL backend, since
 * there are various symbol clashes around.
 */
#define	IFX_TU_YEAR  0
#define	IFX_TU_MONTH  2
#define	IFX_TU_DAY  4
#define	IFX_TU_HOUR  6
#define	IFX_TU_MINUTE  8
#define	IFX_TU_SECOND  10
#define	IFX_TU_FRAC  12
#define	IFX_TU_F1  11
#define	IFX_TU_F2  12
#define	IFX_TU_F3  13
#define	IFX_TU_F4  14
#define	IFX_TU_F5  15

/*
 * Timestamp or interval qualifier range
 */
typedef struct IfxTemporalRange
{
    int start;
    int end;
    int precision;
} IfxTemporalRange;

/*
 * Output format for interval/timestamp conversion
 * format strings.
 *
 * Currently, PostgreSQL and Informix uses nearly identical
 * format placeholders to format a interval or timestamp
 * value into their corresponding types or vice versa. However,
 * there are some minor differences (e.g. US vs. F format placeholders
 * for the fraction of an interval value). ifxGetIntervalFormatString()
 * currently is the only place which honours this difference. Use
 * the following definitions to get the right format string
 * back.
 */
typedef enum IfxFormatMode
{
    FMT_PG,
	FMT_IFX
} IfxFormatMode;

/*
 * Defines Informix indicator values. Currently,
 * NULL and NOT NULL values are supported.
 */
typedef enum IfxIndicatorValue
{
	INDICATOR_NULL,
	INDICATOR_NOT_NULL,
	INDICATOR_NOT_VALID
} IfxIndicatorValue;

/*
 * IfxAttrDef
 *
 * Holds Informix column type definitions
 * retrieved by an query descriptor.
 */
typedef struct IfxAttrDef
{
	IfxSourceType  type;
	IfxExtendedType extended_id;
	int            len;
	char          *name;
	IfxIndicatorValue indicator;
	size_t            mem_allocated; /* memory allocated for data */
	size_t            loc_buf_size;  /* memory allocated for additional BLOB buffer */
	char             *loc_buf;       /* BLOB data buffer of size loc_buf_size */
	int               offset;        /* offset into the data memory buffer */
	int               converrcode;   /* internal Informix conversion error code,
									  * 0 if no conversion error set. This value
									  * is only set during modify action when converting
									  * column values to Informix.
									  * -255 is set by the FDW in case of memory
									  * overflow
									  */
} IfxAttrDef;

/*
 * Stores plan data, e.g. row and cost estimation.
 * Pushed down from the planner stage to ifxBeginForeignScan().
 * Stays here only because it's currently used in IfxConnectionInfo
 * (XXX: need to change that, not really required) :(
 */
typedef struct IfxPlanData
{
	double estimated_rows;
	double costs;
	double total_costs;

	/*
	 * Table statistics derived
	 * from Informix.
	 * XXX: not used at the moment
	 */
	double nrows;
	double npages;
	short pagesize;
	short row_size;

} IfxPlanData;

/*
 * Foreign scan modes.
 *
 * Describes the specific steps performed through
 * a foreign scan.
 */
typedef enum
{
	IFX_PLAN_SCAN,    /* plan a new foreign scan, generate new refid for scan */
	IFX_BEGIN_SCAN,   /* start/preparing foreign scan */
	IFX_ITERATE_SCAN, /* foreign scan iteration step */
	IFX_END_SCAN,     /* end foreign scan */
	IFX_IMPORT_SCHEMA /* Scan mode for IMPORT FOREIGN SCHEMA */
} IfxForeignScanMode;

/*
 * Informix connection
 */
typedef struct IfxConnectionInfo
{
	char *servername;
	char *informixdir;
	char *username;
	char *password;
	char *database;

	/*
	 * once generated, this holds the connection
	 * name and connection string
	 */
	char conname[IFX_CONNAME_LEN + 1];
	char *dsn;

	/*
	 * Table to access with query
	 */
	char *tablename;
	char *query;

	/*
	 * Set to IFX_PLAN_SCAN if a new foreign scan on a foreign table
	 * is requested.
	 *
	 * This is currently used in ifxPlanForeignScan() and
	 * ifxGetForeignRelSize() to teach the connection cache
	 * to generate a new scan id only. In detail, ifxConCache_add()
	 * will increase the usage counter of the cached connection
	 * handle only, if a new scan is requested.
	 */
	IfxForeignScanMode scan_mode;

	/*
	 * Environment options, e.g. GL_DATE, ...
	 */
	char *gl_date;
	char *gl_datetime;
	char *client_locale;
	char *db_locale;
	char *db_monetary;
	short tx_enabled; /* 0 = n tx, 1 = tx enabled */
	int   xact_level; /* current nest level of transactions */
	short db_ansi; /* 0 = non-ANSI database, 1 = ANSI-enabled database */
	short is_obsolete; /* currently: 0 = non SE instance, 1 = Informix SE instance */
	short predicate_pushdown; /* 0 = disabled, 1 = enabled */
	short enable_blobs; /* 0 = no special BLOB support,
						   1 = special BLOB support */
	short disable_rowid; /* 1 = disable, 0 enable rowid (default) */
	short delimident; /* 1 = DELIMIDENT set, 0 = disabled */

	/* plan data */
	IfxPlanData planData;

} IfxConnectionInfo;

/*
 * This is a ancestor of IfxCachedConnection structure
 * suitable to be passed down to the Informix API. Since we can't
 * include Informix headers because of symbol collisions in pre 9.2
 * versions, we just use a cast to this generic struct for passing
 * cached connection handles down to the Informix layer.
 */
typedef struct IfxPGCachedConnection
{
	char ifx_connection_name[IFX_CONNAME_LEN];
	char *servername;
	char *informixdir;
	char *username;
	char *database;
	char *db_locale;
	char *client_locale;
	int usage;
	int tx_enabled;
	int tx_in_progress;
	int db_ansi;

	/*
	 * Stats counters
	 */
	int tx_num_commit;
	int tx_num_rollback;
} IfxPGCachedConnection;

/*
 * IfxStatementInfo
 *
 * Transports state information during a FDW scan.
 */
typedef struct IfxStatementInfo
{
	/*
	 * Links the informix database connection to
	 * this statement.
	 */
	char conname[IFX_CONNAME_LEN + 1];

	/*
	 * Which kind of Informix Cursor to use.
	 * Default must be initialized to IFX_SCROLLL_CURSOR.
	 */
	IfxCursorUsage cursorUsage;

	/*
	 * Reference ID of the connection handle this
	 * statement handle was created for. This is effectively
	 * the usage counter passed to this structure when
	 * a foreign table scan is prepared. Used to uniquely
	 * identify the associated cursor.
	 *
	 * Unassociated statement handles must be initialized
	 * with -1.
	 */
	int refid;

	/*
	 * SQLSTATE value retrieved by the last action.
	 *
	 * This value shouldn't be set directly. Instead
	 * use ifxSetError() (see ifx_connection.ec for details).
	 */
	char sqlstate[6];

	/*
	 * Call stack. Used to track
	 * the current state of informix calls.
	 */
	unsigned short call_stack;

	/*
	 * Number of exceptions per ESQL call.
	 */
	int exception_count;

	/*
	 * Query text.
	 */
	char *query;

	/*
	 * Predicate string to be pushed down.
	 * This is the string representation of the WHERE
	 * expressions examined during the planning phase...
	 */
	char *predicate;

	/*
	 * Name of an associated cursor.
	 */
	char *cursor_name;

	/*
	 * Name of the prepared statement.
	 */
	char *stmt_name;

	/*
	 * Named descriptor area.
	 */
	char *descr_name;

	/*
	 * Size of the informix column descriptor list.
     * Should match number of IfxAttrDef elements in ifxAttrDefs.
	 */
	int ifxAttrCount;

	/*
	 * Size of the foreign table column descriptor
	 * list. Should match the number of columns of
	 * the foreign table defined in pgAttrDefs.
	 *
	 * XXX: Required here?
	 *
	 * XXX: Pointer to internal informix SQLDA structure
	 */
	void *sqlda;

	/*
	 * Allocated row size. Should be > 0 in case any allocations
	 * to SQLDA and sqldata within sqlvar structs occur.
	 */
	size_t row_size;

	/*
	 * Memory area for sqlvar structs to store values.
	 */
	char *data;

	/*
	 * Memory area for SQLDA indicator values.
	 */
	short *indicator;

	/*
	 * Special flags set during DESCRIBE phase. Helps
	 * to identify special column types.
	 */
	short special_cols;

	/*
	 * Attribute number map.
	 *
	 * Maps PostgreSQL attribute numbers to
	 * Informix attribute numbers.
	 */
	int *pgIfxAttrMap;

	/*
	 * Dynamic list of attribute definitions
	 */
	IfxAttrDef *ifxAttrDefs;

} IfxStatementInfo;

/*
 * Number of current transactions
 * in progress per backend.
 */
extern unsigned int ifxXactInProgress;

extern void ifxCreateConnectionXact(IfxConnectionInfo *coninfo);
void ifxSetConnection(IfxConnectionInfo *coninfo);
int ifxSetConnectionIdent(char *conname);
void ifxDisconnectConnection(char *conname);
void ifxDestroyConnection(char *conname);
void ifxPrepareQuery(char *query, char *stmt_name);
void ifxAllocateDescriptor(char *descr_name, int num_items);
void ifxDescribeAllocatorByName(IfxStatementInfo *state);
int ifxDescriptorColumnCount(IfxStatementInfo *state);
void ifxDeclareCursorForPrepared(char *stmt_name, char *cursor_name,
								 IfxCursorUsage cursorType);
void ifxOpenCursorForPrepared(IfxStatementInfo *state);
size_t ifxGetColumnAttributes(IfxStatementInfo *state);
void ifxFetchRowFromCursor(IfxStatementInfo *state);
void ifxFetchFirstRowFromCursor(IfxStatementInfo *state);
void ifxDeallocateSQLDA(IfxStatementInfo *state);
void ifxSetupDataBufferAligned(IfxStatementInfo *state);
void ifxCloseCursor(IfxStatementInfo *state);
int ifxFreeResource(IfxStatementInfo *state,
					int stackentry);
void ifxDeallocateDescriptor(char *descr_name);
char ifxGetSQLCAWarn(signed short warn);
int ifxGetSQLCAErrd(signed short ca);
void ifxSetDescriptorCount(char *descr_name, int count);
void ifxGetSystableStats(char *tablename, IfxPlanData *planData);
void ifxPutValuesInPrepared(IfxStatementInfo *state);
void ifxFlushCursor(IfxStatementInfo *info);
IfxIndicatorValue ifxSetSqlVarIndicator(IfxStatementInfo *info, int ifx_attnum,
										IfxIndicatorValue value);
void ifxExecuteStmt(IfxStatementInfo *state);
void ifxDescribeStmtInput(IfxStatementInfo *state);
void ifxExecuteStmtSqlda(IfxStatementInfo *state);
IfxTemporalRange ifxGetTemporalQualifier(IfxStatementInfo *state,
										 int ifx_attnum);

/*
 * Transaction control
 */
int ifxCommitTransaction(IfxPGCachedConnection *cached, int subXactLevel);
int ifxRollbackTransaction(IfxPGCachedConnection *cached, int subXactLevel);
int ifxStartTransaction(IfxPGCachedConnection *cached,
						IfxConnectionInfo *coninfo);


/*
 * Error handling
 */
IfxSqlStateClass ifxSetException(IfxStatementInfo *state);
IfxSqlStateClass ifxConnectionStatus(void);
IfxSqlStateClass ifxGetSqlStateClass(void);
int ifxExceptionCount(void);
void ifxGetSqlStateMessage(int id, IfxSqlStateMessage *message);
int ifxGetSqlCode(void);

/*
 * Functions to access specific datatypes
 * within result sets
 */
char *ifxGetFloatAsString(IfxStatementInfo *state, int attnum, char *buf);
char *ifxGetInt8(IfxStatementInfo *state, int attnum, char *buf);
char *ifxGetBigInt(IfxStatementInfo *state, int attnum, char *buf);
char *ifxGetDateAsString(IfxStatementInfo *state, int ifx_attnum,
						 char *buf);
char *ifxGetTimestampAsString(IfxStatementInfo *state, int ifx_attnum,
							  char *buf);
char ifxGetBool(IfxStatementInfo *state, int ifx_attnum);
short ifxGetInt2(IfxStatementInfo *state, int attnum);
int ifxGetInt4(IfxStatementInfo *state, int attnum);
char *ifxGetText(IfxStatementInfo *state, int attnum);
char *ifxGetTextFromLocator(IfxStatementInfo *state, int ifx_attnum,
							long *loc_buf_len);
char *ifxGetDecimal(IfxStatementInfo *state, int ifx_attnum,
					char *buf);
char *ifxGetIntervalAsString(IfxStatementInfo *state, int ifx_attnum,
							 char *buf);

/*
 * Functions to copy values into an Informix SQLDA structure.
 */
void ifxSetFloat(IfxStatementInfo *info,
				 int ifx_attnum,
				 char *buf);
void ifxSetDecimal(IfxStatementInfo *state, int ifx_attnum, char *value);
void ifxSetInteger(IfxStatementInfo *info, int ifx_attnum, int value);
void ifxSetInt8(IfxStatementInfo *info, int ifx_attnum, char *value);
void ifxSetBigint(IfxStatementInfo *info, int ifx_attnum, char *value);
void ifxSetInt2(IfxStatementInfo *info, int ifx_attnum, short value);
void ifxSetTimestampFromString(IfxStatementInfo *info, int ifx_attnum,
							   char *dtstring);
void ifxSetTimeFromString(IfxStatementInfo *info, int ifx_attnum,
                          char *timestr);
void ifxSetDateFromString(IfxStatementInfo *info,
						  int ifx_attnum,
						  char *datestr);
void ifxSetText(IfxStatementInfo *info, int ifx_attnum, char *value);
void ifxSetSimpleLO(IfxStatementInfo *info, int ifx_attnum, char *buf,
					int buflen);
void ifxSetIntervalFromString(IfxStatementInfo *info, int ifx_attnum,
							  char *format,
							  char *instring);

/*
 * Helper functions, basically wrap esql/c API functions
 * into accessible wrapper functions without the need
 * to include incl/esql/sqltypes.h...
 */
short ifxIsColumnNullable(short typeid);
short ifxSQLType(short typeid);
void ifxDecodeColumnLength(short typeid, short collength,
						   short *min, short *max);
short ifxCharColumnLen(short typeid, short collength);
short ifxMaskTypeId(short typeid);

/*
 * Helper macros.
 */
#define SQLCA_WARN_SET 0
#define SQLCA_WARN_TRANSACTIONS 1
#define SQLCA_WARN_ANSI 2
#define SQLCA_WARN_NO_IFX_SE 3
#define SQLCA_WARN_FLOAT_IS_DEC 4
#define SQLCA_WARN_RESERVED 5 /* not used */
#define SQLCA_WARN_REPLICATED_DB 6
#define SQLCA_WARN_DB_LOCALE_MISMATCH 7
#define SQLCA_WARN(a) sqlca.sqlwarn.sqlwarn##a

#define SQLCA_NROWS_PROCESSED 0
#define SQLCA_NROWS_WEIGHT    3

#endif
