#include <stdhdrs.h>
#include <procreg.h>
#include <deadstart.h>
#define USE_SQLITE_INTERFACE
#define BUILDS_INTERFACE
#include "../SQLlib/sqlstruc.h"
#include "sqlite3.h"

struct sqlite_interface my_sqlite_interface = {
	sqlite3_result_text
														 , sqlite3_user_data
														 , sqlite3_last_insert_rowid
														 , sqlite3_create_function
														 , sqlite3_get_autocommit
														 , sqlite3_open
														 , sqlite3_errmsg
														 , sqlite3_finalize
														 , sqlite3_close
                                           , sqlite3_prepare_v2
                                           , sqlite3_step
                                           , sqlite3_column_name
                                           , sqlite3_column_text
                                           , sqlite3_column_bytes
                                           , sqlite3_column_type
														 , sqlite3_column_count
                                           , sqlite3_config
                                           , sqlite3_db_config
};


static POINTER CPROC GetSQLiteInterface( void )
{
   //RealSQLiteInterface._global_font_data = GetGlobalFonts();
   return &my_sqlite_interface;
}

static void CPROC DropSQLiteInterface( POINTER p )
{
}

PRIORITY_PRELOAD( RegisterSQLiteInterface, DEFAULT_PRELOAD_PRIORITY-1 )
{
	RegisterInterface( WIDE("sqlite3"), GetSQLiteInterface, DropSQLiteInterface );

}
