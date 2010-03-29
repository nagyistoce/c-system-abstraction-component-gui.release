/*
 *  Crafted by James Buckeyne
 *
 *   (c) Freedom Collective 2000-2006++
 *
 *   code to provide a robust text class for C
 *   Parsing, text substitution, replacment, phrase splitting
 *   options for paired parsing of almost all pairable symbols
 *   used in common language.
 *   
 *
 * see also - include/typelib.h
 *
 */

#include <stdhdrs.h>
#include <deadstart.h>
#include <stdarg.h>
#include <sack_types.h>
#include <sharemem.h>
#include <logging.h>

#include <stdio.h>

#ifdef __cplusplus
namespace sack {
namespace containers {
namespace text {
	using namespace sack::memory;
	using namespace sack::logging;
	using namespace sack::containers::queue;
#endif


typedef PTEXT (CPROC*GetTextOfProc)( PTRSZVAL, POINTER );

typedef struct text_exension_tag {
	_32 bits;
	GetTextOfProc TextOf;
	PTRSZVAL psvData;
}  TEXT_EXTENSION, *PTEXT_EXTENSION;

typedef struct vartext_tag {
	TEXTSTR collect_text;
	INDEX collect_used;
	INDEX collect_avail;
	_32   expand_by;
	PTEXT collect;
	PTEXT commit;  
} VARTEXT;


//#ifdef __cplusplus
static PTEXT newline;
static PTEXT blank;
PRELOAD( AllocateDefaults )
{
	newline = (PTEXT)SegCreateFromText( WIDE("") );
	blank = (PTEXT)SegCreateFromText( WIDE(" ") );
}
//#define newline (*newline)
//#define blank   (*blank)
//#else
//__declspec( dllexport ) TEXT newline = { TF_STATIC, NULL, NULL, {1,1},{0,WIDE("")}};
//__declspec( dllexport ) TEXT blank = { TF_STATIC, NULL, NULL, {1,1},{1,WIDE(" ")}};
//#endif
static PLIST pTextExtensions;
//---------------------------------------------------------------------------

PTEXT SegCreateEx( S_32 size DBG_PASS )
{
   PTEXT pTemp;
#if defined( _MSC_VER )
	//if( size > 0x8000 )
	//	_asm int 3;
#endif
   pTemp = (PTEXT)AllocateEx( sizeof(TEXT) + (size
#ifdef _MSC_VER 
	   + 1
#endif
	   )*sizeof(TEXTCHAR)
	   DBG_RELAY ); // good thing [1] is already counted.
   MemSet( pTemp, 0, sizeof(TEXT) + (size*sizeof(TEXTCHAR)) );
   pTemp->format.flags.prior_background = 1;
   pTemp->format.flags.prior_foreground = 1;
   pTemp->data.size = size; // physical space IS one more....
   return pTemp;
}

//---------------------------------------------------------------------------

PTEXT GetIndirect(PTEXT segment )
{
   if( !segment )
      return NULL;
	if( (segment->flags&TF_APPLICATION) )
	{
		INDEX idx;
		PTEXT_EXTENSION pte;
		LIST_FORALL( pTextExtensions, idx, PTEXT_EXTENSION, pte )
		{
			if( pte->bits & segment->flags )
			{
            // size is used as a pointer...
				segment = pte->TextOf( pte->psvData, (POINTER)segment->data.size );
				break;
			}
		}
		if( !pte )
         return NULL;
		return segment;
	}
   // if it's not indirect... don't result..
	if( !(segment->flags&TF_INDIRECT) )
      return NULL;
   return (PTEXT)(((CTEXTSTR)NULL) + segment->data.size);
}

//---------------------------------------------------------------------------

TEXTSTR GetText( PTEXT segment )
{
   while( segment )
	{
		if( segment->flags & (TF_INDIRECT|TF_APPLICATION) )
		{
			segment = GetIndirect( segment );
		}
		else
			return segment->data.data;
	}
	return NULL;
}

//---------------------------------------------------------------------------

INDEX GetTextSize( PTEXT segment )
{
	while( segment )
	{
		if( segment->flags & (TF_INDIRECT|TF_APPLICATION) )
		{
			segment = GetIndirect( segment );
		}
      else
			if( !segment->data.size )
			{
				if( segment->flags & IS_DATA_FLAGS )
				{
					//lprintf( "Is Data falgs returns 2. %08x", segment->flags & IS_DATA_FLAGS );
					return segment->data.size; // is data even if is not acurate....
				}
            break;
			}
			else
				return segment->data.size;
	}
   return 0;
}

//---------------------------------------------------------------------------

_32 GetTextFlags( PTEXT segment )
{
   if( !segment )
      return 0;
   if( segment->flags & (TF_INDIRECT|TF_APPLICATION) )
      return GetTextFlags( GetIndirect( segment ) );
   return segment->flags;
}

//---------------------------------------------------------------------------

PTEXT SegDuplicateEx( PTEXT pText DBG_PASS )
{
   PTEXT t;
   _32 n;
   if( pText )
	{
		if( pText->flags & TF_APPLICATION )
		{
			t = SegCreateIndirect( (PTEXT)pText->data.size );
         t->format = pText->format;
         t->flags = pText->flags;
		}
      else if( pText->flags & TF_INDIRECT )
      {
			t = SegCreateIndirectEx( SegDuplicateEx( GetIndirect( pText ) DBG_RELAY ) DBG_RELAY );
			t->format = pText->format;
			// some other mask needs to be here.. the getindirect
         // will have other flags...
         t->flags = pText->flags;
      }
      else
		{
			t = SegCreateEx( n = GetTextSize( pText ) DBG_RELAY );
         t->format = pText->format;
         MemCpy( GetText(t), GetText(pText), sizeof( TEXTCHAR ) * ( n + 1 ) );
         t->flags = pText->flags;
      }
		t->flags &= ~(TF_DEEP|TF_STATIC);
      return t;
   }
   return NULL;
}

//---------------------------------------------------------------------------

PTEXT LineDuplicateEx( PTEXT pText DBG_PASS )
{
   PTEXT pt;
   pt = pText;
   while( pt )
   {
      if( !(pt->flags&TF_STATIC) )
         HoldEx( (P_8)pt DBG_RELAY  );
      if( (pt->flags & TF_INDIRECT ) || (pt->flags&TF_APPLICATION) )
			LineDuplicateEx( GetIndirect( pt ) DBG_RELAY );
      pt = NEXTLINE( pt );
   }
   return pText;
}

//---------------------------------------------------------------------------

PTEXT TextDuplicateEx( PTEXT pText, int bSingle DBG_PASS )
{
   PTEXT pt;
   PTEXT pDup = NULL, pNew;
   pt = pText;
   while( pt )
   {
      if( (pt->flags & TF_INDIRECT ) && !(pt->flags&TF_APPLICATION) )
      {
         pNew = SegCreateIndirectEx(
                     TextDuplicateEx(
                           GetIndirect( pt ), bSingle DBG_RELAY ) DBG_RELAY );
			pNew->format.position = pt->format.position;
         pNew->flags |= pt->flags&(IS_DATA_FLAGS);
         pNew->flags |= TF_DEEP;
      }
      else
         pNew = SegDuplicateEx( pt DBG_RELAY );
      pDup = SegAppend( pDup, pNew );
      if( bSingle )
         break;
      pt = NEXTLINE( pt );
   }
   return pDup;
}

//---------------------------------------------------------------------------

PTEXT SegCreateFromTextEx( CTEXTSTR text DBG_PASS )
{
	PTEXT pTemp;
	int   nSize;
	if( text )
	{
		pTemp = SegCreateEx( nSize = (int)strlen( text ) DBG_RELAY );
		// include nul on copy
		MemCpy( pTemp->data.data, text, sizeof( TEXTCHAR ) * ( nSize + 1 ) );
		return pTemp;
	}
	return NULL;
}

//---------------------------------------------------------------------------

PTEXT SegCreateFromIntEx( int value DBG_PASS )
{
	PTEXT pResult;
	pResult = SegCreateEx( 12 DBG_RELAY);
	pResult->data.size = snprintf( pResult->data.data, 12, WIDE("%d"), value );
	return pResult;
}

//---------------------------------------------------------------------------

PTEXT SegCreateFrom_64Ex( S_64 value DBG_PASS )
{
   PTEXT pResult;
   pResult = SegCreateEx( 32 DBG_RELAY);
	pResult->data.size = snprintf( pResult->data.data, 32, WIDE("%")_64f, value );
   return pResult;
}

//---------------------------------------------------------------------------

PTEXT SegCreateFromFloatEx( float value DBG_PASS )
{
   PTEXT pResult;
   pResult = SegCreateEx( 32 DBG_RELAY);
   pResult->data.size = snprintf( pResult->data.data, 32, WIDE("%f"), value );
   return pResult;
}

//---------------------------------------------------------------------------

PTEXT SegCreateIndirectEx( PTEXT pText DBG_PASS )
{
   PTEXT pSeg;
   pSeg = SegCreateEx( -1 DBG_RELAY ); // no data content for indirect...
   pSeg->flags |= TF_INDIRECT;
   pSeg->data.size = (PTRSZVAL)pText;
   return pSeg;
}

//---------------------------------------------------------------------------

PTEXT SegBreak(PTEXT segment)  // remove leading segments.
{    // return leading segments!  might be ORPHANED if not handled.
   PTEXT temp;
   if( !segment )
      return NULL;
   if((temp=PRIORLINE(segment)))
      SETNEXTLINE(temp,NULL);
   SETPRIORLINE(segment,NULL);
   return(temp);
}

