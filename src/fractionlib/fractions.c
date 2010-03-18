
#include <stdhdrs.h>
#include <stdio.h>
#include <logging.h>

#include "fractions.h"


#ifdef __cplusplus
	namespace sack { namespace math { namespace fraction {
#endif
//---------------------------------------------------------------------------

FRACTION_PROC( int, sLogFraction )( TEXTCHAR *string, PFRACTION x )
{
	if( x->denominator < 0 )
	{
		if( x->numerator > -x->denominator )
			return snprintf( string, 31, WIDE("-%d %d/%d")
						, x->numerator / (-x->denominator)
						, x->numerator % (-x->denominator), -x->denominator );
		else
			return snprintf( string, 31, WIDE("-%d/%d"), x->numerator, -x->denominator );
	}
	else
	{
		if( x->numerator > x->denominator )
			return snprintf( string, 31, WIDE("%d %d/%d")
						, x->numerator / x->denominator
						, x->numerator % x->denominator, x->denominator );
		else
			return snprintf( string, 31, WIDE("%d/%d"), x->numerator, x->denominator );
	}
}

//---------------------------------------------------------------------------

FRACTION_PROC( int, sLogCoords )( TEXTCHAR *string, PCOORDPAIR pcp )
{
	TEXTCHAR *start = string;
	string += snprintf( string, 2, WIDE("(") );
	string += sLogFraction( string, &pcp->x );
	string += snprintf( string, 2, WIDE(",") );
	string += sLogFraction( string, &pcp->y );
	string += snprintf( string, 2, WIDE(")") );
	return (int)(string - start);
}

FRACTION_PROC( void, LogCoords )( PCOORDPAIR pcp )
{
	TEXTCHAR buffer[256];
	TEXTCHAR *string = buffer;
	string += snprintf( string, 2, WIDE("(") );
	string += sLogFraction( string, &pcp->x );
	string += snprintf( string, 2, WIDE(",") );
	string += sLogFraction( string, &pcp->y );
	string += snprintf( string, 2, WIDE(")") );
	Log( buffer );
}
//---------------------------------------------------------------------------

static void NormalizeFraction( PFRACTION f )
{
	int n;
	for( n = 2; n < 32; n++ )
	{
		if( ( ( f->numerator % n) == 0 ) &&
		    ( ( f->denominator % n ) == 0 ) )
		{
			f->numerator /= n;
			f->denominator /= n;
			n = 1; // one cause we add one before looping again;
			continue;
		}
	}
}

//---------------------------------------------------------------------------

FRACTION_PROC( PFRACTION, AddFractions )( PFRACTION base, PFRACTION offset )
{
	if( !offset->numerator ) // 0 addition either way is same.
		return base;
	//LogFraction( base );
	//fprintf( log, WIDE(" + ") );
	//LogFraction( offset );
	//fprintf( log, WIDE(" = ") );
	if( base->denominator < 0 )
	{
		if( offset->denominator < 0 )
		{
			// result is MORE negative when adding them... this is good.
			if( offset->denominator == base->denominator )
			{
				base->numerator += offset->numerator;
			}
			else
			{
				// results in a positive value
				base->numerator = -( ( base->numerator * offset->denominator ) +
				                     ( offset->numerator * base->denominator ) );
				base->denominator *= offset->denominator;
				// need to retain it's original sign....
				base->denominator = -base->denominator;
			}
		}
		else
		{
			// base (probably small negative) - offset + (probably a BIG positive)
			if( offset->denominator == -base->denominator )
			{
				base->numerator -= offset->numerator;
			}
			else
			{
				// result is positive - which is original sign.
				base->numerator = -( ( base->numerator * offset->denominator ) +
									      ( offset->numerator * base->denominator ) );
				base->denominator *= -offset->denominator;
			}
		}
	}
	else
	{
		if( offset->denominator < 0 )
		{
			// correct - base positive, offset negative
			// results in a positive addition from the origin...
			// making it less negative and closer to the bottom/right
			if( offset->denominator == -base->denominator )
			{
				base->numerator = offset->numerator - base->numerator;
				base->denominator = -base->denominator;
			}
			else
			{
				base->numerator = ( base->numerator * offset->denominator ) +
									   ( offset->numerator * base->denominator );
				base->denominator *= offset->denominator;
			}
		}
		else
		{
			if( offset->denominator == base->denominator )
			{
				base->numerator += offset->numerator;
			}
			else
			{
				base->numerator = ( base->numerator * offset->denominator ) +
										( offset->numerator * base->denominator );
				base->denominator *= offset->denominator;
			}
		}
	}
	NormalizeFraction( base );
	return base;
	//LogFraction( base );
	//fprintf( log, WIDE("\n") );
}

//---------------------------------------------------------------------------

FRACTION_PROC( void, AddCoords )( PCOORDPAIR base, PCOORDPAIR offset )
{
	AddFractions( &base->x, &offset->x );
	AddFractions( &base->y, &offset->y );
}

//---------------------------------------------------------------------------

FRACTION_PROC( PFRACTION, ScaleFraction )( PFRACTION result, S_32 value, PFRACTION f )
{
	result->numerator = value * f->numerator;
	result->denominator = f->denominator;
	return result;
}


//---------------------------------------------------------------------------

FRACTION_PROC( _32, ScaleValue )( PFRACTION f, S_32 value )
{
	S_32 result = 0;
if( f->denominator )
	result = ( value * f->numerator ) / f->denominator;
	return result;
}

//---------------------------------------------------------------------------

FRACTION_PROC( _32, InverseScaleValue )( PFRACTION f, S_32 value )
{
	S_32 result =0;
if( f->numerator )
	result = ( value * f->denominator ) / f->numerator;
	return result;
}

//---------------------------------------------------------------------------

FRACTION_PROC( S_32, ReduceFraction )( PFRACTION f )
{
	return ( f->numerator ) / f->denominator;
}

#ifdef __cplusplus
	}}}//	namespace sack { namespace math { namespace fraction {
#endif

//---------------------------------------------------------------------------
// $Log: fractions.c,v $
// Revision 1.6  2005/01/27 07:39:23  panther
// Linux cleaned.
//
// Revision 1.5  2004/09/03 14:43:47  d3x0r
// flexible frame reactions to font changes...
//
// Revision 1.4  2003/03/25 08:45:50  panther
// Added CVS logging tag
//
