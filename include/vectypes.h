#ifndef VECTOR_TYPES_DEFINED
#define VECTOR_TYPES_DEFINED
// this file merely defines the basic calculation unit...
// more types are defined in VECTLIB.H which has the number
// of dimensions defined... 

#include <sack_types.h>
#include <math.h>
#include <float.h>


VECTOR_NAMESPACE

#define RCOORD_IS_DOUBLE
typedef double RCOORD, *PRCOORD;  // basic type used here....

// these SHOULD be dimension relative, but we lack much code for that...
typedef RCOORD MATRIX[4][4];
typedef RCOORD PMATRIX[][4];

#ifdef RCOORD_IS_DOUBLE
#define RCOORDBITS(v)  (*(_64*)&(v))
#else
#define RCOORDBITS(v)  (*(_32*)&(v))
#endif
#define NEG_INFINITY ((RCOORD)-9999999999.0)
#define POS_INFINITY ((RCOORD)9999999999.0)

#define e1 (0.00001)
#define NearZero( n ) (fabs(n)<e1)
#ifndef __cplusplus
#endif


// THRESHOLD may be any number more than 1.
// eveything 0 or more makes pretty much every number
// which is anything like another number equal...
// threshold is in count of bits...
// therefore for 1millionth digit accuracy that would be 20 bits.
// 10 bits is thousanths different are near
// 0 bits is if the same magnitude of number... but may be
//   quite different
// -10 bits is thousands different are near
// 1 == 1.5
// 1 == 1.01

#ifdef RCOORD_IS_DOUBLE
#define THRESHOLD 16
#ifdef _MSC_VER
#define EXPON(f) ((int)(( RCOORDBITS(f) & 0x4000000000000000I64 ) ?   \
                    (( RCOORDBITS(f) &  0x3FF0000000000000I64 ) >> (20+32)) :    \
                    ((( RCOORDBITS(f) & 0x3FF0000000000000I64 ) >> (20+32)) - 1024)))
#else
#define EXPON(f) ((int)(( RCOORDBITS(f) & 0x4000000000000000LL ) ?   \
                    (( RCOORDBITS(f) &  0x3FF0000000000000LL ) >> (20+32)) :    \
                    ((( RCOORDBITS(f) & 0x3FF0000000000000LL ) >> (20+32)) - 1024)))
#endif
#else
#define THRESHOLD 1
#define EXPON(f) ((int)( RCOORDBITS(f) & 0x40000000L ) ?   \
                    (( RCOORDBITS(f) & 0x3F800000L ) >> 23) :    \
                    ((( RCOORDBITS(f) & 0x3F800000L ) >> 23) - 128))
