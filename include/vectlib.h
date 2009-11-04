#ifndef ROTATE_DECLARATION
#define ROTATE_DECLARATION

// one day I'd like to make a multidimensional library
// but for now - 3D is sufficient - it can handle everything
// under 2D ignoring the Z axis... although it would be more
// efficient to do 2D implementation alone... 
// but without function overloading the names of all the functions
// become much too complex.. well perhaps - maybe I can
// make all the required functions with a suffix - and 
// supply defines to choose the default based on the dimension number

#if !defined(__STATIC__) && !defined(__UNIX__)
#  ifdef VECTOR_LIBRARY_SOURCE
#    define MATHLIB_EXPORT EXPORT_METHOD
#    if defined( __WATCOMC__ ) || defined( _MSC_VER )
// data requires an extra extern to generate the correct code *boggle*
#      define MATHLIB_DEXPORT extern EXPORT_METHOD
#    else
#      define MATHLIB_DEXPORT EXPORT_METHOD
#    endif
#  else
#    define MATHLIB_EXPORT IMPORT_METHOD
#    if defined( __WATCOMC__ ) || defined( _MSC_VER )
// data requires an extra extern to generate the correct code *boggle*
#      ifndef __cplusplus_cli
#        define MATHLIB_DEXPORT extern IMPORT_METHOD
#      else
#        define MATHLIB_DEXPORT IMPORT_METHOD
#      endif
#    else
#      define MATHLIB_DEXPORT IMPORT_METHOD
#    endif
#  endif
#else
#ifndef VECTOR_LIBRARY_SOURCE
#define MATHLIB_EXPORT extern
#define MATHLIB_DEXPORT extern
#else
#define MATHLIB_EXPORT
#define MATHLIB_DEXPORT
#endif
#endif

#define DIMENSIONS 3

#if( DIMENSIONS > 0 )
   #define vRight   0
   #define _1D(exp)  exp
   #if( DIMENSIONS > 1 )
      #define vUp      1
      #define _2D(exp)  exp
      #if( DIMENSIONS > 2 )
         #define vForward 2
         #define _3D(exp)  exp
         #if( DIMENSIONS > 3 )
            #define vIn      3  // 4th dimension 'IN'/'OUT' since projection is scaled 3d...
            #define _4D(exp)  exp
         #else
            #define _4D(exp)
         #endif
      #else
         #define _3D(exp)
         #define _4D(exp)
      #endif
   #else
      #define _2D(exp)
      #define _3D(exp)
      #define _4D(exp)
   #endif
#else
   // print out a compiler message can't perform zero-D transformations...
#endif

#ifdef __cplusplus
#define VECTOR_NAMESPACE SACK_NAMESPACE namespace math { namespace vector {
#define VECTOR_NAMESPACE_END } } SACK_NAMESPACE_END
#define USE_VECTOR_NAMESPACE using namespace sack::math::vector;
#else
#define VECTOR_NAMESPACE
//extern "C" {
#define VECTOR_NAMESPACE_END
//}
#define USE_VECTOR_NAMESPACE 
#endif


#include "vectypes.h"

VECTOR_NAMESPACE

#include "../src/vectlib/vecstruc.h"



typedef RCOORD _POINT[DIMENSIONS];
typedef RCOORD *P_POINT;
typedef const RCOORD *PC_POINT;

typedef _POINT VECTOR;
typedef P_POINT PVECTOR;
typedef PC_POINT PCVECTOR;

typedef struct {
   _POINT o; // origin
   _POINT n; // normal
} RAY, *PRAY;

typedef struct lineseg_tag {
	RAY r; // direction/slope
	RCOORD dFrom, dTo;
} LINESEG, *PLINESEG;

typedef struct orthoarea_tag {
    RCOORD x, y;
    RCOORD w, h;
} ORTHOAREA, *PORTHOAREA;

// relics from fixed point math dayz....
#define ZERO (0.0f)
#define ONE  (1.0f)

#ifndef M_PI
#define M_PI (3.1415926535)
#endif

