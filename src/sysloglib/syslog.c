/*
 *  Crafted by James Buckeyne
 *
 *   (c) Freedom Collective 2000-2006++
 *
 *   created to provide standard logging features
 *   lprintf( format, ... ); simple, basic
 *   if DEBUG, then logs to a file of the same name as the program
 *   if RELEASE most of this logging goes away at compile time.
 *
 *  standardized to never use int.
 *
 * see also - include/logging.h
 *
 */
#define COMPUTE_CPU_FREQUENCY

//#undef UNICODE
#ifdef BCC16
#include <winsock.h>
#else
#include <loadsock.h>
#endif

#ifdef __LCC__
#include <intrinsics.h>
#endif
#ifdef BCC16
#include <time.h>
#endif

#ifdef __UNIX__
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sharemem.h> // is bad read ptr
#include <stdarg.h>
#endif

#include <stdhdrs.h>
#ifdef __WINDOWS__
#include <io.h> // unlink
#endif
#include <stdio.h>
#include <sack_types.h>
#include <deadstart.h>

#include <idle.h>
#include <logging.h>

// okay this brings TIGHT integration.... but standardization for core levels.
#include <filesys.h>
#ifndef __NO_OPTIONS__
#include <sqlgetoption.h>
#endif
#ifdef __cplusplus
LOGGING_NAMESPACE
#endif


static _64 cpu_tick_freq;
// flags that control the operation of system logging....
static struct state_flags{
	_32 bInitialized : 1;
	_32 bUseDay : 1;
	_32 bUseDeltaTime : 1;
	_32 bLogTime : 1;
	_32 bLogHighTime : 1;
	_32 bLogCPUTime : 1;
	_32 bProtectLoggedFilenames : 1;
	_32 bLogProgram : 1;
	_32 bLogThreadID : 1;
	_32 bLogOpenAppend : 1;
	_32 bLogOpenBackup : 1;
	_32 bLogSourceFile : 1;
//} flags = { 0,0,1,1,0,1,0,0,0}; // log delta cpu time.(no protect), log program too
//} flags = { 0,0,1,1,0,1,1,1,1}; // delta cpu time, threadID, process name, protect logged filenames(slower)

// deadstart with localtime conversion(to get timeofday) sucks.
// okay it's alright... as long as we don't log TOOOO early
#ifdef _MSC_VER
#define _LOG_FULL_FILE_NAMES
} flags = { 0,0,0,0,0,0,0,0,0}; // normal time protect logged filenames(slower)
// log CPU time....
//} flags = { 0,0,1,1,0,1,0,0,0,0,0}; // normal time protect logged filenames(slower)
#else
} flags = { 0,0,1,1,1,0,1,1,1,0,0}; // cpu time delta protect logged filenames(slower)
//} flags = { 0,0,0,0,0,0,1,1,0,0,0}; // normal time protect logged filenames(slower)
#endif

// a conserviative minimalistic configuration...
//} flags = { 0,0,1,0,1,0,1,1,0};

static TEXTCHAR *pProgramName;
static UserLoggingCallback UserCallback;

static enum syslog_types logtype = SYSLOG_NONE;

#ifdef _DEBUG
static _32 nLogLevel = LOG_NOISE + 1000; // default log EVERYTHING
#else
static _32 nLogLevel = LOG_NOISE-1; // default log EVERYTHING
#endif
static _32 nLogCustom = 0; // bits enabled and disabled for custom mesasges...
static CTEXTSTR gFilename;// = "LOG";
static FILE *file;
static SOCKET   hSock = INVALID_SOCKET;
static int bCPUTickWorks = 1; // assume this works, until it fails
static _64 tick_bias;

//----------------------------------------------------------------------------
//#if !defined(__STATIC__) && ( defined( _WIN32 ) || defined( BCC16 ) )
#ifndef BAG
#ifndef __STATIC__
LIBMAIN()
{
	return 1;
}
LIBEXIT()
{
	return 1;
}
LIBMAIN_END();
#endif
#endif


// we should really wait until the very end to cleanup?
PRIORITY_ATEXIT( CleanSyslog, ATEXIT_PRIORITY_SYSLOG )
{
	enum syslog_types _logtype = logtype;
	if( ( _logtype == SYSLOG_AUTO_FILE && file ) || ( _logtype != SYSLOG_AUTO_FILE && _logtype == SYSLOG_NONE ) )
		lprintf( WIDE( "Final log - syslog closed." ) );
   return;
	pProgramName = NULL; // this was dynamic allocated memory, and it is now gone.
	if( ( _logtype == SYSLOG_AUTO_FILE && file ) || ( _logtype != SYSLOG_AUTO_FILE && _logtype == SYSLOG_NONE ) )
		lprintf( WIDE( "Final log - syslog closed." ) );
	logtype = SYSLOG_NONE;
	switch( _logtype )
	{
	case SYSLOG_FILE:
	case SYSLOG_FILENAME:
      fclose( file );
		break;
	case SYSLOG_UDP:
	case SYSLOG_UDPBROADCAST:
		closesocket( hSock );
		hSock = INVALID_SOCKET;
		break;
	default:
      // else... no resources to cleanup
      break;
	}
}


#if 0
				/*
				 * this code would ideally check to see if
				 * the cpu rdtsc instruction worked....
				 * someday we should consider using the rdtscp instruction
				 * but that will require fetching CPU characteristics
             * - SEE mmx.asm in src/imglib/
             */
void TestCPUTick( void )
{
	_64 tick, _tick;
	int n;
	bCPUTickWorks = 1;
	_tick = tick = GetCPUTick();
	for( n = 0; n < 10000000; n++ )
	{
#ifdef GCC
      //asm( "cpuid\n" );
#endif
		tick = GetCPUTick();
		if( tick > _tick )
		{
			//lprintf( "%020Ld %020Ld", _tick, tick );
			_tick = tick;
		}
		else
		{
         lprintf( "CPU TICK FAILED!" );
			bCPUTickWorks = 0;
         break;
		}
		Relinquish();
	}
}
#endif