 _32  GetSegmentSpaceEx ( PTEXT segment, int position, int nTabs, INDEX *tabs)
{
	int total = 0;
	do
	{
		if( segment && !( segment->flags & TF_FORMATPOS ) )
		{
			int n;
			for( n = 0; n < nTabs && (INDEX)position > tabs[n]; n++ );
			if( n < nTabs )
				// now position is before the first tab... such that
				for( ; n < nTabs && n < segment->format.position.tabs; n++ )
				{
					total += tabs[n]-position;
					position = tabs[n];
				}
         lprintf( WIDE("Adding %d spaces"), segment->format.position.spaces );
			total += segment->format.position.spaces;
		}
	}
	while( (segment->flags & TF_INDIRECT) && ( segment = GetIndirect( segment ) ) );
	return total;
}
//---------------------------------------------------------------------------
 _32  GetSegmentSpace ( PTEXT segment, int position, int nTabSize )
{
	int total = 0;
	do
	{
		if( segment && !( segment->flags & TF_FORMATPOS ) )
		{
			int n;
			for( n = 0; n < segment->format.position.tabs; n++ )
			{
				if( !total )
					// I think this is wrong.  need to validate this equation.
					total += (position % nTabSize) + 1;
				else
					total += nTabSize;
			}
			total += segment->format.position.spaces;
		}
	}
	while( (segment->flags & TF_INDIRECT) && ( segment = GetIndirect( segment ) ) );
	return total;
}
//---------------------------------------------------------------------------
 _32  GetSegmentLengthEx ( PTEXT segment, int position, int nTabs, INDEX *tabs )
{
	while( segment && segment->flags & TF_INDIRECT )
      segment = GetIndirect( segment );
   return GetSegmentSpaceEx( segment, position, nTabs, tabs ) + GetTextSize( segment );
}
//---------------------------------------------------------------------------
 _32  GetSegmentLength ( PTEXT segment, int position, int nTabSize )
{
	while( segment && segment->flags & TF_INDIRECT )
      segment = GetIndirect( segment );
   return GetSegmentSpace( segment, position, nTabSize ) + GetTextSize( segment );
}
//---------------------------------------------------------------------------

PTEXT SegAppend(PTEXT source,PTEXT other)
{
   PTEXT temp=source;

   if( temp )
   {
      if( other )
      {
         SetEnd(temp);
         SETNEXTLINE(temp,other);
         SETPRIORLINE(other,temp);
      }
   }
   else
   {
      source=other;  // nothing was before...
   }
   return(source);
}

//---------------------------------------------------------------------------

void SegReleaseEx( PTEXT seg DBG_PASS)
{
   if( seg )
      ReleaseEx( seg DBG_RELAY );
}

//---------------------------------------------------------------------------

PTEXT SegExpandEx(PTEXT source, int nSize DBG_PASS)
{
   PTEXT temp;
   //Log1( WIDE("SegExpand...%d"), nSize );
   temp = SegCreateEx( sizeof( TEXTCHAR)*( GetTextSize( source ) + nSize ) DBG_RELAY );
   if( source )
	{
	   MemCpy( temp->data.data, source->data.data, sizeof( TEXTCHAR)*(GetTextSize( source ) + 1) );
   	   temp->flags = source->flags;
	   temp->format = source->format;
   	   SegSubst( temp, source );
	   SegRelease( source );
	}
   return temp;
}

//---------------------------------------------------------------------------

void LineReleaseEx(PTEXT line DBG_PASS )
{
   PTEXT temp;

   if( !line )
      return;

   SetStart(line);
   while(line)
   {
      temp=NEXTLINE(line);
      if( !(line->flags&TF_STATIC) )
      {
			if( (( line->flags & (TF_INDIRECT|TF_DEEP) ) == (TF_INDIRECT|TF_DEEP) ) )
				if( !(line->flags & TF_APPLICATION) ) // if indirect, don't want to release application content
					LineReleaseEx( GetIndirect( line ) DBG_RELAY );
         ReleaseEx( line DBG_RELAY );
      }
      line=temp;
   }
}

//---------------------------------------------------------------------------

PTEXT SegConcatEx(PTEXT output,PTEXT input,S_32 offset,S_32 length DBG_PASS )
{
   S_32 idx=0,len=0;
   PTEXT newseg;
   SegAppend( output, newseg = SegCreate( length ) );
   output = newseg;
   //output=SegExpandEx(output, length DBG_RELAY); /* add 1 for a null */

   GetText(output)[0]=0;

   while (input&&idx<length)
   {
//#define min(a,b) (((a)<(b))?(a):(b))
      len = min( (S_32)GetTextSize( input ) - offset, length-idx );
      MemCpy( GetText(output) + idx,
              GetText(input) + offset,
              sizeof( TEXTCHAR ) * ( len + 1 ) );
      idx += len;
      offset = 0;
      input=NEXTLINE(input);
   }
   GetText(output)[idx]=0;
   return(output);
}

//---------------------------------------------------------------------------

PTEXT SegUnlink(PTEXT segment)
{
   PTEXT temp;
   if (segment)
   {
      if( ( temp = PRIORLINE(segment) ) )
         SETNEXTLINE(temp,NEXTLINE(segment));
      if( ( temp = NEXTLINE(segment) ) )
         SETPRIORLINE(temp,PRIORLINE(segment));
      SETPRIORLINE(segment, NULL);
      SETNEXTLINE(segment, NULL);
   }
   return segment;
}

//---------------------------------------------------------------------------

PTEXT SegGrab( PTEXT segment )
{
   SegUnlink( segment );
   return segment;
}

//---------------------------------------------------------------------------

PTEXT SegDelete( PTEXT segment )
{
   LineReleaseEx( SegGrab( segment ) DBG_SRC );
   return NULL;
}

//---------------------------------------------------------------------------


PTEXT SegInsert( PTEXT what, PTEXT before )
{
   PTEXT that_start = what ,
         that_end= what;
   SetStart( that_start );
   SetEnd( that_end );
   if( before )
   {
      if( ( that_start->Prior = before->Prior) )
         that_start->Prior->Next = that_start;
      if( ( that_end->Next = before ) )
         that_end->Next->Prior = that_end;
   }
   return what;
}

//---------------------------------------------------------------------------

PTEXT SegSubst( PTEXT _this, PTEXT that )
{
   PTEXT that_start = that ,
         that_end= that;
   SetStart( that_start );
   SetEnd( that_end );

   if( ( that_end->Next = _this->Next ) )
      that_end->Next->Prior = that_end;

   if( ( that_start->Prior = _this->Prior) )
      that_start->Prior->Next = that_start;

   _this->Next = NULL;
   _this->Prior = NULL;
   return _this;
}

//---------------------------------------------------------------------------

PTEXT SegSplitEx( PTEXT *pLine, int nPos  DBG_PASS)
{
	// there includes the character at nPos - so all calculations
	// on there are +1...
   PTEXT here, there;
   int nLen;
   nLen = GetTextSize( *pLine );
   if( nPos > nLen )
   {
      return NULL;
   }
	if( nPos == nLen )
		return *pLine;
   here = SegCreateEx( nPos DBG_RELAY );
   here->flags  = (*pLine)->flags;
   here->format = (*pLine)->format;
   there = SegCreateEx( (nLen - nPos) DBG_RELAY );
   there->flags  = (*pLine)->flags;
   there->format = (*pLine)->format;
   there->format.position.spaces = 0; // was two characters presumably...
	there->format.position.tabs = 0;

	MemCpy( GetText( here ), GetText( *pLine ), sizeof(TEXTCHAR)*nPos );
    GetText( here )[nPos] = 0;
	if( nLen - nPos )
	{
		MemCpy( GetText( there ), GetText( *pLine ) + nPos, sizeof(TEXTCHAR)*(nLen - nPos) );
        GetText( there )[nLen-nPos] = 0;
	}

   SETNEXTLINE( PRIORLINE( *pLine ), here );
   SETPRIORLINE( here, PRIORLINE( *pLine ) );
   SETNEXTLINE( here, there );
	SETPRIORLINE( there, here );
	SETNEXTLINE( there, NEXTLINE( *pLine ) );
	SETPRIORLINE( NEXTLINE( *pLine ), there );

	SETNEXTLINE( *pLine, NULL );
	SETPRIORLINE( *pLine, NULL );

	LineReleaseEx( *pLine DBG_RELAY );
   *pLine = here;
   return here;
}


//----------------------------------------------------------------------

TEXTCHAR NextCharEx( PTEXT input, INDEX idx )
{
	if( ( ++idx ) >= input->data.size )
	{
		idx -= input->data.size;
		input = NEXTLINE( input );
	}
	if( input )
		return input->data.data[idx];
	return 0;
}
#define NextChar() NextCharEx( input, index )
//----------------------------------------------------------------------

// In this final implementation - it was decided that for a general
// library, that expressions, escapes of expressions, apostrophes
// were of no consequence, and without expressions, there is no excess
// so this simply is text stream in, text stream out.

// these are just shortcuts - these bits of code were used repeatedly....

#define SET_SPACES() do {		word->format.position.spaces = (_16)spaces; \
		word->format.position.tabs = (_16)tabs;                             \
		spaces = 0;                                                         \
		tabs = 0; } while(0)


//static CTEXTSTR normal_punctuation=WIDE("\'\"\\({[<>]}):@%/,;!?=*&$^~#`");
//static CTEXTSTR not_punctuation;

PTEXT TextParse( PTEXT input, CTEXTSTR punctuation, CTEXTSTR filter_space, int bTabs, int bSpaces  DBG_PASS )
// returns a TEXT list of parsed data
{
//#define DBG_OVERRIDE DBG_SRC
#define DBG_OVERRIDE DBG_RELAY
   /* takes a line of input and creates a line equivalent to it, but
      burst into its block peices.*/
   VARTEXT out;
   PTEXT outdata=(PTEXT)NULL,
         word;
   TEXTSTR tempText;

   _32 index;
   INDEX size;

   TEXTCHAR character;
   _32 elipses = FALSE,
      spaces = 0, tabs = 0;

   if (!input)        // if nothing new to process- return nothing processed.
      return((PTEXT)NULL);

	VarTextInitEx( &out DBG_OVERRIDE );

   while (input)  // while there is data to process...
   {
		if( input->flags & TF_INDIRECT )
		{

      	word = VarTextGetEx( &out DBG_OVERRIDE );
      	if( word )
			{
            SET_SPACES();
      		outdata = SegAppend( outdata, word );
      	}
			outdata = SegAppend( outdata, burst( GetIndirect( input ) ) );
			input = NEXTLINE( input );
			continue;
		}
      tempText = GetText(input);  // point to the data to process...
      size = GetTextSize(input);
      if( input->format.position.spaces || input->format.position.tabs )
      {
      	word = VarTextGetEx( &out DBG_OVERRIDE );
      	if( word )
      	{
            SET_SPACES();
      		outdata = SegAppend( outdata, word );
      	}
      }
		spaces += input->format.position.spaces;
      tabs += input->format.position.tabs;
      //Log1( WIDE("Assuming %d spaces... "), spaces );
      for (index=0;(character = tempText[index]),
                   (index < size); index++) // while not at the
                                         // end of the line.
      {
         if( elipses && character != '.' )
         {
         	if( VarTextEndEx( &out DBG_OVERRIDE ) )
         	{
         		PTEXT word = VarTextGetEx( &out DBG_OVERRIDE );
         		if( word )
         		{
						SET_SPACES();
      	      	outdata = SegAppend( outdata, word );
      	      }
      	      //else
      	      //	Log( WIDE("VarTextGet Failed to result.") );
         	}
            elipses = FALSE;
         }
         else if( elipses ) // elipses and character is . - continue
         {
         	VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
            continue;
         }
			if( strchr( filter_space, character ) )
			{
            goto is_a_space;
			}
			else if( strchr( punctuation, character ) )
			{
				if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
				{
            	outdata = SegAppend( outdata, word );
					SET_SPACES();
               VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
					word = VarTextGetEx( &out DBG_OVERRIDE );
            	outdata = SegAppend( outdata, word );
            }
            else
            {
               VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
					word = VarTextGetEx( &out DBG_OVERRIDE );
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
			}
         else switch(character)
         {
         case '\n':
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
            outdata = SegAppend( outdata, SegCreate( 0 ) ); // add a line-break packet
            break;
			case ' ':
				if( bSpaces )
				{
			is_a_space:
					if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
					{
						SET_SPACES();
						outdata = SegAppend( outdata, word );
					}
					spaces++;
					break;
				}
				if(0) {
				case '\t':
					if( bTabs )
					{
						if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
						{
							SET_SPACES();
							outdata = SegAppend( outdata, word );
						}
						if( spaces )
						{
						//lprintf( WIDE("Input stream has mangled spaces and tabs.") );
                     spaces = 0; // assume that the tab takes care of appropriate spacing
						}
						tabs++;
						break;
					}
				} else if(0) {
				case '\r': // a space space character...
					if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
					{
						SET_SPACES();
						outdata = SegAppend( outdata, word );
					}
					break;
				} else if(0) {
				case '.': // handle multiple periods grouped (elipses)
					//goto NormalPunctuation;
					{
						TEXTCHAR c;
						if( ( !elipses &&
							  ( c = NextChar() ) &&
							  ( c == '.' ) ) )
						{
							if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
							{
								outdata = SegAppend( outdata, word );
								SET_SPACES();
							}
							VarTextAddCharacterEx( &out, '.' DBG_OVERRIDE );
							elipses = TRUE;
							break;
						}
						if( ( c = NextChar() ) &&
							( c >= '0' && c <= '9' ) )
						{
							// gather together as a floating point number...
							VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
							break;
						}
					}
				} else if(0) {
				case '-':  // work seperations flaming-long-sword
				case '+':
				{
               int c;
					if( ( c = NextChar() ) &&
						( c >= '0' && c <= '9' ) )
					{
						if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
						{
							outdata = SegAppend( outdata, word );
							SET_SPACES();
							// gather together as a sign indication on a number.
						}
						VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
						break;
					}
				}
//         NormalPunctuation:
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
            	outdata = SegAppend( outdata, word );
					SET_SPACES();
               VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
					word = VarTextGetEx( &out DBG_OVERRIDE );
            	outdata = SegAppend( outdata, word );
            }
            else
            {
               VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
					word = VarTextGetEx( &out DBG_OVERRIDE );
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
				break;
				}
			default:
            if( elipses )
            {
            	if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            	{
            		outdata = SegAppend( outdata, word );
						SET_SPACES();
            	}
               elipses = FALSE;
            }
            VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
            break;
         }
      }
      input=NEXTLINE(input);
   }

   if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) ) // any generic outstanding data?
   {
   	outdata = SegAppend( outdata, word );
		SET_SPACES();
   }