#define _5  (RCOORD)((5.0/180.0)*M_PI )
#define _15 (RCOORD)((15.0/180.0)*M_PI )
#define _30 (RCOORD)((30.0/180.0)*M_PI )
#define _45 (RCOORD)((45.0/180.0)*M_PI )

// should end up layering this macro based on DIMENSIONS
#define SetPoint( d, s ) ( (d)[0] = (s)[0], (d)[1]=(s)[1], (d)[2]=(s)[2] )
// invert vector....
MATHLIB_EXPORT P_POINT Invert( P_POINT a );
//#define Invert( a ) { a[0] = -a[0], a[1]=-a[1], a[2]=-a[2]; }

MATHLIB_EXPORT void PrintVectorEx( char *lpName, PCVECTOR v DBG_PASS );
#define PrintVector(v) PrintVectorEx( #v, v DBG_SRC )
MATHLIB_EXPORT void PrintVectorStdEx( char *lpName, VECTOR v DBG_PASS );
#define PrintVectorStd(v) PrintVectorStd( #v, v DBG_SRC )
MATHLIB_EXPORT void PrintMatrixEx( char *lpName, MATRIX m DBG_PASS );
#define PrintMatrix(m) PrintMatrixEx( #m, m DBG_SRC )

typedef struct transform_tag *PTRANSFORM, TRANSFORM;
typedef const TRANSFORM *PCTRANSFORM;
typedef const PCTRANSFORM *CPCTRANSFORM;

//------ Constants for origin(0,0,0), and axii
#ifndef VECTOR_LIBRARY_SOURCE
MATHLIB_DEXPORT const PC_POINT _0;
MATHLIB_DEXPORT const PC_POINT _X;
MATHLIB_DEXPORT const PC_POINT _Y;
MATHLIB_DEXPORT const PC_POINT _Z;
MATHLIB_DEXPORT const PCTRANSFORM _I;
#define _0 ((PC_POINT)_0)
#define _X ((PC_POINT)_X)
#define _Y ((PC_POINT)_Y)
#define _Z ((PC_POINT)_Z)
#endif

                                   
#define Near( a, b ) ( COMPARE(a[0],b[0]) && COMPARE( a[1], b[1] ) && COMPARE( a[2], b[2] ) )

MATHLIB_EXPORT P_POINT add( P_POINT pr, PC_POINT pv1, PC_POINT pv2 );
MATHLIB_EXPORT P_POINT sub( P_POINT pr, PC_POINT pv1, PC_POINT pv2 );
MATHLIB_EXPORT P_POINT scale( P_POINT pr, PC_POINT pv1, RCOORD k );
MATHLIB_EXPORT P_POINT addscaled( P_POINT pr, PC_POINT pv1, PC_POINT pv2, RCOORD k );
MATHLIB_EXPORT void normalize( P_POINT pv );
MATHLIB_EXPORT void crossproduct( P_POINT pr, PC_POINT pv1, PC_POINT pv2 );
MATHLIB_EXPORT RCOORD SinAngle( PC_POINT pv1, PC_POINT pv2 );
MATHLIB_EXPORT RCOORD CosAngle( PC_POINT pv1, PC_POINT pv2 );
MATHLIB_EXPORT RCOORD dotproduct( PC_POINT pv1, PC_POINT pv2 );
// result is the projection of project onto onto
MATHLIB_EXPORT P_POINT project( P_POINT pr, PC_POINT onto, PC_POINT project );
MATHLIB_EXPORT RCOORD Length( PC_POINT pv );
MATHLIB_EXPORT RCOORD Distance( PC_POINT v1, PC_POINT v2 );

#define SetRay( pr1, pr2 ) { SetPoint( (pr1)->o, (pr2)->o ),  \
                             SetPoint( (pr1)->n, (pr2)->n ); }



MATHLIB_EXPORT PTRANSFORM CreateTransform ( void );
MATHLIB_EXPORT void DestroyTransform      ( PTRANSFORM pt );
MATHLIB_EXPORT void ClearTransform        ( PTRANSFORM pt );
MATHLIB_EXPORT void InvertTransform        ( PTRANSFORM pt );
MATHLIB_EXPORT void Scale                 ( PTRANSFORM pt, RCOORD sx, RCOORD sy, RCOORD sz );

