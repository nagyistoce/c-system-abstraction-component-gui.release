/*
 *  server.h
 *  Copyright: FortuNet, Inc.  2006
 *  Product:  BingLink      Project: alpha2server   Version: Alpha2
 *  Contributors:    Jim Buckeyne, Christopher Green
 *  Main header file for alpha2server
 *
 */


#include <stdhdrs.h>
#include <timers.h>
//sack and altanik stuff

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <stddef.h>
#include <stdlib.h>

#include <pssql.h>

#include <signal.h>
#include <system.h> //gotta have for PTASK_INFO


#include <network.h>

#include <render.h>// for the interface.
#include <psi.h>//// for the interface.?

#define lpszFilename "server.ini"


#ifdef BINGLINKSERVER_SOURCE
#  define BINGLINKSERVER_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#  define BINGLINKSERVER_PROC(type,name) IMPORT_METHOD type CPROC name
#endif

#define USE_IMAGE_INTERFACE g.pii
#define USE_RENDER_INTERFACE g.pdi

#define g global_binglink_data

typedef struct{

	struct{
		_32 bExit:1;
		_32 bDeleteLinkOnOpenTCPFail:1;
		BIT_FIELD bUseBingoDay : 1;
		BIT_FIELD bReadOnly : 1;
	}flags;

	//PTHREAD pMainThread;
	PTASK_INFO task;
   CTEXTSTR MyLoadPath;
   PLIST tasks;
   PIMAGE_INTERFACE pii;
   PRENDER_INTERFACE pdi;
	PODBC odbc;
   _32 poll_delay;
} GLOBAL;

#ifndef DECLARE_GLOBAL
extern
#endif
GLOBAL global_binglink_data;


#define l local_binglink_data

typedef struct
{
	struct {
		_32 bConnected : 1; // state flag of bdata connection to this hall
		_32 bHostAssigned: 1;
		_32 bBdataAssigned: 1;
		_32 bAssignedUDP : 1;
		_32 uiBdataFailures;
		BIT_FIELD bParticipating : 1;
		BIT_FIELD bHosting : 1;
	} flags;
	_32 hall_systems_id, hall_id, enabled, controller, participating, master_ready, delegate_ready, prohibited, dvd_active,media_active;
	_32 task_launched;
   LOGICAL reset_state;
	LOGICAL bSoundPlaying;
   char announcement[65];
   PCLIENT pcBdata;
   _32 nTickClosed;
}LINK_HALL_STATE, *PLINK_HALL_STATE;

typedef struct binghall_struct {
	struct{
//		char szSiteName[128];
//		char szVideoAddr[128];
//		char szBdataAddr[128];
		CTEXTSTR szSiteName;
		CTEXTSTR szVideoAddr;
		CTEXTSTR szBdataAddr;
		//_32 uiState;
	} stIdentity;

	LINK_HALL_STATE LinkHallState;

   //PTASK_INFO task;
}BINGHALL, *PBINGHALL;

typedef struct
{
	_32 master_hall_id, delegated_master_hall_id, controller_hall_id;

}LINK_STATE, *PLINK_STATE;



typedef struct{

	char **env;

	struct{
		char szDisplay[400];
		char szBroadcast[400];
		char szSound[400];
		char szExtendedScriptSupportPath[400];
	} command;

	PLIST pHallList; // list of PBINGHALL structures

	struct {
		PLIST pLinkHallStates;

		LINK_STATE LinkState;
      
	} last_state;


	struct {
		PLIST pLinkHallStates;
		LINK_STATE LinkState;
		LINK_HALL_STATE LinkHallState;
      
	} current_state;

	PLIST pLinkHallStates;
	PTHREAD check_state_thread; // wake this thread to kick the state check process...
	CTEXTSTR hall_name;
	PBINGHALL pMyHall;
	PLIST buttons;

	struct {
		BIT_FIELD use_events : 1;
	} flags;
}LOCAL;

#ifndef DECLARE_LOCAL
extern
#endif
LOCAL local_binglink_data;


//in bdata_tcp_relay.c
void InitBdataService( void );