_64 GetCPUTick(void )
{
/*
 * being the core of CPU tick layer type stuff
 * this should result in ticks, and fail ticks
 * to return reasonable defaults...
 * I guess there should be a tick_base to result
 * the same type of number when it does go backwards
 */
	if( bCPUTickWorks )
	{
      static _64 lasttick;
#if defined( __LCC__ )
		return _rdtsc();
#elif defined( __WATCOMC__ )
		extern void GetCPUTicks3();
		_64 tick;
#ifndef __WATCOMC__
		// haha a nasty compiler trick to get the variable used
		// but it's also a 'meaningless expression' so watcom pukes.
		(1)?(0):(tick = 0);
#endif
#pragma aux GetCPUTicks3 = "rdtsc"  \
	"mov dword ptr tick, eax"     \
	"mov dword ptr tick+4, edx "
		GetCPUTicks3();
		if( !lasttick )
			lasttick = tick;
		else if( tick < lasttick )
		{
			bCPUTickWorks = 0;
			cpu_tick_freq = 1;
			tick_bias = lasttick - ( timeGetTime()/*GetTickCount()*/ * 1000 );
         tick = lasttick + 1; // more than prior, but no longer valid.
		}
      lasttick = tick;
		return tick;
#elif defined( _MSC_VER )
#ifdef _M_CEE_PURE
	  //return System::DateTime::now;
	  return 0;
#else
		_64 tick;
		_asm rdtsc;
		_asm mov dword ptr [tick], eax;
		_asm mov dword ptr [tick + 4], edx;
		if( !lasttick )
			lasttick = tick;
		else if( tick < lasttick )
		{
			bCPUTickWorks = 0;
			cpu_tick_freq = 1;
			tick_bias = lasttick - ( timeGetTime()/*GetTickCount()*/ * 1000 );
         tick = lasttick + 1; // more than prior, but no longer valid.
		}
      lasttick = tick;
		return tick;
#endif
#elif defined( GCC ) && !defined( __arm__ )
		union {
			_64 tick;
			PREFIX_PACKED struct { _32 low, high; } PACKED parts;
		}tick;
		asm( WIDE("rdtsc\n") : "=a"(tick.parts.low), "=d"(tick.parts.high) );
		if( !lasttick )
			lasttick = tick.tick;
		else if( tick.tick < lasttick )
		{
			bCPUTickWorks = 0;
			cpu_tick_freq = 1;
			tick_bias = lasttick - ( timeGetTime()/*GetTickCount()*/ * 1000 );
         tick.tick = lasttick + 1; // more than prior, but no longer valid.
		}
      lasttick = tick.tick;
		return tick.tick;
#else
		DebugBreak();
#endif
	}
	return tick_bias + (timeGetTime()/*GetTickCount()*/ * 1000);
}

_64 GetCPUFrequency( void )
{
#ifdef COMPUTE_CPU_FREQUENCY
	{
		_64 cpu_tick, _cpu_tick;
		_32 tick, _tick;
      cpu_tick = _cpu_tick = GetCPUTick();
		tick = _tick = timeGetTime()/*GetTickCount()*/;
		cpu_tick_freq = 0;
		while( bCPUTickWorks && ( ( tick = timeGetTime()/*GetTickCount()*/ ) - _tick ) < 250 );
		{
			cpu_tick = GetCPUTick();
		}
      if( bCPUTickWorks )
			cpu_tick_freq = ( ( cpu_tick - _cpu_tick ) / ( tick - _tick ) )  / 1000; // microseconds;
	}
#else
	cpu_tick_freq = 1;
#endif
   return cpu_tick_freq;
}

char filepath[256];

void SetDefaultName( char *extra )
{
	char *newpath;
	size_t len;
	newpath = (char*)malloc( len = 9 + strlen( filepath ) + (extra?strlen(extra):0) + 5 );
#ifdef __cplusplus_cli
	snprintf( newpath, len, "%s%s.cli.log", filepath, extra?extra:"" );
#else
	snprintf( newpath, len, "%s%s.log", filepath, extra?extra:"" );
#endif
	gFilename = strdup( newpath ); // use the C heap.
	free( newpath ); // get rid of this ...
}

