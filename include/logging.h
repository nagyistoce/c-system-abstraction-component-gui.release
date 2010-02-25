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
 */


#ifndef LOGGING_MACROS_DEFINED
#define LOGGING_MACROS_DEFINED
#include <stdarg.h>
#include <sack_types.h>

#ifdef BCC16
#ifdef SYSLOG_SOURCE
#define SYSLOG_PROC(type,name) type STDPROC _export name
#else
#define SYSLOG_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef SYSLOG_SOURCE
#define SYSLOG_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define SYSLOG_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#else
#ifdef SYSLOG_SOURCE
#define SYSLOG_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define SYSLOG_PROC(type,name) extern type CPROC name
#endif
#endif
#endif

enum syslog_types {
SYSLOG_NONE     =   -1 // disable any log output.
,SYSLOG_UDP      =    0
,SYSLOG_FILE     =    1
,SYSLOG_FILENAME =    2
,SYSLOG_SYSTEM   =    3
,SYSLOG_UDPBROADCAST= 4
// Allow user to specify a void UserCallback( char * )
// which recieves the formatted output.
,SYSLOG_CALLBACK    = 5
,SYSLOG_AUTO_FILE = SYSLOG_FILE + 100
};

#if !defined( NO_LOGGING )
#define DO_LOGGING
#endif
// this was forced, force no_logging off...
#if defined( DO_LOGGING )
#undef NO_LOGGING
#endif


#ifdef __cplusplus
#define LOGGING_NAMESPACE namespace sack { namespace logging {
#define LOGGING_NAMESPACE_END }; };
#else
#define LOGGING_NAMESPACE 
#define LOGGING_NAMESPACE_END
#endif
LOGGING_NAMESPACE

#ifdef __LINUX__
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

SYSLOG_PROC( LOGICAL, IsBadReadPtr )( CPOINTER pointer, PTRSZVAL len );
#endif

SYSLOG_PROC( CTEXTSTR, GetPackedTime )( void );


// 
typedef void (CPROC*UserLoggingCallback)( CTEXTSTR log_string );
SYSLOG_PROC( void, SetSystemLog )( enum syslog_types type, const void *data );
SYSLOG_PROC( void, ProtectLoggedFilenames )( LOGICAL bEnable );

SYSLOG_PROC( void, SystemLogFL )( CTEXTSTR FILELINE_PASS );
SYSLOG_PROC( void, SystemLogEx )( CTEXTSTR DBG_PASS );
SYSLOG_PROC( void, SystemLog )( CTEXTSTR );
SYSLOG_PROC( void, LogBinaryFL )( P_8 buffer, _32 size FILELINE_PASS );
SYSLOG_PROC( void, LogBinaryEx )( P_8 buffer, _32 size DBG_PASS );
SYSLOG_PROC( void, LogBinary )( P_8 buffer, _32 size );
// logging level defaults to 1000 which is log everything
SYSLOG_PROC( void, SetSystemLoggingLevel )( _32 nLevel );
#ifdef _DEBUG
#define LogBinary(buf,sz) LogBinaryFL((P_8)buf,sz DBG_SRC )
#define SystemLog(buf)    SystemLogFL(buf DBG_SRC )
#else
// need to include the typecast... binary logging doesn't really care what sort of pointer it gets.
#define LogBinary(buf,sz) LogBinary((P_8)buf,sz )
//#define LogBinaryEx(buf,sz,...) LogBinaryFL(buf,sz FILELINE_NULL)
//#define SystemLogEx(buf,...) SystemLogFL(buf FILELINE_NULL )
#endif

// int result is useless... but allows this to be
// within expressions, which with this method should be easy.
typedef INDEX (CPROC*RealVLogFunction)(CTEXTSTR format, va_list args );
typedef INDEX (CPROC*RealLogFunction)(CTEXTSTR format,...)
#ifdef GCC
	__attribute__ ((__format__ (__printf__, 1, 2)))