   SetStart(outdata);

   VarTextEmptyEx( &out DBG_OVERRIDE );

   return(outdata);
}



PTEXT burstEx( PTEXT input DBG_PASS )
// returns a TEXT list of parsed data
{
//#define DBG_OVERRIDE DBG_SRC
//#define DBG_OVERRIDE DBG_RELAY
   /* takes a line of input and creates a line equivalent to it, but
      burst into its block peices.*/
   VARTEXT out;
   PTEXT outdata=(PTEXT)NULL,
         word;
   TEXTSTR tempText;

   _32 index;
   INDEX size;

   TEXTCHAR character;
   _32 elipses = FALSE,
      spaces = 0, tabs = 0;

	if (!input)        // if nothing new to process- return nothing processed.
		return((PTEXT)NULL);

	VarTextInitEx( &out DBG_OVERRIDE );

   while (input)  // while there is data to process...
   {
		if( input->flags & TF_INDIRECT )
		{

      		word = VarTextGetEx( &out DBG_OVERRIDE );
      		if( word )
			{
				SET_SPACES();
      			outdata = SegAppend( outdata, word );
      		}
			outdata = SegAppend( outdata, burst( GetIndirect( input ) ) );
			input = NEXTLINE( input );
			continue;
		}
		tempText = GetText(input);  // point to the data to process...
		size = GetTextSize(input);
		if( input->format.position.spaces || input->format.position.tabs )
		{
      		word = VarTextGetEx( &out DBG_OVERRIDE );
      		if( word )
      		{
				SET_SPACES();
      			outdata = SegAppend( outdata, word );
      		}
		}
		spaces += input->format.position.spaces;
		tabs += input->format.position.tabs;
		//Log1( WIDE("Assuming %d spaces... "), spaces );
		for (index=0;(character = tempText[index]),
                   (index < size); index++) // while not at the
                                         // end of the line.
      {
         if( elipses && character != '.' )
         {
         	if( VarTextEndEx( &out DBG_OVERRIDE ) )
         	{
         		PTEXT word = VarTextGetEx( &out DBG_OVERRIDE );
         		if( word )
         		{
						SET_SPACES();
      	      	outdata = SegAppend( outdata, word );
      	      }
      	      //else
      	      //	Log( WIDE("VarTextGet Failed to result.") );
         	}
            elipses = FALSE;
         }
         else if( elipses ) // elipses and character is . - continue
         {
         	VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
            continue;
         }

         switch(character)
         {
         case '\n':
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
            outdata = SegAppend( outdata, SegCreate( 0 ) ); // add a line-break packet
            break;
         case ' ':
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
            spaces++;
            break;
         case '\t':
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
				}
				if( spaces )
				{
				//lprintf( WIDE("Input stream has mangled spaces and tabs.") );
               spaces = 0;
				}
            tabs++;
            break;
         case '\r': // a space space character...
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
            break;
         case '.': // handle multiple periods grouped (elipses)
            //goto NormalPunctuation;
            {
               TEXTCHAR c;
               if( ( !elipses &&
                     ( c = NextChar() ) &&
                     ( c == '.' ) ) )
               {
                  if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
                  {
                  	outdata = SegAppend( outdata, word );
							SET_SPACES();
                  }
                  VarTextAddCharacterEx( &out, '.' DBG_OVERRIDE );
                  elipses = TRUE;
                  break;
               }
               if( ( c = NextChar() ) &&
                   ( c >= '0' && c <= '9' ) )
               {
                  // gather together as a floating point number...
                  VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
                  break;
               }
            }
			case '-':  // work seperations flaming-long-sword
			case '+':
				{
               int c;
					if( ( c = NextChar() ) &&
						( c >= '0' && c <= '9' ) )
					{
						if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
                  {
                  	outdata = SegAppend( outdata, word );
							SET_SPACES();
                  }
						// gather together as a sign indication on a number.
						VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
						break;
					}
				}
         case '\'': // single quote bound
         case '\"': // double quote bound
         case '\\': // escape next thingy... unusable in c processor

         case '(': // expression bounders
         case '{':
         case '[':
         case '<':

         case ')': // expression closers
         case '}':
         case ']':
         case '>':

         case ':':  // internet addresses
         case '@':  // email addresses
         case '%':
         case '/':
         case ',':
         case ';':
         case '!':
         case '?':
         case '=':
         case '*':
         case '&':
         case '$':
         case '^':
         case '~':
         case '#':
         case '`':
//         NormalPunctuation:
            if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            {
            	outdata = SegAppend( outdata, word );
					SET_SPACES();
               VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
					word = VarTextGetEx( &out DBG_OVERRIDE );
            	outdata = SegAppend( outdata, word );
            }
            else
            {
               VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
					word = VarTextGetEx( &out DBG_OVERRIDE );
					SET_SPACES();
            	outdata = SegAppend( outdata, word );
            }
            break;

         default:
            if( elipses )
            {
            	if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) )
            	{
            		outdata = SegAppend( outdata, word );
						SET_SPACES();
            	}
               elipses = FALSE;
            }
            VarTextAddCharacterEx( &out, character DBG_OVERRIDE );
            break;
         }
      }
      input=NEXTLINE(input);
   }

   if( ( word = VarTextGetEx( &out DBG_OVERRIDE ) ) ) // any generic outstanding data?
   {
   	outdata = SegAppend( outdata, word );
		SET_SPACES();
   }

   SetStart(outdata);

   VarTextEmptyEx( &out DBG_OVERRIDE );

   return(outdata);
}