#ifndef __NO_OPTIONS__
static void LoadOptions( char *filename )
{
   // this overrides options with options available from SQL database.
	if( SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Default Log Location is current directory", 0, TRUE ) )
	{
		// override filepath, if log exception.
		TEXTSTR program = strdup( filename );
		TEXTCHAR buffer[256];
		GetCurrentPath( buffer, sizeof( buffer ) );
		snprintf( filepath, sizeof( filepath ), "%s/%s", buffer, program );
      SetDefaultName( NULL );
		Release( program );
	}
	else
	{
		TEXTCHAR buffer[256];
		// if this is blank, then length result from getprofilestring is 0, and default is with the program.
		// so I'll lave functionality as expected for a default.
		SACK_GetProfileStringEx( GetProgramName(), "SACK/Logging/Default Log Location", "", buffer, sizeof( buffer ), TRUE );
		if( buffer[0] )
		{
			TEXTSTR program = strdup( filename );
			snprintf( filepath, sizeof( filepath ), "%s/%s", buffer, program );
			SetDefaultName( NULL );
			Release( program );
		}
	}


	nLogLevel = SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Default Log Level (1001:all, 100:least)", nLogLevel, TRUE );
	flags.bLogThreadID = SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log Thread ID", 0, TRUE );
	flags.bLogProgram = SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log Program", 0, TRUE );
	flags.bLogSourceFile = SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log Source File", 1, TRUE );
	if( SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log CPU Tick Time and Delta", 0, TRUE ) )
	{
		SystemLogTime( SYSLOG_TIME_CPU|SYSLOG_TIME_DELTA );
	}
	else if( SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log Time as Delta", 0, TRUE ) )
	{
		SystemLogTime( SYSLOG_TIME_HIGH|SYSLOG_TIME_DELTA );
	}
	else if( SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log Time", 1, TRUE ) )
	{
		if( SACK_GetProfileIntEx( GetProgramName(), "SACK/Logging/Log Date", 1, TRUE ) )
		{
			SystemLogTime( SYSLOG_TIME_LOG_DAY|SYSLOG_TIME_HIGH );
		}
		else
			SystemLogTime( SYSLOG_TIME_HIGH );
	}
	else
		SystemLogTime( 0 );
}
#endif

#ifdef __WATCOM_CPLUSPLUS__
#pragma initialize 35
#endif
PRIORITY_PRELOAD( InitSyslog, SYSLOG_PRELOAD_PRIORITY )
{
#ifdef _WIN32
   CTEXTSTR filename;
	// make sure we only read up to 4 ... might have to add our own .log entirely
	// or maybe it's a dot only file extension?
	{
		TEXTCHAR *ext;
		// computes default filepath  (where logs are)
      // and current filename
		GetModuleFileName( NULL, filepath, sizeof( filepath ) - 4 );
		ext = strrchr( filepath, '.' );
		if( ext )
		{
			ext[0] = 0;
		}
		filename = pathrchr( filepath );
		if( filename )
			filename++;
		else
			filename = filepath;
	}
#  ifdef UNICODE
   //lprintf( "This will be bad." ); ... not really it's allocated in non (tmp) heap
	pProgramName = strdup( filename );
#  else
	pProgramName = strdup( filename );
#  endif

	SetDefaultName( NULL );
#else
	//logtype = SYSLOG_FILE;
	//file = stderr;
	//fprintf( stderr, WIDE("Syslog Initializing, debug mode, startup stderr\n") );
	{
		//char filename[256];
		//char buf[256], 
		char *pb;
		int n;
		n = readlink("/proc/self/exe",filepath,256);
		if( n >= 0 )
		{
			filepath[n]=0; //linux
			if( !n )
			{
				strcpy( filepath, WIDE(".") );
				filepath[ n = readlink( WIDE("/proc/curproc/"),filepath,sizeof(filepath))]=0; // fbsd
			}
		}
		else
			strcpy( filepath, WIDE(".")) ;
		pb = strrchr(filepath,'/');
		if( pb )
		{
			//pb[0]=0;
			pb++;
		}
#  ifdef UNICODE
		pProgramName = strdup( pb );
#  else
		pProgramName = strdup( pb );
#  endif
      SetDefaultName( NULL );
	}
#endif

#ifdef _DEBUG
   // actually this is unusable in _MSC_VER.
#  if defined( _MSC_VER )
	logtype = SYSLOG_SYSTEM;
#  else
#    ifdef _WIN32

#       ifdef  __WATCOMC__
	// this is just a quick way to stub this out... based on compiler switches.
   // very sorry this causes 'expression in statement always true'
	if( 1 )
#       else
	// this is just a quick way to stub this out... based on compiler switches.
   // very sorry this causes 'expression in statement always false'
	if( 0 )
#       endif
	{
		/* using SYSLOG_AUTO_FILE option does not require this to be open.
		 * it is opened on demand.
       */
		logtype = SYSLOG_AUTO_FILE;
      flags.bLogOpenAppend = 0;
		flags.bLogOpenBackup = 1;
		flags.bLogSourceFile = 1;
		flags.bLogProgram = 0;
      SystemLogTime( SYSLOG_TIME_HIGH );
		//lprintf( WIDE("Syslog Initializing, debug mode, startup programname.log\n") );
	}
	else
	{
      logtype = SYSLOG_UDPBROADCAST;
	}
#    else
	{
		logtype = SYSLOG_AUTO_FILE;
		flags.bLogOpenAppend = 0;
      flags.bLogOpenBackup = 1;
		flags.bLogSourceFile = 1;
      SystemLogTime( SYSLOG_TIME_CPU|SYSLOG_TIME_DELTA );
	}
#    endif
#  endif
#else
    // stderr?
	logtype = SYSLOG_NONE;
   file = NULL;
#endif

#ifndef __NO_OPTIONS__
	LoadOptions( pProgramName );
#endif

	flags.bInitialized = 1;
}

//#endif

//----------------------------------------------------------------------------
CTEXTSTR GetTimeEx( int bUseDay )
{
	static TEXTCHAR timebuffer[256];
   /* used by sqlite extension to support now() */
#ifdef _WIN32
#ifndef WIN32
#define WIN32 _WIN32
#endif
#endif

#if defined( WIN32 )
   SYSTEMTIME st;
	GetLocalTime( &st );

	if( bUseDay )
	   snprintf( timebuffer, sizeof(timebuffer), WIDE("%02d/%02d/%d %02d:%02d:%02d")
   	                  , st.wMonth, st.wDay, st.wYear
      	               , st.wHour, st.wMinute, st.wSecond );
	else
	   snprintf( timebuffer, sizeof(timebuffer), WIDE("%02d:%02d:%02d")
      	               , st.wHour, st.wMinute, st.wSecond );

#else
	struct tm *timething;
	time_t timevalnow;
	time(&timevalnow);
	timething = localtime( &timevalnow );
	strftime( timebuffer
				, sizeof( timebuffer )
				, (bUseDay)?"%m/%d/%Y %H:%M:%S":"%H:%M:%S"
				, timething );
#endif
	return timebuffer;
}

CTEXTSTR GetTime( void )
{
   return GetTimeEx( flags.bUseDay );
}



CTEXTSTR GetPackedTime( void )
{
	static TEXTCHAR timebuffer[256];
   /* used by sqlite extension to support now() */
#ifdef _WIN32
#ifndef WIN32
#define WIN32 _WIN32
#endif
#endif

#if defined( WIN32 )
   SYSTEMTIME st;
	GetLocalTime( &st );

	snprintf( timebuffer, sizeof(timebuffer), WIDE("%04d%02d%02d%02d%02d%02d")
			  , st.wYear
			  , st.wMonth, st.wDay
			  , st.wHour, st.wMinute, st.wSecond );

#else
	struct tm *timething;
	time_t timevalnow;
	time(&timevalnow);
	timething = localtime( &timevalnow );
	strftime( timebuffer
				, sizeof( timebuffer )
				, "%Y%m%d%H%M%S"
				, timething );
#endif
	return timebuffer;
}

//----------------------------------------------------------------------------
#ifndef BCC16 // no gettime of day - no milliseconds
static TEXTCHAR *GetTimeHigh( void )
{
    static TEXTCHAR timebuffer[256];
#ifdef WIN32
	static SYSTEMTIME _st;
	SYSTEMTIME st, st_save;
	if( flags.bUseDeltaTime )
	{
		GetLocalTime( &st );
		st_save = st;
		if( !_st.wYear )
			_st = st;
		st.wMilliseconds -= _st.wMilliseconds;
		if( st.wMilliseconds & 0x8000 )
		{
			st.wMilliseconds += 1000;
			st.wSecond--;
		}
		st.wSecond -= _st.wSecond;
		if( st.wSecond & 0x8000 )
		{
			st.wSecond += 60;
			st.wMinute--;
		}
		st.wMinute -= _st.wMinute;
		if( st.wMinute & 0x8000 )
		{
			st.wMinute += 60;
			st.wHour--;
		}
		st.wHour -= _st.wHour;
		if( st.wHour & 0x8000 )
			st.wHour += 24;
		_st = st_save;
	}
	else
		GetLocalTime( &st );

	if( flags.bUseDay )
	   snprintf( timebuffer, sizeof(timebuffer), WIDE("%02d/%02d/%d %02d:%02d:%02d.%03d")
   	        , st.wMonth, st.wDay, st.wYear
      	     , st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
	else
	   snprintf( timebuffer, sizeof(timebuffer), WIDE("%02d:%02d:%02d.%03d")
				  , st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
#else
	static struct timeval _tv;
   static struct tm _tm;
	struct timeval tv, tv_save;
   struct tm *timething, tm, tm_save;
   int len;
	gettimeofday( &tv, NULL );
	if( flags.bUseDeltaTime )
	{
		tv_save = tv;
		timething = localtime( &tv.tv_sec );
		tm = tm_save = *timething;
		if( !_tm.tm_year )
		{
			_tm = *timething;
         _tv = tv;
		}
		tv.tv_usec -= _tv.tv_usec;
		if( tv.tv_usec < 0 )
		{
			tv.tv_usec += 1000000;
         tm.tm_sec--;
		}
		tm.tm_sec -= _tm.tm_sec;
		if( tm.tm_sec < 0 )
		{
			tm.tm_sec += 60;
         tm.tm_min--;
		}
		tm.tm_min -= _tm.tm_min;
		if( tm.tm_min < 0 )
		{
			tm.tm_min += 60;
         tm.tm_hour--;
		}
		tm.tm_hour -= _tm.tm_hour;
		if( tm.tm_hour < 0 )
			tm.tm_hour += 24;

		_tm = tm_save;
		_tv = tv_save;
	}
	else
	{
		timething = localtime( &tv.tv_sec );
		tm = *timething;
	}

	len = strftime( timebuffer
                  , sizeof( timebuffer )
                  , (flags.bUseDay)?"%m/%d/%Y %H:%M:%S":"%H:%M:%S"
                  , &tm );
	sprintf( timebuffer + len, WIDE(".%03ld"), tv.tv_usec / 1000 );
   /*
    // this code is kept in case borland's compiler don't like it.
    {
    time_t timevalnow;
    time(&timevalnow);
    timething = localtime( &timevalnow );
    strftime( timebuffer
    , sizeof( timebuffer )
    , WIDE("%m/%d/%Y %H:%M:%S.000")
    , timething );
    }
    */
#endif
	return timebuffer;
}
#else
#define GetTimeHigh GetTime
#endif

_32 ConvertTickToMicrosecond( _64 tick )
{
	if( bCPUTickWorks )
	{
		if( !cpu_tick_freq )
			GetCPUFrequency();
		if( !cpu_tick_freq )
			return 0;
		return (_32)(tick / cpu_tick_freq);
	}
	else
      return (_32)tick;
}




void PrintCPUDelta( TEXTCHAR *buffer, int buflen, _64 tick_start, _64 tick_end )
{
#ifdef COMPUTE_CPU_FREQUENCY
	if( !cpu_tick_freq )
		GetCPUFrequency();
	if( cpu_tick_freq )
		snprintf( buffer, buflen, WIDE("%")_64fs WIDE(".%03") _64f
				 , ((tick_end-tick_start) / cpu_tick_freq ) / 1000
				 , ((tick_end-tick_start) / cpu_tick_freq ) % 1000
				 );
   else
#endif
		snprintf( buffer, buflen, WIDE("%")_64fs, tick_end - tick_start
		   	 );
}

static TEXTCHAR *GetTimeHighest( void )
{
	static _64 lasttick;
	_64 tick;
	static TEXTCHAR timebuffer[64];
	tick = GetCPUTick();
	if( !lasttick )
		lasttick = tick;
	if( flags.bUseDeltaTime )
	{
		int ofs = sprintf( timebuffer, WIDE("%20lld") WIDE(" "), tick );
		PrintCPUDelta( timebuffer + ofs, sizeof( timebuffer ), lasttick, tick );
		lasttick = tick;
	}
	else
		sprintf( timebuffer, WIDE("%20") _64fs, tick );
	// have to find a generic way to get this from _asm( rdtsc );
	return timebuffer;
}

static CTEXTSTR GetLogTime( void )
{
	if( flags.bLogTime )
	{
		if( flags.bLogHighTime )
			return GetTimeHigh();
		else if( flags.bLogCPUTime )
			return GetTimeHighest();
		else
			return GetTime();
	}
	return WIDE("");
}

//----------------------------------------------------------------------------
#ifndef FBSD
static SOCKADDR saLogBroadcast  = { 2, { 0x02, 0x02, (char)0xff, (char)0xff, (char)0xff, (char)0xff } };
static SOCKADDR saLog  = { 2, { 0x02, 0x02, 0x7f, 0x00, 0x00, 0x01 } };
static SOCKADDR saBind = { 2, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
#else
static SOCKADDR saLog  = { 2, 0x02, 0x02, 0x7f, 0x00, 0x00, 0x01  };
static SOCKADDR saBind = { 2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  };
#endif
static LOGICAL      bStarted;
static LOGICAL bLogging;

static void UDPSystemLog( const TEXTCHAR *message )
{
	while( bLogging )
		Idle();
	bLogging = 1;
	if( !bStarted )
	{
#ifdef _WIN32
#ifndef MAKEWORD
#define MAKEWORD(a,b) (((a)<<8)|(b))
#endif
		WSADATA ws;  // used to start up the socket services...
		if( WSAStartup( MAKEWORD(1,1), &ws ) )
		{
			bLogging = 0;
			return;
		}
#endif
		bStarted = TRUE;
	}
	if( hSock == INVALID_SOCKET )
	{
		LOGICAL bEnable = TRUE;
		hSock = socket(PF_INET,SOCK_DGRAM,0);
		if( hSock == INVALID_SOCKET )
		{
			bLogging = 0;
			return;
		}
		if( bind(hSock,&saBind,sizeof(SOCKADDR)) )
		{
			closesocket( hSock );
			hSock = INVALID_SOCKET;
			bLogging = 0;
			return;
		}
#ifndef BCC16
		if( setsockopt( hSock, SOL_SOCKET
						  , SO_BROADCAST, (char*)&bEnable, sizeof( bEnable ) ) )
		{
			Log( WIDE("Failed to set sock opt - BROADCAST") );
		}
#endif
	}
	{
		INDEX nSent;
		int nSend;
		static TEXTCHAR realmsg[1024];
		nSend = snprintf( realmsg, sizeof( realmsg ) / sizeof( realmsg[0] ), /*"[%s]"*/ WIDE("%s")
				  //, pProgramName
				  , message );
		message = realmsg;
#ifdef __cplusplus_cli
		char *tmp = CStrDup( realmsg );
#define SENDBUF tmp
#else
#define SENDBUF message
#endif
		nSent = sendto( hSock, SENDBUF, nSend, 0
						  ,(logtype == SYSLOG_UDPBROADCAST)?&saLogBroadcast:&saLog, sizeof( SOCKADDR ) );
#ifdef __cplusplus_cli
		Release( tmp );
#endif
      if( logtype != SYSLOG_UDPBROADCAST )
			Relinquish(); // allow logging agents time to pick this up...
	}
	bLogging = 0;
}

#ifdef __LINUX__
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

LOGICAL IsBadReadPtr( CPOINTER pointer, PTRSZVAL len )
{
	static FILE *maps;
	//return FALSE;
   //DebugBreak();
	if( !maps )
		maps = fopen( WIDE("/proc/self/maps"), WIDE("rt") );
	else
      fseek( maps, 0, SEEK_SET );
   //fprintf( stderr, WIDE("Testing a pointer..\n") );
	if( maps )
	{
      PTRSZVAL ptr = (PTRSZVAL)pointer;
		char line[256];
		while( fgets( line, sizeof(line)-1, maps ) )
		{
			PTRSZVAL low, high;
			sscanf( line, WIDE("%"PTRSZVALfx"-%"PTRSZVALfx""), &low, &high );
			//fprintf( stderr, WIDE("%s") WIDE("Find: %08") PTRSZVALfx WIDE(" Low: %08") PTRSZVALfx WIDE(" High: %08") PTRSZVALfx WIDE("\n")
			//		 , line, pointer, low, high );
			if( ptr >= low && ptr <= high )
			{
				//fclose( maps );
				return FALSE;
			}
		}
		//fclose( maps );
	}
   //fprintf( stderr, WIDE("%p is not valid. %d"), pointer, errno );
   return TRUE;
}

//---------------------------------------------------------------------------
#endif

static void FileSystemLog( CTEXTSTR message )
{
	if( file )
	{
		fputs( message, file );
		fprintf( file, WIDE("\n") );
		fflush( file );
	}
}


static void BackupFile( const TEXTCHAR *source, int source_name_len, int n )
{
	FILE *file;
	Fopen( file, source, WIDE("rt") );
	if( file )
	{
		TEXTCHAR backup[256];
		fclose( file );
		// move file to backup..
		snprintf( backup, sizeof( backup ), WIDE("%*.*s.%d")
				  , source_name_len
				  , source_name_len
				  , source, n );
		if( n < 10 )
		{
			BackupFile( backup
							, source_name_len
							, n+1 );
		}
		else
			unlink( source );
		rename( source, backup );
	}
}


static void DoSystemLog( const TEXTCHAR *buffer )
{
	if( logtype == SYSLOG_UDP
		|| logtype == SYSLOG_UDPBROADCAST )
		UDPSystemLog( buffer );
	else if( ( logtype == SYSLOG_FILE ) || ( logtype == SYSLOG_AUTO_FILE ) )
	{
		if( logtype == SYSLOG_AUTO_FILE )
			if( !file && gFilename )
			{
				int n_retry = 0;
retry_again:
				if( flags.bLogOpenBackup )
				{
					BackupFile( gFilename, (int)strlen( gFilename ), 1 );
				}
				else if( flags.bLogOpenAppend )
					Fopen( file, gFilename, WIDE("at+") );
				if( file )
					fseek( file, 0, SEEK_END );
            else
					Fopen( file, gFilename, WIDE("wt") );
				if( !file )
				{
					if( n_retry < 5 )
					{
						TEXTCHAR tmp[10];
						sprintf( tmp, "%d", n_retry++ );
						SetDefaultName( tmp );
						goto retry_again;
					}
					else
               // can't open the logging file, stop trying now, will save us trouble in the future
					logtype = SYSLOG_NONE;
				}
			}
		FileSystemLog( buffer );
	}
#ifdef _WIN32
	else if( logtype == SYSLOG_SYSTEM )
	{
#ifdef __cplusplus_cli
		// requires referenced xperdex.classes... if this doesn't compile, please add the reference
		xperdex::classes::Log::log( gcnew System::String( buffer ) );
		//System::Console::WriteLine( gcnew System::String( buffer ) );
		//System::Diagnostics::Debug
#else
		OutputDebugString( buffer );
		OutputDebugString( WIDE("\n") );
#endif
	}
#endif
	else if( logtype == SYSLOG_CALLBACK )
		UserCallback( buffer );
}

void SystemLogFL( const TEXTCHAR *message FILELINE_PASS )
{
	static TEXTCHAR buffer[4096];
	static TEXTCHAR threadid[32];
	static TEXTCHAR sourcefile[256];
	CTEXTSTR logtime;
	static _32 lock;
	while( LockedExchange( &lock, 1 ) ) Relinquish();
   logtime = GetLogTime();
	if( flags.bLogSourceFile )
	{
      snprintf( sourcefile, sizeof( sourcefile ), WIDE("") FILELINE_FILELINEFMT  FILELINE_RELAY );
	}
	else
      sourcefile[0] = 0;
	if( flags.bLogThreadID )
		snprintf( threadid, sizeof( threadid ), WIDE("%") _64fX WIDE("~"), GetMyThreadID() );
	if( pFile )
		snprintf( buffer, sizeof( buffer )
				  , WIDE("%s%s%s%s%s%s%s")
				  , logtime, logtime[0]?WIDE("|"):WIDE("")
				  , flags.bLogThreadID?threadid:WIDE("")
				  , flags.bLogProgram?(pProgramName?pProgramName:WIDE("")):WIDE("")
				  , flags.bLogProgram?WIDE("@"):WIDE("")
				  , flags.bLogSourceFile?sourcefile:WIDE("")
				  , message );
	else
		snprintf( buffer, sizeof( buffer )
				  , WIDE("%s%s%s%s%s%s")
				  , logtime, logtime[0]?WIDE("|"):WIDE("")
				  , flags.bLogThreadID?threadid:WIDE("")
				  , flags.bLogProgram?(pProgramName?pProgramName:WIDE("")):WIDE("")
				  , flags.bLogProgram?WIDE("@"):WIDE("")
				  , message );
	DoSystemLog( buffer );
   lock = 0;
}

#undef SystemLogEx
void SystemLogEx ( const TEXTCHAR *message DBG_PASS )
{
#ifdef _DEBUG
	SystemLogFL( message DBG_RELAY );
#else
	SystemLogFL( message FILELINE_NULL );
#endif
}

#undef SystemLog
SYSLOG_PROC( void, SystemLog )( const TEXTCHAR *message )
{
	SystemLogFL( message FILELINE_NULL );
}

SYSLOG_PROC( void, LogBinaryFL )( P_8 buffer, _32 size FILELINE_PASS )
{
	int nOut = size;
	P_8 data = buffer;
#ifndef _LOG_FULL_FILE_NAMES
	CTEXTSTR p;
	for( p = pFile + (pFile?strlen(pFile) -1:0);p > pFile;p-- )
		if( p[0] == '/' || p[0] == '\\' )
		{
			pFile = p+1;break;
		}
#endif
	while( nOut > 0 )
	{
		TEXTCHAR cOut[96];
		int ofs = 0;
		int x;
		ofs = 0;
		for ( x=0; x<nOut && x<16; x++ )
			ofs += snprintf( cOut+ofs, sizeof(cOut)-ofs, WIDE("%02X "), (unsigned char)data[x] );

		for ( x=0; x<nOut && x<16; x++ )
		{
			if( data[x] >= 32 && data[x] < 127 )
				ofs += snprintf( cOut+ofs, sizeof(cOut)-ofs, WIDE("%c"), (unsigned char)data[x] );
			else
				ofs += snprintf( cOut+ofs, sizeof(cOut)-ofs, WIDE(".") );
		}
		SystemLogFL( cOut FILELINE_RELAY );
		nOut -= 16;
		data += 16;
	}
}
#undef LogBinaryEx
SYSLOG_PROC( void, LogBinaryEx )( P_8 buffer, _32 size DBG_PASS )
{
#ifdef _DEBUG
	LogBinaryFL( buffer,size DBG_RELAY );
#else
	LogBinaryFL( buffer,size FILELINE_NULL );
#endif
}
#undef LogBinary
SYSLOG_PROC( void, LogBinary )( P_8 buffer, _32 size )
{
	LogBinaryFL( buffer,size FILELINE_NULL );
}

SYSLOG_PROC( void, SetSystemLog )( enum syslog_types type, const void *data )
{
	if( file )
	{
		fclose( file );
		file = NULL;
	}	
	if( type == SYSLOG_FILE )
	{
		if( data )
		{
			logtype = type;
			file = (FILE*)data;
		}
	}
	else if( type == SYSLOG_FILENAME )
	{
		FILE *log;
		Fopen( log, (CTEXTSTR)data, WIDE("wt") );
		file = log;
		logtype = SYSLOG_FILE;
	}
	else if( type == SYSLOG_CALLBACK )
	{
		UserCallback = (UserLoggingCallback)data;
	}
	else
	{
		logtype = type;
	}
	//SystemLog( WIDE("thing is: ") STRSYM( (SYSLOG_EXTERN) ) );
}

SYSLOG_PROC( void, SystemLogTime )( LOGICAL enable )
{
	flags.bLogTime = FALSE;
	flags.bUseDay = FALSE;
	flags.bUseDeltaTime = FALSE;
	flags.bLogHighTime = FALSE;
	flags.bLogCPUTime = FALSE;
	if( enable )
	{
		flags.bLogTime = TRUE;
		if( enable & SYSLOG_TIME_HIGH )
			flags.bLogHighTime = TRUE;
		if( enable & SYSLOG_TIME_LOG_DAY )
			flags.bUseDay = TRUE;
		if( enable & SYSLOG_TIME_DELTA )
			flags.bUseDeltaTime = TRUE;
		if( enable & SYSLOG_TIME_CPU )
			flags.bLogCPUTime = TRUE;
	}
}

// information for the call to _real_lprintf file and line information...
static struct next_lprint_info{
	// please use this enter when resulting a function, and leave from said function.
	// oh - but then we couldn't exist before crit sec code...
	//CRITICALSECTION cs;
	_32 nLevel;
	CTEXTSTR pFile;
	int nLine;
} next_lprintf;

// may have to switch based on the version of gnuc here...
//#if defined( HAVE_GOOD_VA_ARGS ) || defined( __GNUC__ )
//---------------------------------------------------------------------------
SYSLOG_PROC( void, _vlprintf )( int argsize, CTEXTSTR format, va_list args )
{
	if( next_lprintf.nLevel & LOG_CUSTOM )
	{
	// apply log custom bits to only compare bits which
	// the user may specify, then compare that value with the
	// bits which have been specified as enabled... if this fails,
	// this message is not enabled.
		if( !(( next_lprintf.nLevel & LOG_CUSTOM_BITS ) & nLogCustom ) )
			return;
      // perform some clever mask based on something else...
	}
	else if( next_lprintf.nLevel > nLogLevel )
		return;
	if( logtype != SYSLOG_NONE )
	{
		CTEXTSTR logtime = GetLogTime();
		//va_list _args = args;
		int ofs;
		TEXTCHAR buffer[1023];
		TEXTCHAR threadid[32];

		if( logtime[0] )
			ofs = snprintf( buffer, 1023, WIDE("%s|")
							  , logtime );
		else
			ofs = 0;
		// argsize - the program's giving us file and line
		// debug for here or not, this must be used.
		if( argsize )
		{
			CTEXTSTR pFile;
#ifndef _LOG_FULL_FILE_NAMES
			CTEXTSTR p, prog;
#endif
			_32 nLine;
			if( ( pFile = next_lprintf.pFile ) )
			{
#ifndef _LOG_FULL_FILE_NAMES
				for( p = pFile + strlen(pFile) -1;p > pFile;p-- )
					if( p[0] == '/' || p[0] == '\\' )
					{
						pFile = p+1;break;
					}
				for( prog = pProgramName + strlen(pProgramName) -1;prog > pProgramName;prog-- )
					if( prog[0] == '/' || prog[0] == '\\' )
					{
						pFile = p+1;break;
					}
#endif
				nLine = next_lprintf.nLine;
				if( flags.bLogThreadID )
					snprintf( threadid, sizeof( threadid ), WIDE("%") _64fX WIDE("~"), GetMyThreadID() );
				ofs += snprintf( buffer + ofs, 1023 - ofs, WIDE("%s%s%s%s(%") _32f WIDE("):")
									, flags.bLogThreadID?threadid:WIDE("")
									, flags.bLogProgram?pProgramName:WIDE("")
									, flags.bLogProgram?WIDE("@"):WIDE("")
									, pFile, nLine );
			}
			ofs += vsnprintf( buffer + ofs, 1023 - ofs, format, args );
		}
		DoSystemLog( buffer );
	}
}
//#endif

static INDEX CPROC _null_vlprintf ( CTEXTSTR format, va_list args )
{
   return 0;
}

static PLINKQUEUE buffers;

static INDEX CPROC _real_vlprintf ( CTEXTSTR format, va_list args )
{
	if( logtype != SYSLOG_NONE )
	{
		CTEXTSTR logtime = GetLogTime();
		int ofs;
		// because of threading concerns... either I dynamically allocate this...
		// or lock it.... or ...
		TEXTCHAR *buffer;
		TEXTCHAR threadid[32];

		if( !buffers )
		{
         int n;
			buffers = CreateLinkQueue() ;
			for( n = 0; n < 64; n++ )
				EnqueLink( &buffers, (POINTER)1 );
			for( n = 0; n < 64; n++ )
            DequeLink( &buffers );
		}

		buffer = (TEXTCHAR*)DequeLink( &buffers );
		if( !buffer )
		{
			buffer = (TEXTCHAR*)malloc( 4096 );//NewArray( TEXTCHAR, 4096 );
		}

		if( logtime[0] )
			ofs = snprintf( buffer, 4095, WIDE("%s|")
							  , logtime );
		else
			ofs = 0;
		// argsize - the program's giving us file and line
		// debug for here or not, this must be used.
		{
			CTEXTSTR pFile;
#ifndef _LOG_FULL_FILE_NAMES
			CTEXTSTR p;
#endif
			_32 nLine;
			if( ( pFile = next_lprintf.pFile ) )
			{
				if( flags.bProtectLoggedFilenames )
					if( IsBadReadPtr( pFile, 2 ) )
                  pFile = WIDE("(Unloaded file?)");
#ifndef _LOG_FULL_FILE_NAMES
				for( p = pFile + strlen(pFile) -1;p > pFile;p-- )
					if( p[0] == '/' || p[0] == '\\' )
					{
						pFile = p+1;break;
					}
#endif
				nLine = next_lprintf.nLine;
				if( flags.bLogThreadID )
					snprintf( threadid, sizeof( threadid ), WIDE("%") _64fX WIDE("~"), GetMyThreadID() );
				ofs += snprintf( buffer + ofs, 4095 - ofs, WIDE("%s%s%s%s(%") _32f WIDE("):")
									, flags.bLogThreadID?threadid:WIDE("")
									, flags.bLogProgram?pProgramName:WIDE("")
									, flags.bLogProgram?WIDE("@"):WIDE("")
									, pFile, nLine );
			}
			ofs += vsnprintf( buffer + ofs, 4095 - ofs, format, args );
		}
		DoSystemLog( buffer );
		EnqueLink( &buffers, buffer );
	}
	//LeaveCriticalSec( &next_lprintf.cs );
   return 0;
}

static INDEX CPROC _real_lprintf( CTEXTSTR f, ... )
{
	va_list args;
	va_start( args, f );
	return _real_vlprintf( f, args );
}

static INDEX CPROC _null_lprintf( CTEXTSTR f, ... )
{
   return 0;
}


SYSLOG_PROC( RealVLogFunction, _vxlprintf )( _32 level DBG_PASS )
{
   //EnterCriticalSec( &next_lprintf.cs );
#if _DEBUG
	next_lprintf.pFile = pFile;
	next_lprintf.nLine = nLine;
#endif
	if( level & LOG_CUSTOM )
	{
		if( !(level & nLogCustom) )
			return _null_vlprintf;
		return _real_vlprintf;
	}
	else if( level <= nLogLevel )
	{
		return _real_vlprintf;
	}
   return _null_vlprintf;
}

RealLogFunction _xlprintf( _32 level DBG_PASS )
{
	//EnterCriticalSec( &next_lprintf.cs );
#if _DEBUG
	next_lprintf.pFile = pFile;
	next_lprintf.nLine = nLine;
#endif
	if( level & LOG_CUSTOM )
	{
		if( !(level & nLogCustom) )
			return _null_lprintf;
		next_lprintf.nLevel = level;
		return _real_lprintf;
	}
	else if( level <= nLogLevel )
	{
		next_lprintf.nLevel = level;
		return _real_lprintf;
	}
   return _null_lprintf;
}

/*
SYSLOG_PROC( void, _lprintf )( int argsize, CTEXTSTR format , ... )
{
	va_list args;
	va_start( args, format );
	_vlprintf( argsize, format, args );
}
*/

#if 0
// this is for classic code compatibility.
// one day someone should say screw it, delete this, and make everyone compile distclean.
//#undef lprintf
SYSLOG_PROC( void, lprintf )( int argsize, CTEXTSTR format , ... )
{
	va_list args;
	va_start( args, format );
	_vlprintf( argsize, format, args );
}
#endif

#ifdef __WATCOMC__
// # again - WATCOM Compiler warning here, function
// defined, but never referenced.  This is true,
// but when the linker comes around to it, it cares for the
// presense of this function in order to force floating support
// for printf and scanf, since this is module is passed ANY
// format and ANY paramter, it may require floating point.
// no special handling required for GCC, lcc, MSVC
static int f(void )
{
	extern int fltused_;
   return fltused_ + (int)f;
	//int n = fltused_; // force inclusion of math libs...
}
#endif

void ProtectLoggedFilenames( LOGICAL bEnable )
{
   flags.bProtectLoggedFilenames = bEnable;
}

void SetSystemLoggingLevel( _32 nLevel )
{
	if( nLevel & LOG_CUSTOM )
	{
      nLogCustom |= nLevel & ( LOG_CUSTOM_BITS );
	}
	else if( nLevel & LOG_CUSTOM_DISABLE )
	{
     nLogCustom &= ~( nLevel & ( LOG_CUSTOM_BITS ) );
	}
	else
		nLogLevel = nLevel;
}

CTEXTSTR GetProgramName( void )
{
   return pProgramName;
}

void SetSyslogOptions( FLAGSETTYPE *options )
{
   // the mat operations don't turn into valid bitfield operators. (watcom)
	flags.bLogOpenAppend = TESTFLAG( options, SYSLOG_OPT_OPENAPPEND )?1:0; // open for append, else open for write
   flags.bLogOpenBackup = TESTFLAG( options, SYSLOG_OPT_OPEN_BACKUP )?1:0; // open for append, else open for write
	flags.bLogProgram = TESTFLAG( options, SYSLOG_OPT_LOG_PROGRAM_NAME )?1:0; // open for append, else open for write
	flags.bLogThreadID = TESTFLAG( options, SYSLOG_OPT_LOG_THREAD_ID )?1:0; // open for append, else open for write
	flags.bLogSourceFile = TESTFLAG( options, SYSLOG_OPT_LOG_SOURCE_FILE )?1:0; // open for append, else open for write
}

#ifdef __cplusplus_cli

static public ref class Log
{
public:
	static void log( String^ ouptut )
	{
				pin_ptr<const WCHAR> _output = PtrToStringChars(ouptut);
				TEXTSTR __ouptut = sack::containers::text::WcharConvert( _output );
		lprintf( "%s", __ouptut );
		Release( __ouptut );
	}
};
#endif

#ifdef __cplusplus
LOGGING_NAMESPACE_END
#endif

//---------------------------------------------------------------------------
// $Log: syslog.c,v $
// Revision 1.74  2005/05/30 11:56:36  d3x0r
// various fixes... working on psilib update optimization... various stabilitizations... also extending msgsvr functionality.
//
// Revision 1.73  2005/05/25 16:50:29  d3x0r
// Synch with working repository.
//
// Revision 1.80  2005/05/13 23:51:51  jim
// Massive changes to enhance ATEXIT, implement a PRIORITY_ATEXIT, and be happy.
//
// Revision 1.79  2005/05/03 20:03:13  jim
// Reformat, also truncate the passed filename to LogBinary
//
// Revision 1.78  2005/04/19 22:49:30  jim
// Looks like the display module technology nearly works... at least exits graceful are handled somewhat gracefully.
//
// Revision 1.77  2005/03/23 09:12:15  panther
// attempt to cleanup atexit - problem SYSLOG_FILE should be handled by the application, not this.
//
// Revision 1.76  2005/01/27 07:02:18  panther
// Windows clean as can be.
//
// Revision 1.75  2005/01/27 07:02:01  panther
// Linux clean.
//
// Revision 1.74  2005/01/16 23:44:49  panther
// Minor spelling
//
// Revision 1.73  2004/11/29 07:14:59  panther
// Change default logging to progname.log
//
// Revision 1.72  2004/10/31 17:22:28  d3x0r
// Minor fixes to control library...
//
// Revision 1.71  2004/09/23 00:41:05  d3x0r
// set default flags to force CPU resolution delta logging.
//
// Revision 1.70  2004/09/03 14:43:48  d3x0r
// flexible frame reactions to font changes...
//
// Revision 1.69  2004/09/01 03:27:21  d3x0r
// Control updates video display issues?  Image blot message go away...
//
// Revision 1.68  2004/08/26 04:38:33  d3x0r
// Fix debug/release stupidity
//
// Revision 1.67  2004/08/25 15:01:06  d3x0r
// Checkpoint - more vc compat fixes
//
// Revision 1.66  2004/08/25 08:44:52  d3x0r
// Portability changes for MSVC... Updated projects, all projects build up to PSI, no display...
//
// Revision 1.65  2004/08/22 11:57:50  d3x0r
// Checkpoint for msvc update... procreg and psilib are going to be beasts next.
//
// Revision 1.64  2004/08/22 09:36:52  d3x0r
// Okay this will support old logging and new logging... but more importantly with clever cutting can be done WITHOUT __VA_ARGS__!
//
// Revision 1.63  2004/08/16 07:12:47  d3x0r
// Schedule syslog init FIRST.
//
// Revision 1.62  2004/07/13 04:01:40  d3x0r
// Clean warnings from new preload code and linux strictness.
//
// Revision 1.61  2004/05/25 17:22:37  d3x0r
// Added preload conditions to initialize logging
//
// Revision 1.60  2004/05/04 07:34:38  d3x0r
// Fix defninition of SystemLog
//
// Revision 1.59  2004/05/02 04:36:02  d3x0r
// Added user callback support, fixed lprintf definition.  Fixed formatting. Trimmed old logging.
//
// Revision 1.58  2004/01/05 12:20:05  panther
// Add protection for when file does not exist
//
// Revision 1.57  2003/12/05 11:06:48  panther
// Fix log binary to not show garbage
//
// Revision 1.56  2003/12/04 10:41:30  panther
// remove watcom compile only option code
//
//