MATHLIB_EXPORT void Translate             ( PTRANSFORM pt, RCOORD tx, RCOORD ty, RCOORD tz );
MATHLIB_EXPORT void TranslateV            ( PTRANSFORM pt, PC_POINT t );
MATHLIB_EXPORT void TranslateRel          ( PTRANSFORM pt, RCOORD tx, RCOORD ty, RCOORD tz );
MATHLIB_EXPORT void TranslateRelV         ( PTRANSFORM pt, PC_POINT t );


MATHLIB_EXPORT void RotateAbs( PTRANSFORM pt, RCOORD rx, RCOORD ry, RCOORD rz );
MATHLIB_EXPORT void RotateAbsV( PTRANSFORM pt, PC_POINT );
MATHLIB_EXPORT void RotateRel( PTRANSFORM pt, RCOORD rx, RCOORD ry, RCOORD rz );
MATHLIB_EXPORT void RotateRelV( PTRANSFORM pt, PC_POINT );
MATHLIB_EXPORT void RotateAround( PTRANSFORM pt, PC_POINT p, RCOORD amount );
MATHLIB_EXPORT void RotateMast( PTRANSFORM pt, PCVECTOR vup );
MATHLIB_EXPORT void RotateAroundMast( PTRANSFORM pt, RCOORD angle );

MATHLIB_EXPORT void LoadTransform( PTRANSFORM pt, CTEXTSTR filename );
MATHLIB_EXPORT void SaveTransform( PTRANSFORM pt, CTEXTSTR filename );


MATHLIB_EXPORT void RotateTo( PTRANSFORM pt, PCVECTOR vforward, PCVECTOR vright );
MATHLIB_EXPORT void RotateRight( PTRANSFORM pt, int A1, int A2 );

MATHLIB_EXPORT void Apply           ( PCTRANSFORM pt, P_POINT dest, PC_POINT src );
MATHLIB_EXPORT void ApplyR          ( PCTRANSFORM pt, PRAY dest, PRAY src );
MATHLIB_EXPORT void ApplyT          ( PCTRANSFORM pt, PTRANSFORM dest, PTRANSFORM src );
// I know this was a result - unsure how it was implented...
//void ApplyT              (PTRANFORM pt, PTRANSFORM pt1, PTRANSFORM pt2 );

MATHLIB_EXPORT void ApplyInverse    ( PCTRANSFORM pt, P_POINT dest, PC_POINT src );
MATHLIB_EXPORT void ApplyInverseR   ( PCTRANSFORM pt, PRAY dest, PRAY src );
MATHLIB_EXPORT void ApplyInverseT   ( PCTRANSFORM pt, PTRANSFORM dest, PTRANSFORM src );
// again note there was a void ApplyInverseT

MATHLIB_EXPORT void ApplyRotation        ( PCTRANSFORM pt, P_POINT dest, PC_POINT src );
MATHLIB_EXPORT void ApplyRotationT       ( PCTRANSFORM pt, PTRANSFORM ptd, PTRANSFORM pts );

MATHLIB_EXPORT void ApplyInverseRotation ( PCTRANSFORM pt, P_POINT dest, PC_POINT src );

MATHLIB_EXPORT void ApplyInverseRotationT( PCTRANSFORM pt, PTRANSFORM ptd, PTRANSFORM pts );

MATHLIB_EXPORT void ApplyTranslation     ( PCTRANSFORM pt, P_POINT dest, PC_POINT src );
MATHLIB_EXPORT void ApplyTranslationT    ( PCTRANSFORM pt, PTRANSFORM ptd, PTRANSFORM pts );

MATHLIB_EXPORT void ApplyInverseTranslation( PCTRANSFORM pt, P_POINT dest, PC_POINT src );
MATHLIB_EXPORT void ApplyInverseTranslationT( PCTRANSFORM pt, PTRANSFORM ptd, PTRANSFORM pts );

// after Move() these callbacks are invoked.
typedef void (*MotionCallback)( PTRSZVAL, PTRANSFORM );
MATHLIB_EXPORT void AddTransformCallback( PTRANSFORM pt, MotionCallback callback, PTRSZVAL psv );