//---------------------------------------------------------------------------
#undef LineLengthExx
_32 LineLengthExx( PTEXT pt, _32 bSingle, PTEXT pEOL )
{
   return LineLengthExEx( pt, bSingle, 8, pEOL );
}

_32 LineLengthExEx( PTEXT pt, _32 bSingle, INDEX nTabsize, PTEXT pEOL )
{
   int   TopSingle = bSingle;
   PTEXT pStack[32];
   int   nStack;
   int   skipspaces = ( PRIORLINE(pt) != NULL );
   _32 length = 0;
   nStack = 0;
   while( pt )
   {
      if( pt->flags & TF_BINARY )
      {
         pt = NEXTLINE( pt );
			if( bSingle )
            break;
         continue;
      }

      if( !(pt->flags & ( IS_DATA_FLAGS | TF_INDIRECT)) &&
          !pt->data.size
        )
		{
			if( pEOL )
				length += pEOL->data.size;
			else
				length += 2; // full binary \r\n insertion assumed
		}
      else
      {
			if( skipspaces )
				skipspaces = FALSE;
			else
			{
				if( !(pt->flags & (TF_FORMATABS|TF_FORMATREL)) )
					length += GetSegmentSpace( pt, length, nTabsize ); // not-including NULL.
			}

         if( pt->flags&TF_INDIRECT )
         {
            bSingle = FALSE; // will be restored when we get back to top seg.
				pStack[nStack++] = pt;
            pt = GetIndirect( pt );
            //if( nStack >= 32 )
            //   DebugBreak();
            continue;
         }
			else
				length += GetTextSize( pt ); // not-including NULL.

stack_resume:
         if( pt->flags&TF_TAG )
            length += 2;
         if( pt->flags&TF_PAREN )
            length += 2;
         if( pt->flags&TF_BRACE )
            length += 2;
         if( pt->flags&TF_BRACKET )
            length += 2;
         if( pt->flags&TF_QUOTE )
            length += 2;
         if( pt->flags&TF_SQUOTE )
            length += 2;

      }
      if( bSingle )
      {
         bSingle = FALSE;
         break;
      }
      pt = NEXTLINE( pt );
   }
   if( nStack )
   {
      pt = pStack[--nStack];
		if( !nStack )
         bSingle = TopSingle;
      goto stack_resume;
   }
//   if( length > 60000 )
//      _asm int 3;
   return length;
}

#undef LineLengthEx
_32 LineLengthEx( PTEXT pt, _32 bSingle )
{
	return LineLengthExx( pt, bSingle, NULL );
}

//---------------------------------------------------------------------------

// attempts to build a solitary line segment from the text passed
// however, if there are color changes, or absolute position changes
// this cannot work... and it must provide multiple peices...

#undef BuildLineExx
PTEXT BuildLineExx( PTEXT pt, int bSingle, PTEXT pEOL DBG_PASS )
{
   return BuildLineExEx( pt, bSingle, 8, pEOL DBG_RELAY );
}

PTEXT BuildLineExEx( PTEXT pt, int bSingle, INDEX nTabsize, PTEXT pEOL DBG_PASS )
{
   TEXTSTR buf;
   int   TopSingle = bSingle;
   PTEXT pStack[32];
   int   nStack, firstadded;
   int   skipspaces = ( PRIORLINE(pt) != NULL );
   PTEXT pOut;
   PTRSZVAL ofs;

	{
		int len;
		len = LineLengthExx( pt,bSingle,pEOL );
		if( !len )
			return NULL;
		pOut = SegCreateEx( len DBG_RELAY );
		firstadded = TRUE;

      buf = GetText( pOut );
   }

   ofs = 0;
   nStack = 0;
   while( pt )
   {
      if( pt->flags & TF_BINARY )
      {
			pt = NEXTLINE( pt );
			if( bSingle )
            break;
         continue;
      }
      // test color fields vs PRIOR_COLOR
      // if either the color IS the prior color - OR the value IS PRIOR_COLOR
      // then they can still be collapsed... DEFAULT_COLOR MAY be prior color
      // but there's no real telling... default is more like after a
      // attribute reset occurs...
      if( firstadded )
      {
         pOut->format.flags.foreground = pt->format.flags.foreground;
         pOut->format.flags.background = pt->format.flags.background;
         firstadded = FALSE;
      }
      else
      {
			if( ( !pt->format.flags.prior_foreground &&
				  !pt->format.flags.default_foreground &&
               pt->format.flags.foreground != pOut->format.flags.foreground ) ||
             ( !pt->format.flags.prior_background &&
				  !pt->format.flags.default_background &&
               pt->format.flags.background != pOut->format.flags.background )
			  )
         {
            PTEXT pSplit;
            // ofs is the next valid character position....
            //Log( WIDE("Changing segment's color...") );
            if( ofs )
            {
               pSplit = SegSplitEx( &pOut, ofs DBG_RELAY );
					if( !pSplit )
					{
						Log2( WIDE("Line was shorter than offset: %") _32f WIDE(" vs %") PTRSZVALfs WIDE(""), GetTextSize( pOut ), ofs );
					}
               pOut = NEXTLINE( pSplit );
               // new segments takes on the new attributes...
               pOut->format.flags.foreground = pt->format.flags.foreground;
               pOut->format.flags.background = pt->format.flags.background;
            	//Log2( WIDE("Split at %d result %d"), ofs, GetTextSize( pOut ) );
            	buf = GetText( pOut );
               ofs = 0;
            }
            else
            {
               pOut->format.flags.foreground = pt->format.flags.foreground;
               pOut->format.flags.background = pt->format.flags.background;
            }
         }
      }

      if( !(pt->flags& (TF_INDIRECT|IS_DATA_FLAGS)) &&
          !pt->data.size
        )
      {
			if( pEOL )
			{
				MemCpy( buf + ofs, pEOL->data.data, sizeof( TEXTCHAR )*(pEOL->data.size + 1) );
				ofs += pEOL->data.size;
			}
			else
			{
	         buf[ofs++] = '\r';
		      buf[ofs++] = '\n';
			}
      }
      else
      {
			if( skipspaces )
			{
				skipspaces = FALSE;
			}
			else if( !(pt->flags & (TF_FORMATABS|TF_FORMATREL)) )
			{
				int spaces = GetSegmentSpace( pt, ofs, nTabsize );
				// else we cannot collapse into single line (similar to colors.)
				while( spaces-- )
				{
					buf[ofs++] = ' ';
				}
			}

      	// at this point spaces before tags, and after tags
      	// which used to be expression level parsed are not
      	// reconstructed correctly...
         if( pt->flags&TF_TAG )
            buf[ofs++] = '<';
         if( pt->flags&TF_PAREN )
            buf[ofs++] = '(';
         if( pt->flags&TF_BRACE )
            buf[ofs++] = '{';
         if( pt->flags&TF_BRACKET )
            buf[ofs++] = '[';
         if( pt->flags&TF_QUOTE )
            buf[ofs++] = '\"';
         if( pt->flags&TF_SQUOTE )
            buf[ofs++] = '\'';

         if( pt->flags&TF_INDIRECT )
			{
            bSingle = FALSE; // will be restored when we get back to top.
				pStack[nStack++] = pt;
            pt = GetIndirect( pt );
            //if( nStack >= 32 )
            //   DebugBreak();
            continue;
         }
         else
         {
				int len;
				MemCpy( buf+ofs, GetText( pt ), sizeof( TEXTCHAR) * (len = GetTextSize( pt ))+1 );
         		ofs += len;
         }

stack_resume:
         if( pt->flags&TF_SQUOTE )
            buf[ofs++] = '\'';
         if( pt->flags&TF_QUOTE )
            buf[ofs++] = '\"';
         if( pt->flags&TF_BRACKET )
            buf[ofs++] = ']';
         if( pt->flags&TF_BRACE )
            buf[ofs++] = '}';
         if( pt->flags&TF_PAREN )
            buf[ofs++] = ')';
         if( pt->flags&TF_TAG )
            buf[ofs++] = '>';
      }
      if( bSingle )
      {
         bSingle = FALSE;
         break;
      }
      pt = NEXTLINE( pt );
   }
   if( nStack )
	{
		pt = pStack[--nStack];
		if( !nStack )
         bSingle = TopSingle;
      goto stack_resume;
   }

   if( !pOut ) // have to return length instead of new text seg...
      return (PTEXT)ofs;
   SetStart( pOut ); // if formatting was inserted into the stream...
   return pOut;
}