#endif
//cpg26Dec2006 c:\work\sack\include\vectypes.h(75): Warning! W202: Symbol 'COMPARE' has been defined, but not referenced
static int COMPARE( RCOORD n1, RCOORD n2 )
{
	RCOORD tmp1, tmp2;
	int compare_result;
	tmp1=n1-n2;
   /*
	lprintf( WIDE("exponents %ld %ld"), EXPON( n1 ), EXPON( n2 ) );
 	lprintf("%9.9g-%9.9g=%9.9g %s %s %ld %ld %ld"
			 , (n1),(n2),(tmp1)
			 ,!RCOORDBITS(n1)?"zero":"    ",!RCOORDBITS(n2)?"zero":"    "
  		 ,EXPON(n1)-THRESHOLD
			 ,EXPON(n2)-THRESHOLD
			 ,EXPON(tmp1) );
          */
	tmp2=n2-n1;
   /*
	lprintf("%9.9g-%9.9g=%9.9g %s %s %ld %ld %ld"
			 , (n2),(n1),(tmp2)
			 ,!RCOORDBITS(n2)?"zero":"    ",!RCOORDBITS(n1)?"zero":"    "
			 ,EXPON(n2)-THRESHOLD,EXPON(n1)-THRESHOLD,EXPON(tmp2));
			 */
	compare_result = ( ( !RCOORDBITS(n1) )?( (n2) <  0.0000001 &&
														 (n2) > -0.0000001 )?1:0
							:( !RCOORDBITS(n2) )?( (n1) <  0.0000001 &&
														 (n1) > -0.0000001 )?1:0
							:( (n1) == (n2) )?1
							:( ( EXPON(n1) - THRESHOLD ) >=
							  ( EXPON( tmp1 ) ) ) &&
							( ( EXPON(n2) - THRESHOLD ) >=
							 ( EXPON( tmp2) ) ) ? 1 : 0
						  );
   /*
	 lprintf( WIDE("result=%d"), compare_result );
    */
	return compare_result;
}
/*
static RCOORD CompareTemp1, CompareTemp2;
#define COMPARE( n1, n2 ) ( RCOORDBITS(n1)                \
                            ? ( CompareTemp1 = (n1)+(n1), \
                                CompareTemp2 = (n1)+(n2), \
                  (RCOORDBITS(CompareTemp1)&0xFFFFFFF0)==                \
                     (RCOORDBITS(CompareTemp2)&0xFFFFFFF0) )             \
                            : RCOORDBITS(n2)                \
                              ? ( CompareTemp1 = (n2)+(n1), \
                                  CompareTemp2 = (n2)+(n2), \
                                 (RCOORDBITS(CompareTemp1)&0xFFFFFFF0)==  \
                                 (RCOORDBITS(CompareTemp2)&0xFFFFFFF0) )  \
                                                : 1 ) 
*/
/*
                           ( ( ( RCOORDBITS(n1) & 0x80000000 ) !=        \
                              ( RCOORDBITS(n2) & 0x80000000 ) )         \
                              ? ( NearZero(n1)                          \
                                 && ( ( RCOORDBITS(n1)&0x7FFFFFF0 ) ==  \
                                      ( RCOORDBITS(n2)&0x7FFFFFF0 ) ) ) \
                              : ( ( RCOORDBITS(n1)&0xFFFFFFF0 ) ==      \
                                  ( RCOORDBITS(n2)&0xFFFFFFF0 ) ) )
*/
#if 1
#else
inline int COMPARE( RCOORD n1, RCOORD n2 )
{
    RCOORD CompareTemp1, CompareTemp2;
   return  RCOORDBITS(n1)                \
                            ? ( CompareTemp1 = (n1)+(n1), \
                                CompareTemp2 = (n1)+(n2), \
                  (RCOORDBITS(CompareTemp1)&0xFFFFFFF0)==                \
                     (RCOORDBITS(CompareTemp2)&0xFFFFFFF0) )             \
                            : RCOORDBITS(n2)                \
                              ? ( CompareTemp1 = (n2)+(n1), \
                                  CompareTemp2 = (n2)+(n2), \
                                 (RCOORDBITS(CompareTemp1)&0xFFFFFFF0)==  \
                                 (RCOORDBITS(CompareTemp2)&0xFFFFFFF0) )  \
                                                : 1;
}
#endif
VECTOR_NAMESPACE_END

#endif
// $Log: vectypes.h,v $
// Revision 1.12  2005/01/27 08:21:39  panther
// Linux cleaned.
//
// Revision 1.11  2004/02/08 05:42:29  d3x0r
// associate comparetemp1, 2 with routine which needs it.
//
// Revision 1.10  2003/11/28 00:10:39  panther
// fix compare function...
//
// Revision 1.9  2003/11/23 08:42:41  panther
// Toying with the nearness floating point operator
//
// Revision 1.8  2003/09/01 20:04:37  panther
// Added OpenGL Interface to windows video lib, Modified RCOORD comparison
//
// Revision 1.7  2003/08/29 10:26:17  panther
// Checkpoint - converted vectlib to be native double
//
// Revision 1.6  2003/08/29 02:07:41  panther
// Fixed logging, and nearness comparison
//
// Revision 1.5  2003/08/27 07:56:40  panther
// Replace COMPARE macro with one that works a little better
//
// Revision 1.4  2003/03/25 08:38:11  panther
// Add logging
//
