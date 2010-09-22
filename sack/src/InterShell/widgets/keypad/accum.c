#include <stdhdrs.h>
#include <sharemem.h>
#include <stdio.h>

// routines to handle accumulator registers....
#define ACCUMULATOR_STRUCTURE_DEFINED
typedef struct accumulator_tag *PACCUMULATOR;
#include "accum.h"

// maybe these should be 'saved' in 'non-volatile' ram...

#ifdef _WIN32
#define HEAP_BASE 0x2000000
#else
#define HEAP_BASE 0x20000
#endif

typedef struct accumulator_tag
{
	S_64 value;
	S_64 dec_base; // inversion factor for decimal application...
	_64 decimal;
	PVARTEXT pvt_text; // a handle-any-case type text collector...
	struct {
		_32 bDecimal : 1; // whether or not accumulator has a decimal.
		_32 bHaveDecimal : 1; // whether decimal has been entered...
		_32 bDollars : 1; // dollars (formatting)
		_32 bText : 1; // uses text operations instead of numeric
	} flags;
	struct accumulator_tag *next;
	struct accumulator_tag **me;
	void (*Updated)( PTRSZVAL psv, PACCUMULATOR accum );
	PTRSZVAL psvUpdated;
	TEXTCHAR name[];
} ACCUMULATOR;


static PMEM pHeap;
static PACCUMULATOR Accumulators;


PACCUMULATOR SetAccumulator( PACCUMULATOR accum, S_64 value )
{
	accum->value = value;
	if( accum->Updated )
		accum->Updated( accum->psvUpdated, accum );
   return accum;
}

S_64 GetAccumulatorValue( PACCUMULATOR accum )
{
   return accum->value;
}

void KeyIntoAccumulator( PACCUMULATOR accum, S_32 val, _32 base )
{
	// adds in to accumulator as if keyed in...
   /*
	if( accum->flags.bHaveDecimal )
	{
      // whatever the maxint over 10 is...
		if( accum->dec_base < (0xFFFFFFFFFFFFFFFFULL / 10 ) )
		{
			accum->decimal *= base;
			accum->decimal += val;
			accum->dec_base *= 10;
		}
	}
	else
	*/
	if( accum->flags.bText )
	{
      vtprintf( accum->pvt_text, WIDE( "%d" ), val );
	}
	else
	{
		accum->value *= base;
		accum->value += val;
	}
	if( accum->Updated )
		accum->Updated( accum->psvUpdated, accum );
}

void KeyDecimalIntoAccumulator( PACCUMULATOR accum )
{
	if( accum->flags.bText )
	{
      KeyTextIntoAccumulator( accum, WIDE( "." ) );
	}
	else if( accum->flags.bDecimal )
	{
		if( !accum->flags.bHaveDecimal )
		{
			accum->flags.bHaveDecimal = 1;
			accum->value *= 100; // shift left two digits (base 10)
			if( accum->flags.bDollars )
				accum->dec_base = 100;
			else
				accum->dec_base = 1;
		}
	}
	if( accum->Updated )
      accum->Updated( accum->psvUpdated, accum );
}

void ClearAccumulatorDigit( PACCUMULATOR accum, _32 base )
{
	if( accum->flags.bText )
	{
      KeyTextIntoAccumulator( accum, WIDE( "\b" ) );
	}
	else
	{
		// adds in to accumulator as if keyed in...
		accum->value /= base;
	}
	if( accum->Updated )
		accum->Updated( accum->psvUpdated, accum );
}

void ClearAccumulator( PACCUMULATOR accum )
{
	// adds in to accumulator as if keyed in...
	if( accum->flags.bText )
		VarTextEmpty( accum->pvt_text );
	else
	{
		accum->value = 0;
		accum->decimal = 0;
	}
	if( accum->Updated )
      accum->Updated( accum->psvUpdated, accum );
}

PACCUMULATOR AddAcummulator( PACCUMULATOR accum_dest, PACCUMULATOR accum_source )
{
	if( accum_dest->flags.bText )
	{
		if( accum_source->flags.bText )
		{
			PTEXT text = VarTextPeek( accum_source->pvt_text );
         vtprintf( accum_dest->pvt_text, WIDE( "%s" ), GetText( text ) );
		}
		else
		{
         vtprintf( accum_dest->pvt_text, WIDE( "%Ld" ), accum_source->value );
		}
	}
	else
	{
		accum_dest->value += accum_source->value;
	}
	if( accum_dest->Updated )
      accum_dest->Updated( accum_dest->psvUpdated, accum_dest );
   return accum_dest;
}

