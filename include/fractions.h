
#ifndef FRACTIONS_DEFINED
#define FRACTIONS_DEFINED

#ifdef BCC16
#ifdef FRACTION_SOURCE
#define FRACTION_PROC(type,name) type STDPROC _export name
#else
#define FRACTION_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef FRACTION_SOURCE
#define FRACTION_PROC(type,name) EXPORT_METHOD type name
#else
#define FRACTION_PROC(type,name) IMPORT_METHOD type name
#endif
#else
#ifdef FRACTION_SOURCE
#define FRACTION_PROC(type,name) type name
#else
#define FRACTION_PROC(type,name) extern type name
#endif
#endif
#endif


#ifdef __cplusplus
namespace sack { namespace math { namespace fraction {
#endif

typedef struct fraction_tag {
	int numerator;
	int denominator;
} FRACTION, *PFRACTION;
#ifdef HAVE_ANONYMOUS_STRUCTURES
typedef struct coordpair_tag {
	union {
		FRACTION x;
		FRACTION width;
	};
	union {
		FRACTION y;
		FRACTION height;
	};
} COORDPAIR, *PCOORDPAIR;
#else
typedef struct coordpair_tag {
       	FRACTION x;
       	FRACTION y;
} COORDPAIR, *PCOORDPAIR;

#endif

#define SetFraction(f,n,d) ((((f).numerator=((int)(n)) ),((f).denominator=((int)(d)))),(f))
#define SetFractionV(f,w,n,d) (  (d)? \
	((((f).numerator=((int)((n)*(w))) )  \
	,((f).denominator=((int)(d)))),(f))  \
	: 	((((f).numerator=((int)((w))) )  \
	,((f).denominator=((int)(1)))),(f))  \
)
FRACTION_PROC( void, AddCoords )( PCOORDPAIR base, PCOORDPAIR offset );

FRACTION_PROC( PFRACTION, AddFractions )( PFRACTION base, PFRACTION offset );
FRACTION_PROC( PFRACTION, MulFractions )( PFRACTION f, PFRACTION x );

FRACTION_PROC( int, sLogFraction )( TEXTCHAR *string, PFRACTION x );
FRACTION_PROC( int, sLogCoords )( TEXTCHAR *string, PCOORDPAIR pcp );
FRACTION_PROC( void, LogCoords )( PCOORDPAIR pcp );

FRACTION_PROC( PFRACTION, ScaleFraction )( PFRACTION result, S_32 value, PFRACTION f );
FRACTION_PROC( S_32, ReduceFraction )( PFRACTION f );

FRACTION_PROC( _32, ScaleValue )( PFRACTION f, S_32 value );
FRACTION_PROC( _32, InverseScaleValue )( PFRACTION f, S_32 value );

#ifdef __cplusplus
}}} //namespace sack { namespace math { namespace fraction {
using namespace sack::math::fraction;
#endif

#endif

//---------------------------------------------------------------------------
// $Log: fractions.h,v $
// Revision 1.6  2004/09/03 14:43:40  d3x0r
// flexible frame reactions to font changes...
//
// Revision 1.5  2003/03/25 08:38:11  panther
// Add logging
//
// Revision 1.4  2003/01/27 09:45:03  panther
// Fix lack of anonymous structures
//
// Revision 1.3  2002/10/09 13:16:02  panther
// Support for linux shared memory mapping.
// Support for better linux compilation of configuration scripts...
// Timers library is now Threads AND Timers.
//
//