#endif
	;
SYSLOG_PROC( RealVLogFunction, _vxlprintf )( _32 level DBG_PASS );
SYSLOG_PROC( RealLogFunction, _xlprintf )( _32 level DBG_PASS );
SYSLOG_PROC( CTEXTSTR, GetProgramName )( void );

// utility function to format a cpu delta into a buffer...
// end-start is always printed... therefore tick_end-0 is
// print absolute time... formats as millisecond.NNN
SYSLOG_PROC( void, PrintCPUDelta )( char *buffer, int buflen, _64 tick_start, _64 tick_end );
// return the current CPU tick
SYSLOG_PROC( _64, GetCPUTick )( void );
// result in nano seconds - thousanths of a millisecond...
SYSLOG_PROC( _32, ConvertTickToMicrosecond )( _64 tick );
SYSLOG_PROC( _64, GetCPUFrequency )( void );
SYSLOG_PROC( CTEXTSTR, GetTimeEx )( int bUseDay );

SYSLOG_PROC( void, SetSyslogOptions )( FLAGSETTYPE *options );
enum system_logging_option_list {
	SYSLOG_OPT_OPENAPPEND
										  , SYSLOG_OPT_OPEN_BACKUP
                                , SYSLOG_OPT_LOG_PROGRAM_NAME
                                , SYSLOG_OPT_LOG_THREAD_ID
										  , SYSLOG_OPT_LOG_SOURCE_FILE
                                , SYSLOG_OPT_MAX
};

// this solution was developed to provide the same
// functionality for compilers that refuse to implement __VA_ARGS__
// this therefore means that the leader of the function is replace
// and that extra parenthesis exist after this... therefore the remaining
// expression must be ignored... thereofre when defining a NULL function
// this will result in other warnings, about ignored, or meaningless expressions
# if defined( DO_LOGGING )
#  define vlprintf      _vxlprintf(LOG_NOISE DBG_SRC)
#  define lprintf       _xlprintf(LOG_NOISE DBG_SRC)
#  define _lprintf(DBG_VOIDRELAY)       _xlprintf(LOG_NOISE DBG_RELAY)
#  define xlprintf(level)       _xlprintf(level DBG_SRC)
#  define vxlprintf(level)       _vxlprintf(level DBG_SRC)
# else
#  ifdef _MSC_VER
#   define vlprintf      (1)?(0):
#   define lprintf       (1)?(0):
#   define _lprintf(DBG_VOIDRELAY)       (1)?(0):
#   define xlprintf(level)       (1)?(0):
#   define vxlprintf(level)      (1)?(0):
#  else
#   define vlprintf(f,...)
#   define lprintf(f,...)
#   define  _lprintf(DBG_VOIDRELAY)       lprintf
#   define xlprintf(level) lprintf
#   define vxlprintf(level) lprintf
#  endif
# endif
#undef LOG_WARNING
#undef LOG_ADVISORIES
#undef LOG_INFO
// Defined Logging Levels
enum {
	  // and you are free to use any numerical value,
	  // this is a rough guideline for wide range
	  // to provide a good scaling for levels of logging
	LOG_ALWAYS = 1 // unless logging is disabled, this will be logged
	, LOG_ERRORS = 50 // logging level set to 50 or more will cause this to log
	, LOG_ERROR = LOG_ERRORS // logging level set to 50 or more will cause this to log
	, LOG_WARNINGS = 500 // .......
	, LOG_WARNING = LOG_WARNINGS // .......
   , LOG_ADVISORY = 625
   , LOG_ADVISORIES = LOG_ADVISORY
	, LOG_INFO = 750
	  , LOG_NOISE = 1000
     , LOG_DEBUG = 2000
	, LOG_CUSTOM = 0x40000000 // not quite a negative number, but really big
	, LOG_CUSTOM_DISABLE = 0x20000000 // not quite a negative number, but really big
	// bits may be user specified or'ed with this value
	// such that ...
	// Example 1:SetSystemLoggingLevel( LOG_CUSTOM | 1 ) will
	// enable custom logging messages which have the '1' bit on... a logical
	// and is used to test the low bits of this value.
	// example 2:SetSystemLogging( LOG_CUSTOM_DISABLE | 1 ) will disable logging
	// of messages with the 1 bit set.
#define LOG_CUSTOM_BITS 0xFFFFFF  // mask of bits which may be used to enable and disable custom logging
};
//#  error "Unsupported compiler - No or broken __VA_ARGS__ (notes within)"
// even if one figured out how to do lprintf some other way
// __VA_ARGS__ are used in PSIlib.... and are not possible
// to construct the macros without.  (well longhand expansion I suppose)
#if 0 // just delete this line... endif remains.
#endif