MATHLIB_EXPORT RCOORD SetTimeScale(RCOORD scale );

MATHLIB_EXPORT PC_POINT SetSpeed( PTRANSFORM pt, PC_POINT s );
MATHLIB_EXPORT P_POINT GetSpeed( PTRANSFORM pt, P_POINT s );
MATHLIB_EXPORT PC_POINT SetAccel( PTRANSFORM pt, PC_POINT s );
MATHLIB_EXPORT P_POINT GetAccel( PTRANSFORM pt, P_POINT s );
MATHLIB_EXPORT void Forward( PTRANSFORM pt, RCOORD distance );
MATHLIB_EXPORT void Up( PTRANSFORM pt, RCOORD distance );
MATHLIB_EXPORT void Right( PTRANSFORM pt, RCOORD distance );
MATHLIB_EXPORT PC_POINT SetRotation( PTRANSFORM pt, PC_POINT r );

MATHLIB_EXPORT void Move( PTRANSFORM pt );
MATHLIB_EXPORT void Unmove( PTRANSFORM pt );

MATHLIB_EXPORT void showstdEx( PTRANSFORM pt, char *header );  // debug dump...
MATHLIB_EXPORT void ShowTransformEx( PTRANSFORM pt, char *header DBG_PASS );  // debug dump...
#define ShowTransform( n ) ShowTransformEx( n, #n DBG_SRC )
MATHLIB_EXPORT void showstd( PTRANSFORM pt, char *header );  // debug dump...


MATHLIB_EXPORT void GetOriginV( PTRANSFORM pt, P_POINT o ); 
MATHLIB_EXPORT PC_POINT GetOrigin( PTRANSFORM pt ); 

MATHLIB_EXPORT void GetAxisV( PTRANSFORM pt, P_POINT a, int n ); 
MATHLIB_EXPORT PC_POINT GetAxis( PTRANSFORM pt, int n ); 

MATHLIB_EXPORT void SetAxis( PTRANSFORM pt, RCOORD a, RCOORD b, RCOORD c, int n ); 
MATHLIB_EXPORT void SetAxisV( PTRANSFORM pt, PC_POINT a, int n ); 

MATHLIB_EXPORT void GetGLMatrix( PTRANSFORM pt, PMATRIX out );



#ifdef __cplusplus