#undef BuildLineEx
PTEXT BuildLineEx( PTEXT pt, int bSingle DBG_PASS )
{
	return BuildLineExx( pt, bSingle, FALSE DBG_RELAY );
}


PTEXT FlattenLine( PTEXT pLine )
{
    PTEXT pCur, p;
    pCur = pLine;
    // all indirected segments get promoted to
    // the first level...
    while( pCur )
    {
        if( pCur->flags & TF_STATIC )
        {
			  p = SegDuplicate( pCur );
			  if( p )
			  {
				  SegSubst( pCur, p );
				  if( pCur == pLine )
					  pLine = p;
				  LineReleaseEx( pCur DBG_SRC );
				  pCur = p;
			  }
			  else
			  {
					PTEXT next = NEXTLINE( pCur );
					SegGrab( pCur );
					LineRelease( pCur );
					pCur = next;
               continue;
			  }
		  }
        if( pCur->flags & TF_INDIRECT )
        {
            if( pCur->flags & TF_DEEP )
            {
                p = FlattenLine( GetIndirect( pCur ) );
                pCur->flags &= ~TF_DEEP;
            }
            else
            {
                p = TextDuplicate( GetIndirect( pCur ), FALSE );
            }
				if( p )
				{
					SegSubst( pCur, p );
					if( pCur == pLine )
						pLine = p;
					p->flags |= pCur->flags & (~(TF_INDIRECT|TF_DEEP));
					LineReleaseEx( pCur DBG_SRC );
					pCur = p;
				}
				else
				{
					PTEXT next = NEXTLINE( pCur );
					SegGrab( pCur );
					LineRelease( pCur );
               pCur = next;
				}
            continue;

        }
        pCur = NEXTLINE( pCur );
    }
    return pLine;
}

//----------------------------------------------------------------------------

POINTER GetApplicationPointer( PTEXT text )
{
   // okay indirects up to application data are okay.
	while( ( text->flags & TF_INDIRECT ) && !(text->flags & TF_APPLICATION) )
      return GetApplicationPointer( (PTEXT)text->data.size );
	if( text->flags & TF_APPLICATION )
		return (POINTER)text->data.size;
   return NULL;
}

//----------------------------------------------------------------------------

void SetApplicationPointer( PTEXT text, POINTER p)
{
   // sets only this segment.
	if( text )
	{
		text->flags |= TF_APPLICATION;
      text->data.size = (PTRSZVAL)p;
	}
}

//----------------------------------------------------------------------------

void RegisterTextExtension( _32 flags, PTEXT(CPROC*TextOf)(PTRSZVAL,POINTER), PTRSZVAL psvData)
{
	PTEXT_EXTENSION pte = (PTEXT_EXTENSION)Allocate( sizeof( TEXT_EXTENSION ) );
	pte->bits = flags;
	pte->TextOf = TextOf;
	pte->psvData = psvData;
	AddLink( &pTextExtensions, pte );
#if 0
   if( text && ( text->flags & TF_APPLICATION ) )
	{
		INDEX idx;
      PTEXT_EXENSTION pte;
		LIST_FORALL( pTextExtension, idx, PTEXT_EXTENSION, pte )
		{
			if( pte->flags & text->flags )
			{
				text = pte->TextOf( text );
            break;
			}
		}
	}
#endif
   return;
}

//---------------------------------------------------------------------------

int TextIs( PTEXT pText, CTEXTSTR string )
{
	CTEXTSTR data = GetText( pText );
	if( data )
		return !strcmp( data, string );
   return 0;
}

//---------------------------------------------------------------------------

int TextLike( PTEXT pText, CTEXTSTR string )
{
	CTEXTSTR data = GetText( pText );
	if( data )
		return !stricmp( data, string );
   return 0;
}

//---------------------------------------------------------------------------

int SameText( PTEXT l1, PTEXT l2 )
{
	CTEXTSTR d1 = GetText( l1 );
	CTEXTSTR d2 = GetText( l2 );
	if( d1 && d2 )
		return strcmp( d1, d2 );
	else if( d1 )
		return 1;
	else if( d2 )
		return -1;
   return 0;
}
//---------------------------------------------------------------------------

int LikeText( PTEXT l1, PTEXT l2 )
{
	CTEXTSTR d1 = GetText( l1 );
	size_t len1 = d1 ? strlen( d1 ) : 0;
	CTEXTSTR d2 = GetText( l2 );
	size_t len2 = d2 ? strlen( d2 ) : 0;

	if( d1 && d2 )
		return strnicmp( d1, d2, textmin( len1, len2 ) );
	else if( d1 )
		return 1;
	else if( d2 )
		return -1;
   return 0;
}

//---------------------------------------------------------------------------

int CompareStrings( PTEXT pt1, int single1
                  , PTEXT pt2, int single2
                  , int bExact )
{
   while( pt1 && pt2 )
   {
      while( pt1 &&
             pt1->flags && ( pt1->flags & TF_BINARY ) )
         pt1 = NEXTLINE( pt1 );
      while( pt2 &&
             pt2->flags && ( pt2->flags & TF_BINARY ) )
         pt2 = NEXTLINE( pt2 );
      if( !pt1 && pt2 )
         return FALSE;
      if( pt1 && !pt2 )
         return FALSE;
      if( bExact )
      {
         if( SameText( pt1, pt2 ) != 0 )
            return FALSE;
      }
      else
      {
         // Like returns string compare function literal...
         if( LikeText( pt1, pt2 ) != 0 )
            return FALSE;
      }
      if( !single1 )
      {
         pt1 = NEXTLINE( pt1 );
         if( pt1 &&
             !GetTextSize( pt1 ) && !(pt1->flags & IS_DATA_FLAGS))
            pt1 = NULL;
      }
      else
         pt1 = NULL;
      if( !single2 )
      {
         pt2 = NEXTLINE( pt2 );
         if( pt2 &&
             !GetTextSize( pt2 ) &&
             !(pt2->flags & IS_DATA_FLAGS))
            pt2 = NULL;
      }
      else
         pt2 = NULL;
   }
   if( !pt1 && !pt2 )
      return TRUE;
   return FALSE;
}

//--------------------------------------------------------------------------

S_64 IntCreateFromText( CTEXTSTR p )
{
   //CTEXTSTR p;
	int s;
	int begin;
	S_64 num;
	//p = GetText( pText );
	if( !p )
		return FALSE;
	//if( pText->flags & TF_INDIRECT )
	//   return IntCreateFromSeg( GetIndirect( pText ) );

	s = 0;
	num = 0;
	begin = TRUE;
	while( *p )
	{
		if( *p == '.' )
			break;
		else if( *p == '+' )
		{
		}
		else if( *p == '-' && begin)
		{
			s++;
		}
		else if( *p < '0' || *p > '9' )
		{
			break;
		}
		else
		{
			num *= 10;
			num += *p - '0';
		}
		begin = FALSE;
		p++;
	}
	if( s )
		num *= -1;
	return num;
}

//--------------------------------------------------------------------------

S_64 IntCreateFromSeg( PTEXT pText )
{
	CTEXTSTR p;
	p = GetText( pText );
	if( !pText || !p )
		return FALSE;
	if( pText->flags & TF_INDIRECT )
		return IntCreateFromSeg( GetIndirect( pText ) );
	return IntCreateFromText( p );
}

//--------------------------------------------------------------------------

double FloatCreateFromText( CTEXTSTR p, CTEXTSTR *vp )
{
   int s, begin, bDec = FALSE;
   double num;
   double base = 1;
   double temp;
   if( !p )
   {
	   if( vp )
		   (*vp) = p;
       return 0;
   }
   s = 0;
   num = 0;
   begin = TRUE;
   while( *p )
   {
      if( *p == '-' && begin )
      {
         s++;
      }
      else if( *p < '0' || *p > '9' )
      {
         if( *p == '.' )
         {
            bDec = TRUE;
            base = 0.1;
         }
         else
            break;
      }
      else
      {
         if( bDec )
         {
            temp = *p - '0';
            num += base * temp;
            base /= 10;
         }
         else
         {
            num *= 10;
            num += *p - '0';
         }
      }
      begin = FALSE;
      p++;
   }
   if( vp )
	   (*vp) = p;
   if( s )
      num *= -1;
   return num;
}