PACCUMULATOR TransferAccumluator( PACCUMULATOR accum_dest, PACCUMULATOR accum_source )
{
	if( accum_dest->flags.bText )
	{
		if( accum_source->flags.bText )
		{
			PTEXT text = VarTextGet( accum_source->pvt_text );
			vtprintf( accum_dest->pvt_text, WIDE( "%s" ), accum_source->pvt_text );
         Release( text );
		}
		else
		{
         vtprintf( accum_dest->pvt_text, WIDE( "%Ld" ), accum_source->value );
			accum_source->value = 0;
		}
	}
	else
	{
		accum_dest->value += accum_source->value;
		accum_source->value = 0;
	}
	if( accum_source->Updated )
      accum_source->Updated( accum_source->psvUpdated, accum_source );
	if( accum_dest->Updated )
      accum_dest->Updated( accum_dest->psvUpdated, accum_dest );
   return accum_dest;
}

int GetAccumulatorText( PACCUMULATOR accum, char *text, int nLen )
{
	int len = 0;
	if( accum->flags.bText )
	{
		PTEXT result = VarTextPeek( accum->pvt_text );
		if( result )
		{
			strncpy( text, GetText( result ), nLen );
			len = GetTextSize( result );
		}
	}
	else if( accum->flags.bDollars )
		len = snprintf( text, nLen, WIDE("$%Ld.%02Ld")
				 , accum->value / 100
				  , accum->value % 100 );
	else
		len = snprintf( text, nLen, WIDE("%Ld"), accum->value );
	return len;
}

PACCUMULATOR GetAccumulator( CTEXTSTR name, _32 flags )
{
   PACCUMULATOR accum;
	if( !pHeap )
	{
		_32 size = 0;
      /*
		pHeap = (PMEM)OpenSpaceEx( NULL, WIDE("Registers"), HEAP_BASE, &size );
		if( !pHeap && size == 1 ) // didn't exist... (or failed to open)
		{
			Log( WIDE("Didn't exist, failed to open...") );
         size = 0xFFFF;
			pHeap = (PMEM)OpenSpaceEx( NULL, WIDE("Registers"), HEAP_BASE, &size );
			Log1( WIDE("Created heap: %p"), pHeap );
			InitHeap( pHeap, size );
         Accumulators = HeapAllocate( pHeap, sizeof( PACCUMULATOR ) );
		}
      else if( pHeap )
		{
         PACCUMULATOR *ppAccums;
			Log( WIDE("Regsiters existed, use it.") );
         ppAccums = GetFirstUsedBlock( pHeap );
			Accumulators = *ppAccums;
		}
		else
      */
		{
         size = 0xFFF0; // don't make this ODD for ARM process
			pHeap = (PMEM)Allocate( size );
         ((P_32)pHeap)[0] = 0;
         InitHeap( pHeap, size );
			Log1( WIDE("Unhandled register open failure. (%ld)"), size );
		}
		if( !pHeap )
		{
         fprintf( stderr, WIDE("Abort! Could not allocate accumulators!") );
         exit(1);
		}
	}

	accum = Accumulators;
	while( accum )
	{
		if( strcmp( name, accum->name ) == 0 )
			break;
      accum = accum->next;
	}
	if( !accum )
	{
      int len;
		accum = (PACCUMULATOR)HeapAllocate( pHeap, ( sizeof( ACCUMULATOR ) + (len = strlen( name ) + 1 ) ) * sizeof( TEXTCHAR ) );
		StrCpyEx( accum->name, name, len );
		accum->value = 0;
		if( flags & ACCUM_DOLLARS )
			accum->flags.bDollars = 1;
		else
			accum->flags.bDollars = 0;
		if( flags & ACCUM_DECIMAL )
			accum->flags.bDecimal = 1;
		else
			accum->flags.bDecimal = 0;

		if( flags & ACCUM_TEXT )
		{
			accum->pvt_text = VarTextCreate();
			accum->flags.bText = 1;
		}
		else
		{
			accum->pvt_text = NULL;
			accum->flags.bText = 0;
		}
		accum->Updated = NULL;
		if(( accum->next = Accumulators ))
			Accumulators->me = &accum->next;
	}
	return accum;
}

void SetAccumulatorUpdateProc( PACCUMULATOR accum
									  , void (*Updated)(PTRSZVAL psv, PACCUMULATOR accum )
									  , PTRSZVAL psvUser
									  )
{
	if( accum )
	{
        accum->Updated = Updated;
        accum->psvUpdated = psvUser;
	}
}

void KeyTextIntoAccumulator( PACCUMULATOR accum, CTEXTSTR value ) // works with numeric, if value contains numeric characters
{
	if( accum )
	{
		if( accum->flags.bText )
		{
			CTEXTSTR p = value;
			while( p[0] )
			{
				VarTextAddCharacter( accum->pvt_text, p[0] );
				p++;
			}
			if( accum->Updated )
				accum->Updated( accum->psvUpdated, accum );
		}
		else
		{
         // evaluate value and operate on numbers
		}
	}
}


