/*
 *  db.c
 *  Copyright: FortuNet, Inc.  2006
 *  Product:  BingLink      Project: alpha2server   Version: Alpha2
 *  Contributors:    Jim Buckeyne, Christopher Green
 *  Stipulates database table fields and formats and creates them if incorrect or missing
 *
 */


#include <stdhdrs.h>
#include <deadstart.h>
#include <pssql.h>

/*
char *location_table = "CREATE TABLE `location` ("
"  `ID` int(11) NOT NULL auto_increment,"
"  `name` varchar(100) NOT NULL default '',"
"  PRIMARY KEY  (`ID`),"
"  KEY `namekey` (`name`)"
") TYPE=MyISAM; ";
 */

FIELD link_hall_fields[] = { { "id", "int", "NOT NULL auto_increment" }
									, { "hall_id" , "int", "NOT NULL default '0'" }
									, { "enabled"     , "int", "NOT NULL default '0'" }
									, { "controller" , "int", "NOT NULL default '0'" }
									, { "master_ready" , "int", "NOT NULL default '0'" }
									, { "delegate_ready" , "int", "NOT NULL default '0'" }
									, { "participating" , "int", "NOT NULL default '0'" }
									, { "task_launched" , "int", "NOT NULL default '0'" }
									, { "announcing" , "int", "NOT NULL default '0'" }
									, { "reset_state" , "int", "NOT NULL default '0'" }
									, { "standing_alone" , "int", "NOT NULL default '0'" }
									, { "timestamp"  , "timestamp" }
									, { "announcement", "varchar(65)", "NOT NULL default 'NONE'" }
									, { "prohibited" , "int", "NOT NULL default '0'" }
									, { "dvd_active" , "int", "NOT NULL default '0'" }
									, { "media_active" , "int", "NOT NULL default '0'" }
};

// required key def.

DB_KEY_DEF keys[] = { {  .flags={.bPrimary=1} , NULL, KEY_COLUMNS("id")  }
                    , {  .flags={.bUnique=1} , "hallkey", KEY_COLUMNS("hall_id")  }
                    };

TABLE link_hall_state_table = { "link_hall_state", FIELDS( link_hall_fields ), TABLE_KEYS( keys ) };

//-------------
FIELD link_state_fields[] = { { "id", "int", "NOT NULL auto_increment" }
									 , { "master_hall_id", "int", "NOT NULL default '0'" }
									 , { "delegated_master_hall_id", "int", "NOT NULL default '0'" }
									 , { "controller_hall_id", "int", "NOT NULL default '0'" }
									 , { "bingoday", "date", "NOT NULL default '0000-00-00'" }
};
DB_KEY_DEF ID_key[] = { {  .flags={.bPrimary=1} , NULL, KEY_COLUMNS("id") }
								, { .flags={.bUnique=1}, "daykey", {"bingoday"} }
                    };
TABLE link_state_table = { "link_state", FIELDS( link_state_fields ), TABLE_KEYS( ID_key ) };

//-------------
FIELD link_alive_fields[] = { { "hall_id", "int", NULL }
									 , { "last_alive", "timestamp", NULL }
};
DB_KEY_DEF hall_id_key[] = { {  .flags={.bPrimary=1} , NULL, KEY_COLUMNS("hall_id") }
                    };
TABLE link_alive_table = { "link_alive", FIELDS( link_alive_fields ), TABLE_KEYS( hall_id_key ) };


//-------------
FIELD hall_systems_fields[] = { { "id", "int", "NOT NULL auto_increment" }
										, { "hall_id", "int", "NOT NULL default '0'" }
										, { "system_id", "int", "NOT NULL default '0'" }
                           };

DB_KEY_DEF hall_systems_key[] = { {  .flags={.bPrimary=1} , NULL, KEY_COLUMNS("id") }
                    };
TABLE hall_systems_table = { "hall_systems", FIELDS( hall_systems_fields ), TABLE_KEYS( hall_systems_key ) };

//-------------
FIELD location_fields[] = { { "ID", "int", "NOT NULL auto_increment" }
								  , { "name", "varchar(100)", "NOT NULL default ''" }
								  , { "packed_name", "varchar(100)", "NOT NULL default ''" }
								  , { "address_bdata", "varchar(100)", "NOT NULL default ''" }
								  , { "address_video", "varchar(100)", "NOT NULL default ''" }
								  , { "address_host_access", "varchar(100)", "NOT NULL default ''" }
                           };
DB_KEY_DEF location_keys[] = { {  .flags={.bPrimary=1} , NULL, KEY_COLUMNS("id") }
									  , { .flags={.bUnique=1}, "namekey", KEY_COLUMNS( "name" ) }
                    };

TABLE location_table = { "location", FIELDS( location_fields ), TABLE_KEYS( location_keys ) };

//-------------

void CheckMyTables( PODBC odbc )
{
	CheckODBCTable( odbc, &link_hall_state_table, CTO_MERGE );
	CheckODBCTable( odbc, &link_state_table, CTO_MERGE );
	CheckODBCTable( odbc, &link_alive_table, CTO_MERGE );
	CheckODBCTable( odbc, &hall_systems_table, CTO_MERGE );
	CheckODBCTable( odbc, &location_table, CTO_MERGE );
	{
		PTABLE table = GetFieldsInSQL( "create table link_system_layout ( link_system_layout_id int auto_increment, location_id int, is_sql_server int, remote_sql_server VARCHAR(64), PRIMARY KEY(link_system_layout_id))", 0 );
		CheckODBCTable( odbc, table, CTO_MERGE );
      DestroySQLTable( table );
	}
};