//--------------------------------------------------------------------------

double FloatCreateFromSeg( PTEXT pText )
{
   CTEXTSTR p;
   p = GetText( pText );
   if( !p )
      return FALSE;
   return FloatCreateFromText( p, NULL );
}

//--------------------------------------------------------------------------

// if bUseAll - all segments must be part of the number
// otherwise, only as many segments as are needed for the number are used...
int IsSegAnyNumberEx( PTEXT *ppText, double *fNumber, S_64 *iNumber, int *bIntNumber, int bUseAll )
{
	CTEXTSTR pCurrentCharacter;
	PTEXT pBegin;
	PTEXT pText = *ppText;
	int decimal_count, s, begin = TRUE, digits;
	// remember where we started...

	// if the first segment is indirect, collect it and only it
	// as the number... making indirects within a number what then?
	if( pText->flags & TF_INDIRECT )
	{
		int status;
		PTEXT pTemp = GetIndirect( pText );
		if( pTemp
			&& (status = IsSegAnyNumberEx( &pTemp, fNumber, iNumber, bIntNumber, TRUE )) )
		{
			// step to next token - so we toss just this
			// one indirect statement.
			if( fNumber || iNumber )
			{
				// if resulting with a number, then step the text...
				(*ppText) = NEXTLINE( pText );
			}
			return status;
		}
		// not a number....
		return FALSE;
	}
	pBegin = pText;
	decimal_count = 0;
	s = 0;
	digits = 0;
	while( pText )
	{
		// at this point... is this really valid?
		if( pText->flags & TF_INDIRECT )
		{
			lprintf( WIDE("Encountered indirect segment gathering number, stopping.") );
			break;
		}

		if( !begin &&
			( pText->format.position.spaces || pText->format.position.tabs ) )
		{
			// had to continue with new segment, but it had spaces so stop now
			break;
		}

		pCurrentCharacter = GetText( pText );
		while( pCurrentCharacter && *pCurrentCharacter )
		{
			if( *pCurrentCharacter == '.' )
			{
				if( !decimal_count )
					decimal_count++;
				else
					break;
			}
			else if( ((*pCurrentCharacter) == '-') && begin)
			{
				s++;
			}
			else if( ((*pCurrentCharacter) < '0') || ((*pCurrentCharacter) > '9') )
				break;
			else
				digits++;
			begin = FALSE;
			pCurrentCharacter++;
		}
		// invalid character - stop, we're to abort.
		if( *pCurrentCharacter )
			break;
		pText = NEXTLINE( pText );
	} //while( pText );
	if( bUseAll && pText )
		// it's not a number, cause we didn't use all segments to get one
		return FALSE;
	if( *pCurrentCharacter || ( decimal_count > 1 ) || !digits )
	{
		// didn't collect enough meaningful info to be a number..
		// or information in this state is
		return FALSE;
	}
	// yeah it was a number, update the incoming pointer...
	if( fNumber || iNumber )
	{
		// if resulting with a number, then step the text...
		(*ppText) = pText;
	}
	if( decimal_count == 1 )
	{
		if( fNumber )
			(*fNumber) = FloatCreateFromSeg( pBegin );
		if( bIntNumber )
			(*bIntNumber) = 0;
		return 2; // return specifically it's a floating point number
	}
	if( iNumber )
		(*iNumber) = IntCreateFromSeg( pBegin );
	if( bIntNumber )
		(*bIntNumber) = 1;
	return 1; // return yes, and it's an int number
}

//---------------------------------------------------------------------------

//#define VERBOSE_DEBUG_VARTEXT
//---------------------------------------------------------------------------
#define COLLECT_LEN 4096

void VarTextInitEx( PVARTEXT pvt DBG_PASS )
{
	pvt->commit = NULL;
	pvt->collect = SegCreateEx( COLLECT_LEN DBG_RELAY );
	pvt->collect_text = GetText( pvt->collect );
#ifdef VERBOSE_DEBUG_VARTEXT
	Log( WIDE("Resetting collect_used (init)") );
#endif
	pvt->collect_used = 0;
	pvt->collect_avail = COLLECT_LEN;
   pvt->expand_by = 32;
}

 PVARTEXT  VarTextCreateExEx ( _32 initial, _32 expand DBG_PASS )
{
	PVARTEXT pvt = (PVARTEXT)AllocateEx( sizeof( VARTEXT ) DBG_RELAY );

	pvt->commit = NULL;
	pvt->collect = SegCreateEx( initial DBG_RELAY );
	pvt->collect_text = GetText( pvt->collect );
	pvt->collect_used = 0;
	pvt->collect_avail = initial;
   pvt->expand_by = expand;
	return pvt;

}
//---------------------------------------------------------------------------

PVARTEXT VarTextCreateEx( DBG_VOIDPASS )
{
	PVARTEXT pvt = (PVARTEXT)AllocateEx( sizeof( VARTEXT ) DBG_RELAY );
	VarTextInitEx( pvt DBG_RELAY );
	return pvt;
}

//---------------------------------------------------------------------------

void VarTextDestroyEx( PVARTEXT *ppvt DBG_PASS )
{
	if( ppvt && *ppvt )
	{
		VarTextEmptyEx( *ppvt DBG_RELAY );
		ReleaseEx( (*ppvt) DBG_RELAY );
		*ppvt = NULL;
	}
}

//---------------------------------------------------------------------------

void VarTextEmptyEx( PVARTEXT pvt DBG_PASS )
{
	if( pvt )
	{
		LineReleaseEx( pvt->collect DBG_RELAY );
		LineReleaseEx( pvt->commit DBG_RELAY );
		MemSet( pvt, 0, sizeof( VARTEXT ) );
	}
}

//---------------------------------------------------------------------------

void VarTextAddCharacterEx( PVARTEXT pvt, TEXTCHAR c DBG_PASS )
{
	if( !pvt->collect )
		VarTextInitEx( pvt DBG_RELAY );
#ifdef VERBOSE_DEBUG_VARTEXT
	Log1( WIDE("Adding character %c"), c );
#endif
	if( c == '\b' )
	{
		if( pvt->collect_used )
		{
			pvt->collect_used--;
			pvt->collect_text[pvt->collect_used] = 0;
		}
	}
	else
	{
		pvt->collect_text[pvt->collect_used++] = c;
		if( pvt->collect_used >= pvt->collect_avail )
		{
			//lprintf( WIDE("Expanding segment to make sure we have room to extend...(old %d)"), pvt->collect->data.size );
			pvt->collect = SegExpandEx( pvt->collect, COLLECT_LEN DBG_RELAY );
			pvt->collect_avail = pvt->collect->data.size;
			pvt->collect_text = GetText( pvt->collect );
		}
	}
}

//---------------------------------------------------------------------------

int VarTextEndEx( PVARTEXT pvt DBG_PASS )
{
	if( pvt && pvt->collect_used ) // otherwise ofs will be 0...
	{
		PTEXT segs= SegSplitEx( &pvt->collect, pvt->collect_used DBG_RELAY );
		//lprintf( WIDE("End collect at %d %d"), pvt->collect_used, segs?segs->data.size:pvt->collect->data.size );
		if( !segs )
		{
			segs = pvt->collect;
		}

		//Log1( WIDE("Breaking collection adding... %s"), GetText( segs ) );
		// so now the remaining buffer( if any ) 
		// is assigned to collect into.
		// This results in...

		pvt->collect = NEXTLINE( segs );
		if( !pvt->collect ) // used all of the line...
		{
#ifdef VERBOSE_DEBUG_VARTEXT
			Log( WIDE("Starting with new buffers ") );
#endif
			VarTextInitEx( pvt DBG_RELAY );
		}
		else
		{
 			//Log1( WIDE("Remaining buffer is %d"), GetTextSize( pvt->collect ) );
			SegBreak( pvt->collect );
			pvt->collect_text = GetText( pvt->collect );
#ifdef VERBOSE_DEBUG_VARTEXT
			Log( WIDE("resetting collect_used after split") );
#endif
			pvt->collect_avail -= pvt->collect_used;
			pvt->collect_used = 0;
		}
		pvt->commit = SegAppend( pvt->commit, segs );
		return 1;
   }
   if( pvt && pvt->commit )
      return 1;
	return 0;
}

//---------------------------------------------------------------------------

PTEXT VarTextGetEx( PVARTEXT pvt DBG_PASS )
{
	if( !pvt )
	{
#ifdef VERBOSE_DEBUG_VARTEXT
		lprintf( DBG_FILELINEFMT "Get Text failed - no PVT." DBG_RELAY );
#endif
		return NULL;
	}
#ifdef VERBOSE_DEBUG_VARTEXT
	lprintf( DBG_FILELINEFMT "Grabbing the text from %p..." DBG_RELAY, pvt );
#endif
	if( VarTextEndEx( pvt DBG_RELAY ) )
	{
		PTEXT result = pvt->commit;
		pvt->commit = NULL;
		return result;
	}
   return NULL;
}