class TransformationMatrix {
private:
#ifndef TRANSFORM_STRUCTURE
	//char data[32*4 + 4];

//   // requires [4][4] for use with opengl
//   MATRIX m;       // s*rcos[0][0]*rcos[0][1] sin sin   (0)
//                   // sin s*rcos[1][0]*rcos[1][1] sin   (0)
//                   // sin sin s*rcos[2][0]*rcos[2][0]   (0)
//                   // tx  ty  tz                        (1)
//
//   RCOORD s[3];
//        // [x][0] [x][1] = partials... [x][2] = multiplied value.
//
//   RCOORD speed[3]; // speed right, up, forward
//   RCOORD rotation[3]; // pitch, yaw, roll delta
//   int nTime; // rotation stepping for consistant rotation
	struct transform_tag data;
#else
	struct transform_tag data;
#endif
public:
	TransformationMatrix() {
  // 	clear();
	}
	~TransformationMatrix() {} // no result.
	inline void Translate( PC_POINT o ) 
   {	TranslateV( &data, o );	}
	inline void TranslateRel( PC_POINT o ) 
	{	TranslateRelV( &data, o );	}
	inline void Translate( RCOORD x, RCOORD y, RCOORD z ) 
	{	TranslateV( &data, &x );	}
	inline void _RotateTo( PC_POINT forward, PC_POINT right )
	{	//extern void RotateTo( PTRANSFORM pt, PCVECTOR vforward, PCVECTOR vright );
		RotateTo( &data, forward, right );
	}
	inline void RotateRel( RCOORD up, RCOORD right, RCOORD forward )
	{	
		RotateRelV( &data, &up );
	}
	inline void RotateRel( PC_POINT p )
	{	
		RotateRelV( &data, p );
	}
#if 0
	inline void Apply( P_POINT dest, PC_POINT src )
	{	//void Apply( PTRANSFORM pt, P_POINT dest, PC_POINT src );
		Apply( &data, dest, src );
	}
	inline void ApplyRotation( P_POINT dest, PC_POINT src )
	{	//void ApplyRotation( PTRANSFORM pt, P_POINT dest, PC_POINT src );
		ApplyRotation( &data, dest, src );
	}
	inline void Apply( PRAY dest, PRAY src )
	{	//void ApplyR( PTRANSFORM pt, PRAY dest, PRAY src );
		ApplyR( &data, dest, src );
	}								
	inline void ApplyInverse( P_POINT dest, PC_POINT src )
	{ // void ApplyInverse( PTRANSFORM pt, P_POINT dest, PC_POINT src );
		ApplyInverse( &data, dest, src );
	}
	inline void ApplyInverse( PRAY dest, PRAY src )
	{	void ApplyInverseR( PTRANSFORM pt, PRAY dest, PRAY src );
		ApplyInverseR( &data, dest, src );
	}
	inline void ApplyInverseRotation( P_POINT dest, PC_POINT src )
	{	void ApplyInverseRotation( PTRANSFORM pt, P_POINT dest, PC_POINT src );
		ApplyInverseRotation( &data, dest, src );
	}
	inline void SetSpeed( RCOORD x, RCOORD y, RCOORD z )
	{	sack::math::vector::SetSpeed( &data, &x );
	}
	inline void SetSpeed( PC_POINT p )
	{	sack::math::vector::SetSpeed( &data, p );
	}
	inline void SetRotation( PC_POINT p )
	{	sack::math::vector::SetRotation( &data, p );
	}
	inline void Move( void )
	{
		void Move( PTRANSFORM pt );
		sack::math::vector::Move( &data );
	}
	inline void Unmove( void )
	{
		void Unmove( PTRANSFORM pt );
		sack::math::vector::Unmove( &data );
	}
	inline void clear( void )
	{
		sack::math::vector::ClearTransform( &data );
	}
	inline void RotateAbs( RCOORD x, RCOORD y, RCOORD z )
	{
		sack::math::vector::RotateAbsV( (PTRANSFORM)&data, &x );
	}
#endif
	inline void _GetAxis( P_POINT result, int n )
	{
		GetAxisV( (PTRANSFORM)&data, result, n );
	}
	inline PC_POINT _GetAxis( int n )
	{
		return GetAxis( (PTRANSFORM)&data, n);
	}
	PC_POINT _GetOrigin( void )
	{	return GetOrigin( (PTRANSFORM)&data );
	}
	void _GetOrigin( P_POINT p )
	{	GetOriginV( &data, p );
	}
	void show( char *header )
	{
		ShowTransformEx( &data, header DBG_SRC );
	}
	void _RotateRight( int a, int b )
	{
		RotateRight( (PTRANSFORM)&data, a, b );
	}
};
#endif
VECTOR_NAMESPACE_END
USE_VECTOR_NAMESPACE
#endif
// $Log: vectlib.h,v $
// Revision 1.13  2004/08/22 09:56:41  d3x0r
// checkpoint...
//
// Revision 1.12  2004/02/02 22:43:35  d3x0r
// Add lineseg type and orthoarea (min/max box)
//
// Revision 1.11  2004/01/11 23:24:15  panther
// Fix type warnings, conflicts, fix const issues
//
// Revision 1.10  2004/01/11 23:11:49  panther
// Fix const typings
//
// Revision 1.9  2004/01/11 23:10:38  panther
// Include keyboard to avoid windows errors
//
// Revision 1.8  2004/01/04 20:54:18  panther
// Use PCTRANSFORM for prototypes
//
// Revision 1.7  2003/12/29 08:10:18  panther
// Added more functions for applying transforms
//
// Revision 1.6  2003/11/22 23:27:11  panther
// Fix type passed to printvector
//
// Revision 1.5  2003/09/01 20:04:37  panther
// Added OpenGL Interface to windows video lib, Modified RCOORD comparison
//
// Revision 1.4  2003/03/25 08:38:11  panther
// Add logging
//