#define SYSLOG_TIME_DISABLE 0
#define SYSLOG_TIME_ENABLE  1 // enable is anything not zero.
#define SYSLOG_TIME_HIGH    2
#define SYSLOG_TIME_LOG_DAY 4
#define SYSLOG_TIME_DELTA   8
#define SYSLOG_TIME_CPU    16 // logs cpu ticks... implied delta
// this is a flag set consisting of 0 or'ed with the preceeding symbols
SYSLOG_PROC( void, SystemLogTime )( _32 enable );

#if !defined( DO_LOGGING ) || defined( NO_LOGGING )
// any kinda semi decent compiler will optimize this gone...
#define OutputLogString(s) //do{}while(0)
#define Log(s) //do{}while(0)
#define Log1(s,p1) //do{}while(0)
#define Log2(s,p1,p2) //do{}while(0)
#define Log3(s,p1,p2,p3) //do{}while(0)
#define Log4(s,p1,p2,p3,p4) //do{}while(0)
#define Log5(s,p1,p2,p3,p4,p5) //do{}while(0)
#define Log6(s,p1,p2,p3,p4,p5,p6) //do{}while(0)
#define Log7(s,p1,p2,p3,p4,p5,p6,p7) //do{}while(0)
#define Log8(s,p1,p2,p3,p4,p5,p6,p7,p8) //do{}while(0)
#define Log9(s,p1,p2,p3,p4,p5,p6,p7,p8,p9) //do{}while(0)
#define Log10(s,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10) //do{}while(0)
#else
#ifdef __cplusplus
//namespace std {
#endif
#ifdef __cplusplus
//};
#endif

#define OutputLogString(s) SystemLog(s)
#define Log(s)                                   lprintf( WIDE("%s"), s )
#define Log1(s,p1)                               lprintf( s, p1 )
#define Log2(s,p1,p2)                            lprintf( s, p1, p2 )
#define Log3(s,p1,p2,p3)                         lprintf( s, p1, p2, p3 )
#define Log4(s,p1,p2,p3,p4)                      lprintf( s, p1, p2, p3,p4)
#define Log5(s,p1,p2,p3,p4,p5)                   lprintf( s, p1, p2, p3,p4,p5)
#define Log6(s,p1,p2,p3,p4,p5,p6)                lprintf( s, p1, p2, p3,p4,p5,p6)
#define Log7(s,p1,p2,p3,p4,p5,p6,p7)             lprintf( s, p1, p2, p3,p4,p5,p6,p7 )
#define Log8(s,p1,p2,p3,p4,p5,p6,p7,p8)          lprintf( s, p1, p2, p3,p4,p5,p6,p7,p8 )
#define Log9(s,p1,p2,p3,p4,p5,p6,p7,p8,p9)       lprintf( s, p1, p2, p3,p4,p5,p6,p7,p8,p9 )
#define Log10(s,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)  lprintf( s, p1, p2, p3,p4,p5,p6,p7,p8,p9,p10 )

#endif

LOGGING_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::logging;
#endif

#endif