//---------------------------------------------------------------------------

 PTEXT  VarTextPeekEx ( PVARTEXT pvt DBG_PASS )
{
	if( !pvt )
		return NULL;
	if( pvt && pvt->collect_used ) // otherwise ofs will be 0...
	{
      SetTextSize( pvt->collect, pvt->collect_used );
      //VarTextAddCharacterEx( pvt, 0 DBG_RELAY );
		return pvt->collect;
	}
   return NULL;
}

//---------------------------------------------------------------------------

void VarTextExpandEx( PVARTEXT pvt, int size DBG_PASS)
{
	pvt->collect = SegExpandEx( pvt->collect, size DBG_RELAY );
	pvt->collect_text = GetText( pvt->collect );
	pvt->collect_avail += size;
}

//---------------------------------------------------------------------------

int VarTextLength( PVARTEXT pvt )
{
	//Log1( WIDE("Length is : %d"), pvt->collect_used );
	if( pvt )
		return pvt->collect_used;
	return 0;
}


//---------------------------------------------------------------------------

int vvtprintf( PVARTEXT pvt, CTEXTSTR format, va_list args )
{
	int len;
	int tries = 0;
#if defined( UNDER_CE ) || defined( _MSC_VER )// this might be unicode...
	while( 1 )
	{
		int destlen;
		if( pvt->collect_text )
		{
			len = StringCbVPrintf ( pvt->collect_text + pvt->collect_used
									, (destlen = pvt->collect_avail - pvt->collect_used) * sizeof( TEXTCHAR )
									, format, args );
		}
		else
			len = STRSAFE_E_INSUFFICIENT_BUFFER;
		if( len == STRSAFE_E_INSUFFICIENT_BUFFER )
		{
			tries++;
			if( tries == 10 )
			{
				lprintf( WIDE( "Single buffer expanded more then 2560" ) );
				return 0; // didn't add any
			}
			VarTextExpand( pvt, (256<pvt->expand_by)?pvt->expand_by:(256+pvt->expand_by)  );
			continue;
		}
		len = StrLen( pvt->collect_text + pvt->collect_used );
		pvt->collect_used += len;
		break;
	}
	return len;

#elif defined( __GNUC__ )
	{
#ifdef __GNUC__
		va_list tmp_args;
		va_copy( tmp_args, args );
#endif
		// len returns number of characters (not NUL)
		len = vsnprintf( NULL, 0, format
#ifdef __GNUC__
							, tmp_args
#else
							, args
#endif
							);
		if( !len ) // nothign to add... we'll get stuck looping if this is not checked.
			return 0;
#ifdef __GNUC__
      va_end( tmp_args );
#endif
		// allocate +1 for length with NUL
		if( ((_32)len+1) >= (pvt->collect_avail-pvt->collect_used) )
		{
			// expand when we need more room.
			VarTextExpand( pvt, (len+1<pvt->expand_by)?pvt->expand_by:(len+1+pvt->expand_by)  );
		}
#ifdef VERBOSE_DEBUG_VARTEXT
		Log3( WIDE("Print Length: %d into %d after %s"), len, pvt->collect_used, pvt->collect_text );
#endif
		// include NUL in the limit of characters able to print...
		vsnprintf( pvt->collect_text + pvt->collect_used, len+1, format, args );
	}
#elif defined( __WATCOMC__ )
	{
		int destlen;
		va_list _args;
		_args[0] = args[0];
		do {
#ifdef VERBOSE_DEBUG_VARTEXT
			Log2( WIDE("Print Length: ofs %d after %s")
				 , pvt->collect_used
				 , pvt->collect_text );
#endif
			args[0] = _args[0];
			//va_start( args, format );
			len = vsnprintf( pvt->collect_text + pvt->collect_used
								, destlen = pvt->collect_avail - pvt->collect_used
								, format, args );
			if( !len ) // nothign to add... we'll get stuck looping if this is not checked.
				return 0;
#ifdef VERBOSE_DEBUG_VARTEXT
			lprintf( WIDE("result of vsnprintf: %d(%d) \'%s\' (%s)")
					 , len, destlen
					 , pvt->collect_text
					 , format );
#endif
			if( len >= destlen )
				VarTextExpand( pvt, len + pvt->expand_by );
		} while( len >= destlen );
	}
#else
	// uhmm not sure what state this is then...
	{
		do {
			if( strcmp( format, "%s" ) == 0 && pvt->collect_used==7)
            DebugBreak();
			len = vsnprintf( pvt->collect_text + pvt->collect_used
								, pvt->collect_avail - pvt->collect_used
								, format, args );
			if( len < 0 )
				VarTextExpand( pvt, pvt->expand_by );
			//                VarTextExpandEx( pvt, 32 DBG_SRC );
		} while( len < 0 );
		//Log1( WIDE("Print Length: %d"), len );
	}
#endif
#ifdef VERBOSE_DEBUG_VARTEXT
	Log2( WIDE("used: %d plus %d"), pvt->collect_used , len );
#endif
	pvt->collect_used += len;
	return len;
}
//---------------------------------------------------------------------------

int vtprintfEx( PVARTEXT pvt , CTEXTSTR format, ... )
{
    va_list args;
	 va_start( args, format );
    return vvtprintf( pvt, format, args );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// PTEXT DumpText( PTEXT somestring )
//    PTExT (single data segment with full description \r in text)
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static char *Ops[] = {
	"FORMAT_OP_CLEAR_END_OF_LINE",
	"FORMAT_OP_CLEAR_START_OF_LINE",
	"FORMAT_OP_CLEAR_LINE ",
	"FORMAT_OP_CLEAR_END_OF_PAGE",
	"FORMAT_OP_CLEAR_START_OF_PAGE",
	"FORMAT_OP_CLEAR_PAGE",
	"FORMAT_OP_CONCEAL" 
	  , "FORMAT_OP_DELETE_CHARS" // background is how many to delete.
	  , "FORMAT_OP_SET_SCROLL_REGION" // format.x, y are start/end of region -1,-1 clears.
	  , "FORMAT_OP_GET_CURSOR" // this works as a transaction...
	  , "FORMAT_OP_SET_CURSOR" // responce to getcursor...

	  , "FORMAT_OP_PAGE_BREAK" // clear page, home page... result in page break...
	  , "FORMAT_OP_PARAGRAPH_BREAK" // break between paragraphs - kinda same as lines...
};

//---------------------------------------------------------------------------

static void BuildTextFlags( PVARTEXT vt, PTEXT pSeg )
{
	vtprintf( vt, WIDE( "Text Flags: " ));
	if( pSeg->flags & TF_STATIC )
		vtprintf( vt, WIDE( "static " ) );
	if( pSeg->flags & TF_QUOTE )
		vtprintf( vt, WIDE( "\"\" " ) );
	if( pSeg->flags & TF_SQUOTE )
		vtprintf( vt, WIDE( "\'\' " ) );
	if( pSeg->flags & TF_BRACKET )
		vtprintf( vt, WIDE( "[] " ) );
	if( pSeg->flags & TF_BRACE )
		vtprintf( vt, WIDE( "{} " ) );
	if( pSeg->flags & TF_PAREN )
		vtprintf( vt, WIDE( "() " ) );
	if( pSeg->flags & TF_TAG )
		vtprintf( vt, WIDE( "<> " ) );
	if( pSeg->flags & TF_INDIRECT )
		vtprintf( vt, WIDE( "Indirect " ) );
   /*
	if( pSeg->flags & TF_SINGLE )
	vtprintf( vt, WIDE( "single " ) );
   */
	if( pSeg->flags & TF_FORMATREL )
      vtprintf( vt, WIDE( "format x,y(REL) " ) );
	if( pSeg->flags & TF_FORMATABS )
      vtprintf( vt, WIDE( "format x,y " ) );
   else
		vtprintf( vt, WIDE( "format spaces " ) );

	if( pSeg->flags & TF_COMPLETE )
		vtprintf( vt, WIDE( "complete " ) );
	if( pSeg->flags & TF_BINARY )
		vtprintf( vt, WIDE( "binary " ) );
	if( pSeg->flags & TF_DEEP )
		vtprintf( vt, WIDE( "deep " ) );
#ifdef DEKWARE_APP_FLAGS
	if( pSeg->flags & TF_ENTITY )
		vtprintf( vt, WIDE( "entity " ) );
	if( pSeg->flags & TF_SENTIENT )
		vtprintf( vt, WIDE( "sentient " ) );
#endif
	if( pSeg->flags & TF_NORETURN )
		vtprintf( vt, WIDE( "NoReturn " ) );
	if( pSeg->flags & TF_LOWER )
		vtprintf( vt, WIDE( "Lower " ) );
	if( pSeg->flags & TF_UPPER )
		vtprintf( vt, WIDE( "Upper " ) );
	if( pSeg->flags & TF_EQUAL )
		vtprintf( vt, WIDE( "Equal " ) );
	if( pSeg->flags & TF_TEMP )
		vtprintf( vt, WIDE( "Temp " ) );
#ifdef DEKWARE_APP_FLAGS
	if( pSeg->flags & TF_PROMPT )
		vtprintf( vt, WIDE( "Prompt " ) );
	if( pSeg->flags & TF_PLUGIN )
		vtprintf( vt, WIDE( "Plugin=%02x " ), (_8)(( pSeg->flags >> 26 ) & 0x3f ) );
#endif
	
	if( (pSeg->flags & TF_FORMATABS ) )
		vtprintf( vt, WIDE( "Pos:%d,%d " )
				, pSeg->format.position.coords.x
				, pSeg->format.position.coords.y  );
	else if( (pSeg->flags & TF_FORMATREL ) )
		vtprintf( vt, WIDE( "Rel:%d,%d " )
				, pSeg->format.position.coords.x
				, pSeg->format.position.coords.y  );
	else
		vtprintf( vt, WIDE( "%d spaces " )
				, pSeg->format.position.spaces );
	
	if( pSeg->flags & TF_FORMATEX )
		vtprintf( vt, WIDE( "format extended(%s) length:%d" )
		           , Ops[ pSeg->format.flags.format_op
		                - FORMAT_OP_CLEAR_END_OF_LINE ] 
		           , GetTextSize( pSeg ) );
	else
		vtprintf( vt, WIDE( "Fore:%d Back:%d length:%d" )
					, pSeg->format.flags.foreground
					, pSeg->format.flags.background 
					, GetTextSize( pSeg ) );
}


PTEXT DumpText( PTEXT text )
{
	if( text )
	{
		PVARTEXT pvt = VarTextCreate();
		PTEXT textsave = text;
		while( text )
		{
			BuildTextFlags( pvt, text );
			vtprintf( pvt, WIDE( "\n->%s\n" ), GetText( text ) );
			text = NEXTLINE( text );
		}
		textsave = VarTextGet( pvt );
		VarTextDestroy( &pvt );
		return textsave;
	}
	return NULL;
}
//---------------------------------------------------------------------------

#ifdef __cplusplus
#ifdef __cplusplus_cli

char * WcharConvert ( const wchar_t *wch )
{
	// Conversion to char* :
   // Can just convert wchar_t* to char* using one of the 
   // conversion functions such as: 
   // WideCharToMultiByte()
   // wcstombs_s()
   // ... etc
	int len;
	for( len = 0; wch[len]; len++ );
   size_t convertedChars = 0;
   size_t  sizeInBytes = ((len + 1) * 2);
   errno_t err = 0;
   char    *ch = NewArray( char, sizeInBytes);
   err = wcstombs_s(&convertedChars, 
                    ch, sizeInBytes,
                    wch, sizeInBytes);
   if (err != 0)
      printf_s(WIDE( "wcstombs_s  failed!\n" ));

	return ch;

}

#endif

}; //namespace text {
}; //namespace containers {
}; // namespace sack {
#endif


// $Log: text.c,v $
// Revision 1.74  2005/06/01 16:44:34  jim
// SegDuplicate duplicates the first segment including all indirects down to the first segment which is not indirect.
//
// Revision 1.73  2005/06/01 10:02:19  d3x0r
// SegDuplicate shall duplicate the segment indirect up to the the first segment that is not indirect or that is NULL.
//
// Revision 1.72  2005/05/25 16:50:30  d3x0r
// Synch with working repository.
//
// Revision 1.73  2005/01/27 07:18:34  panther
// Linux cleaned.
//
// Revision 1.72  2005/01/26 02:52:38  panther
// Pass extended information on line release
//
// Revision 1.71  2004/11/05 02:34:51  d3x0r
// Minor mods...
//
// Revision 1.70  2004/09/26 20:17:33  d3x0r
// Calculat spacing on indirect segemnts better - all spaces on all indirected parts count.
//
// Revision 1.69  2004/09/17 16:18:30  d3x0r
// ...
//
// Revision 1.68  2004/09/15 16:11:25  d3x0r
// First - tear apart text object... added many many formatting options to it.
//
// Revision 1.67  2004/08/14 00:07:18  d3x0r
// Remove collapse macro which was justa bad idea.
//
// Revision 1.66  2004/07/29 02:30:19  d3x0r
// Protect against self destruction when getting segment space
//
// Revision 1.65  2004/06/12 09:19:45  d3x0r
// Oops double added flattenline
//
// Revision 1.64  2004/06/12 09:15:37  d3x0r
// Added flatten line to common text operations.
//
// Revision 1.63  2004/06/07 10:58:16  d3x0r
// add flattenline
//
// Revision 1.62  2004/06/07 10:55:21  d3x0r
// add tab handling as sepearte type of space, extend linelength and buildline, and provide extended methods of calculating spacing
//
// Revision 1.61  2004/04/06 16:17:44  d3x0r
// Implement text comparison macros as functions for segfault protection
//
// Revision 1.60  2003/12/09 13:59:40  panther
// fix _avail to match new available expanded space
//
// Revision 1.59  2003/12/09 13:20:37  panther
// treat tab as space instead of ignoreing like return
//
// Revision 1.58  2003/12/09 13:13:25  panther
// remove noisy logging
//
// Revision 1.57  2003/12/09 13:11:44  panther
// Fix overflow somehow occuring in vartextadd
//
// Revision 1.56  2003/11/10 03:20:47  panther
// Fix VarTextCreateExEx def
//
// Revision 1.55  2003/11/03 15:51:18  panther
// Add some functionality to VarText, abstract data content
//
// Revision 1.54  2003/10/22 01:58:23  panther
// Fixed critical sections yay! disabled critical section logging
//
// Revision 1.53  2003/10/21 21:33:29  panther
// Changes to credit destroyer with destruction of psi objects
//
// Revision 1.52  2003/10/18 22:44:46  panther
// Fixed vtprintf watcom.  Misc logging removed
//
// Revision 1.51  2003/10/18 04:43:00  panther
// Quick patch...
//
// Revision 1.50  2003/08/10 14:21:25  panther
// fix expansion factor
//
// Revision 1.49  2003/08/10 14:15:43  panther
// reset args every pass through vtprintf for watcom
//
// Revision 1.48  2003/08/01 02:35:57  panther
// Attempt to fix vtprintf for watcom
//
// Revision 1.47  2003/06/03 08:09:44  panther
// Add cleanup to destroy
//
// Revision 1.46  2003/05/20 18:30:46  panther
// New functions - create/destroy vartext
//
// Revision 1.45  2003/04/20 08:14:07  panther
// *** empty log message ***
//
// Revision 1.44  2003/04/16 10:32:04  panther
// default burst attach +/- sign to trailing numbers
//
// Revision 1.43  2003/04/12 20:52:46  panther
// Added new type contrainer - data list.
//
// Revision 1.42  2003/04/08 07:01:22  panther
// Fix cleanup issue (popups) Added another text type EX FORMAT OP
// Fixed a dangling segsplit in text.c
//
// Revision 1.41  2003/04/06 23:23:52  panther
// Update vartext handling for new segsplit
//
// Revision 1.40  2003/04/06 12:29:15  panther
// Extend segsplit to delete the segment which it was splitting
//
// Revision 1.39  2003/04/02 06:45:37  panther
// Define flags for handling positioning in TEXT subsystem
//
// Revision 1.38  2003/03/26 07:23:46  panther
// Include buildline end of line option
//
// Revision 1.37  2003/03/23 13:57:11  panther
// Remove unused 'levels' variable to avoid over-definition
//
// Revision 1.36  2003/03/04 16:28:36  panther
// Cleanup warnings in typecode.  Convert PTRSZVAL to POINTER literal in binarylist
//
// Revision 1.35  2003/01/28 16:37:48  panther
// More logging extended logging
//
// Revision 1.34  2003/01/14 11:07:36  panther
// Fix pointer to int convert warnings, also don't return indirect if TF_APPLICATION
//
// Revision 1.33  2003/01/14 10:51:21  panther
// If no text to return on VarTextGet - return NULL - coincidence made it work before.
//
// Revision 1.32  2002/10/13 20:37:14  panther
// Various.
//
// Revision 1.31  2002/10/09 13:16:02  panther
// Support for linux shared memory mapping.
// Support for better linux compilation of configuration scripts...
// Timers library is now Threads AND Timers.
//
// Revision 1.30  2002/08/04 01:16:25  panther
// Typo in line release patch.
//
// Revision 1.29  2002/08/02 21:49:46  panther
// Shoot - this is the real fix (ignore prior revision!)
//
// Revision 1.28  2002/07/15 08:37:14  panther
// Result with TRUE from vartextend if there is something committed.
//
// Revision 1.27  2002/07/15 08:27:34  panther
// Removed superfluous logging (failed to get result, remaining buffer)
//
// Revision 1.26  2002/06/18 11:03:28  panther
// Fixed a memory loss in VARTEXT_Get
//
// Revision 1.25  2002/06/02 11:46:01  panther
// Modifications to build under MSVC
// Added MSVC project files.
//
// Revision 1.24  2002/05/04 05:13:15  panther
// Forgot to unwide some changes while vtprintfEx is not macroable with LCC
//
// Revision 1.23  2002/05/04 05:11:46  panther
// *** empty log message ***
//
// Revision 1.22  2002/05/03 14:58:13  panther
// Temporary disable of DBG_PASS on vtprintf until LCC can be fixed.
//
// Revision 1.21  2002/04/25 08:14:29  panther
// Forgot some DBG_FORWARD kinda stuff...
//
// Revision 1.20  2002/04/25 04:23:02  panther
// Fix vsnprintf for cygwin - to _vsnprintf
//
// Revision 1.19  2002/04/25 02:55:28  panther
// Modified VarText____ to accept DBG_PASS information for better allocate
// tracking/logging...
//
