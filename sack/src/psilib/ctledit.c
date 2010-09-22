#define _INCLUDE_CLIPBOARD
#include <stdhdrs.h>
#include <sharemem.h>
#include "controlstruc.h"
//#include <actimg.h> // alias to device output independance(?)
#include <logging.h>

#include <keybrd.h>
#include <controls.h>
#include <psi.h>

PSI_EDIT_NAMESPACE
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------

typedef struct edit {
	PSI_CONTROL pc;
	struct {
		int insert:1;
		_32 bPassword : 1; // hide data content.
		_32 bFocused : 1;
		_32 bInternalUpdate : 1;
		_32 bReadOnly : 1;
	}flags;
	TEXTCHAR *content; // our quick and dirty buffer...
	int nCaptionSize, nCaptionUsed;
	int top_side_pad;
	int Start; // first character in edit control
	int cursor_pos; // cursor position
	_32 MaxShowLen;
	int select_anchor, select_start, select_end;
} EDIT, *PEDIT;

#define LEFT_SIDE_PAD 2
//---------------------------------------------------------------------------

CTEXTSTR GetString( PEDIT pe, CTEXTSTR text, int length )
{
   static int lastlen;
	static TEXTCHAR *temp;
	if( lastlen < length )
	{
		if( temp )
			Release( temp );
		temp = NewArray( TEXTCHAR, length+1 );
		lastlen = length;
	}
	if( pe->flags.bPassword )
	{
		int n;
		for( n = 0; n < length; n++ )
			temp[n] = '*';
		temp[n] = 0;
      return temp;
	}

	return text;
}

//---------------------------------------------------------------------------

CUSTOM_CONTROL_DRAW( DrawEditControl, ( PSI_CONTROL pc ) )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	Font font;
	int ofs, x, CursX;
	_32 height;
	BlatColorAlpha( pc->Surface, 0, 0, pc->surface_rect.width, pc->surface_rect.height, basecolor(pc)[EDIT_BACKGROUND] );
	//ClearImageTo( pc->Surface, basecolor(pc)[EDIT_BACKGROUND] );
	font = GetFrameFont( pc );
	// should probably figure some way to center the text vertical also...
	pe->MaxShowLen = GetMaxStringLengthFont( pc->surface_rect.width, font );
	//lprintf( WIDE("Drawing an edit ronctorl... %s"), GetText( pc->caption.text ));
	if( pe->cursor_pos > (pe->MaxShowLen - 2) )
	{
		pe->Start = pe->cursor_pos - (pe->MaxShowLen - 2);
		CursX = GetStringSizeFontEx( GetText( pc->caption.text) + pe->Start, pe->MaxShowLen - 2, NULL, &height, font );
	}
	else
	{
		_32 tmp;
		pe->Start = 0;
		if( ( tmp = GetTextSize( pc->caption.text ) - pe->Start ) < pe->MaxShowLen )
		{
			//lprintf( WIDE("Shortening from maximum to actual we have.. %d  %d"), pe->MaxShowLen, tmp );
			pe->MaxShowLen = tmp;
		}
		CursX = GetStringSizeFontEx( GetText( pc->caption.text) + pe->Start, pe->cursor_pos, NULL, &height, font );
	}
	if( height <= pc->Surface->height )
		pe->top_side_pad = (pc->Surface->height - height) / 2;
	else
		pe->top_side_pad = 0;
	//lprintf( WIDE("------- CURSX = %d --- "), CursX );
	CursX += LEFT_SIDE_PAD;
	ofs = 0;
	x = LEFT_SIDE_PAD;
	// yuck - need to ... do this and that and the other...
	// if something is selected...
	if( pe->nCaptionUsed )
	{
		//lprintf( WIDE("Caption used... %d %d %d")
		//		 , pc->flags.bFocused
		//		 , pe->select_start
		//		 , pe->select_end);
		if( pc->flags.bFocused
			&& pe->select_start >= 0 && pe->select_end >= 0 )
		{
			// this is a working copy of this variable
			// that steps through the ranges of marked-ness
         _32 Start = pe->Start;
			do
			{
				int nLen;
				//lprintf( WIDE("%d %d %d"), Start, pe->select_start, pe->select_end );
				if( Start < pe->select_start )
				{
					if( ( pe->select_start - Start ) < pe->MaxShowLen )
						nLen = pe->select_start - Start;
					else
						nLen = pe->MaxShowLen - ofs;
					//lprintf( WIDE("Showing %d of string in normal color before select..."), nLen );
					PutStringFontEx( pc->Surface, x, pe->top_side_pad
											 , basecolor(pc)[EDIT_TEXT], basecolor(pc)[EDIT_BACKGROUND]
											 , GetString( pe, GetText( pc->caption.text) + Start, nLen ), nLen, font );
					x += GetStringSizeFontEx( GetString( pe, GetText( pc->caption.text) + Start, nLen ), nLen, NULL, NULL, font );
				}
				else if( Start > pe->select_end ) // beyond the end of the display
				{
					nLen = pe->MaxShowLen - ofs;
					//lprintf( WIDE("Showing %d of string in normal color after select..."), nLen );
					PutStringFontEx( pc->Surface, x, pe->top_side_pad
											 , basecolor(pc)[EDIT_TEXT], basecolor(pc)[EDIT_BACKGROUND]
											 , GetString( pe, GetText( pc->caption.text) + Start, nLen ), nLen, font );
					x += GetStringSizeFontEx( GetString( pe, GetText( pc->caption.text) + Start, nLen ), nLen, NULL, NULL, font );
				}
				else // start is IN select start to select end...
				{
					if( ( ofs + pe->select_end - Start ) < pe->MaxShowLen )
					{
						nLen = pe->select_end - Start + 1;
					}
					else
						nLen = pe->MaxShowLen - ofs;
					//lprintf( WIDE("Showing %d of string in selected color..."), nLen );
					PutStringFontEx( pc->Surface, x, pe->top_side_pad
											 , basecolor(pc)[SELECT_TEXT], basecolor(pc)[SELECT_BACK]
											 , GetString( pe, GetText( pc->caption.text) + Start, nLen ), nLen, font );
					x += GetStringSizeFontEx( GetString( pe, GetText( pc->caption.text) + Start, nLen ), nLen, NULL, NULL, font );
				}
				Start += nLen;
				ofs += nLen;
			} while( ofs < pe->MaxShowLen );
		}
		else
		{
			PutStringFontEx( pc->Surface, x, pe->top_side_pad
									 , basecolor(pc)[EDIT_TEXT], basecolor(pc)[EDIT_BACKGROUND]
									 , GetString( pe, GetText( pc->caption.text) + pe->Start, pe->MaxShowLen ), pe->MaxShowLen
									 , font
									 );
			x += GetStringSizeFontEx( GetString( pe, GetText( pc->caption.text) + pe->Start, pe->MaxShowLen ), pe->MaxShowLen, NULL, NULL, font );
		}
	}
	//else
	//   lprintf( WIDE("NO caption used.")) ;
	if( pc->flags.bFocused )
	{
		//lprintf( WIDE("Have focus in edit control - drawing cursor thingy at %d"), CursX );
		do_line( pc->Surface, CursX
				 , 1
				 , CursX
				 , pc->surface_rect.height
				 , Color( 255,255,255 ) );
		do_line( pc->Surface, CursX+1
				 , 1
				 , CursX+1
				 , pc->surface_rect.height
				 , Color( 0,0,0 ) );
		do_line( pc->Surface, CursX-1
				 , 1
				 , CursX-1
				 , pc->surface_rect.height
				 , Color( 0,0,0 ) );
	}
	//else
	//   lprintf( WIDE("have NO focus in edit control, not drawing anything...") );
	return TRUE;
}

//---------------------------------------------------------------------------

static int OnMouseCommon( EDIT_FIELD_NAME )( PSI_CONTROL pc, S_32 x, S_32 y, _32 b )
//CUSTOM_CONTROL_MOUSE(  MouseEditControl, ( PSI_CONTROL pc, S_32 x, S_32 y, _32 b ) )
{
	static int _b;
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	int cx, cy, found = 0, len = pc->caption.text ? GetTextSize( pc->caption.text ) : 0;
	_32 width, _width = 0, height;
	LOGICAL moving_left, moving_right;
	Font font = GetCommonFont( pc );
	//lprintf( WIDE("Edit mosue: %d %d %X"), x, y, b );
	GetStringSize( GetText(pc->caption.text), &width, &height );
	// how to find the line/character we're interested in....
	cy = (y - pe->top_side_pad) / height;
	if( x < LEFT_SIDE_PAD )
	{
      found = 1;
		cx = 0;
	}
	else
	{
		//cx = ( x - LEFT_SIDE_PAD ) / characters...
		// so given any font - variable size, we have to figure out which
      // character is on this line...
		for( cx = 1; cx <= ( len - pe->Start ); cx++ )
		{
			if( GetStringSizeFontEx( GetText( pc->caption.text) + pe->Start, cx, &width, NULL, font ) )
			{
				//lprintf( WIDE("is %*.*s(%d) more than %d?")
				//		 , cx,cx,GetText(pc->caption.text)
				//		 , width
				//		 , x );
				if( ( width + LEFT_SIDE_PAD ) > x )
				{
					// left 1/3 of the currnet character sets the cursor to the left
					// of the character, otherwise cursor is set on(after) the
					// current character.
					// OOP! - previously this test was backwards resulting in seemingly
               // very erratic cursor behavior..
					if( ((width+LEFT_SIDE_PAD)-x) > (width - _width)/3 )
						cx = cx-1;
					//lprintf( WIDE("Why yes, yes it is.") );
					found = 1;
					break;
				}
			}
			_width = width;
		}
	}
	// cx will be strlen + 1 if past end
   // cx is 0 at beginning.
	if( !found )
	{
		cx = len - pe->Start;
		// cx max...
      //lprintf( WIDE("Past end of string...") );
	}
	moving_left = 0;
	moving_right = 0;
	if( b & MK_LBUTTON )
		if( pe->cursor_pos != cx )
		{
			// this updates the current cursor position.
			// this works very well.... (now)
			if( pe->cursor_pos > cx )
				moving_left = 1;
			else
            moving_right = 1;
			pe->cursor_pos = cx;
			SmudgeCommon( pc );
		}
	{
		//lprintf( WIDE("current character: %d %d"), cx, cy );
	}
	if( b & MK_LBUTTON )
	{
      //cx -= 1;
		if( !( _b & MK_LBUTTON ) )
		{
			// we're not really moving, we just started...
         pe->select_start = pe->select_end = -1;
         moving_left = moving_right = 0;
			if( cx < len )
			{
				//lprintf( WIDE("Setting select start, end at %d,%d"), cx, cx );
            pe->select_anchor
					= cx;
			}
			else
			{
            //lprintf( WIDE("Setting select start, end at %d,%d"), len-1, len-1 );
				pe->select_anchor
					= len;
			}
			//lprintf( WIDE("--- Setting begin and end... hmm first button I guess Select...") );
			SmudgeCommon( pc );
		}
		else
		{
			//lprintf( WIDE("still have that mouse button down....") );
			if( moving_left || moving_right )
			{
				if( cx < pe->select_anchor )
				{
					pe->select_start = cx;
               pe->select_end = pe->select_anchor - 1;
				}
				else
				{
					pe->select_end = cx;
               pe->select_start = pe->select_anchor;
				}
            SmudgeCommon( pc );
			}
		}
	}
	_b = b;
   return 1;
}

//---------------------------------------------------------------------------

void CutEditText( PEDIT pe, PTEXT *caption )
{
	// any selected text is now deleted... the buffer is shortened...
    if( pe->select_start < pe->nCaptionUsed )
    {
        pe->nCaptionUsed -= (pe->select_end - pe->select_start) + 1;
        if( pe->nCaptionUsed && pe->nCaptionUsed != pe->select_start )
        {
            MemCpy( GetText( *caption ) + pe->select_start
                   , GetText( *caption ) + pe->select_end+1
						, pe->nCaptionUsed - pe->select_start );
            SetTextSize( *caption, pe->nCaptionUsed );
            GetText( *caption )[pe->nCaptionUsed] = 0;
        }
		  else
		  {
            SetTextSize( *caption, pe->select_start );
				GetText( *caption )[pe->select_start] = 0;
		  }
        pe->cursor_pos = pe->select_start;
	 }
    pe->select_start = -1;
    pe->select_end = -1;
}

static void InsertAChar( PEDIT pe, PTEXT *caption, TEXTCHAR ch )
{
	if( (pe->nCaptionUsed+1) >= pe->nCaptionSize )
	{
		PTEXT newtext;
		pe->nCaptionSize += 16;
		newtext = SegCreate( pe->nCaptionSize );
		StrCpyEx( GetText( newtext ), GetText( *caption )
				 , (pe->nCaptionUsed+1) ); // include the NULL, the buffer will be large enough.
		SetTextSize( newtext, pe->nCaptionUsed );
		GetText( newtext )[pe->nCaptionUsed] = 0;
		LineRelease( *caption );
		*caption = newtext;
	}
	{
		int n;
		pe->nCaptionUsed++;
		for( n = pe->nCaptionUsed; n > pe->cursor_pos; n-- )
		{
			GetText( *caption )[n] =
				GetText( *caption )[n-1];
		}
		GetText( *caption )[pe->cursor_pos] = ch;
		pe->cursor_pos++;
	}
	SetTextSize( *caption, pe->nCaptionUsed );
}

void TypeIntoEditControl( PSI_CONTROL pc, CTEXTSTR text )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	if( pc )
	{
		while( text[0] )
		{
			InsertAChar( pe, &pc->caption.text, text[0] );
			text++;
		}
	}
   SmudgeCommon( pc );
}

//---------------------------------------------------------------------------

#ifdef _WIN32
static void GetMarkedText( PEDIT pe, PTEXT *caption, TEXTCHAR *buffer, int nSize )
{
	if( pe->select_start >= 0 )
	{
		if( nSize > (pe->select_end - pe->select_start) + 1 )
		{
			nSize = (pe->select_end - pe->select_start) + 1;
		}
      else
			nSize--; // leave room for the nul.

		// otherwise nSize is maximal copy or correct amount to copy
		MemCpy( buffer
				, GetText( *caption ) + pe->select_start
				, nSize );
      buffer[nSize] = 0; // set nul terminator.
	}
	else
	{
		MemCpy( buffer
				, GetText( *caption ) + pe->select_start
				, nSize );
		buffer[nSize] = 0; // set nul terminator.
	}
}

//---------------------------------------------------------------------------

static void Paste( PEDIT pe, PTEXT *caption )
{
    if( OpenClipboard(NULL) )
    {
        _32 format;
        // successful open...
		  format = EnumClipboardFormats( 0 );
		  if( pe->select_start >= 0 )
			  CutEditText( pe, caption );
        while( format )
        {
            if( format == CF_TEXT )
            {
                HANDLE hData = GetClipboardData( CF_TEXT );
					 char *pData = (char*)GlobalLock( hData );
					 {
						 while( pData && pData[0] )
						 {
							 InsertAChar( pe, caption, pData[0] );
							 pData++;
						 }
					 }
                break;
            }
            format = EnumClipboardFormats( format );
        }
		  CloseClipboard();
    }
    else
    {
        //DECLTEXT( msg, WIDE("Clipboard was not available") );
        //EnqueLink( &pdp->ps->Command->Output, &msg );
    }
}

//---------------------------------------------------------------------------

static void Copy( PEDIT pe, PTEXT *caption )
{
	TEXTCHAR data[1024];
	GetMarkedText( pe, caption, data, sizeof( data ) );
#ifndef UNDER_CE
	if( data[0] && OpenClipboard(NULL) )
	{
		size_t nLen = strlen( data ) + 1;
		HGLOBAL mem = GlobalAlloc( 
#ifndef _ARM_
			GMEM_MOVEABLE
#else
				0
#endif
			, nLen );
		MemCpy( GlobalLock( mem ), data, nLen );
		GlobalUnlock( mem );
		EmptyClipboard();
		SetClipboardData( CF_TEXT, mem );
		CloseClipboard();
		GlobalFree( mem );
	}
#endif
}

//---------------------------------------------------------------------------

static void Cut( PEDIT pe, PTEXT *caption )
{
	Copy( pe, caption );
	if( pe->select_start >= 0 )
		CutEditText( pe, caption );
	else
		SetCommonText( (PCONTROL)pe, NULL );

}
#endif

CUSTOM_CONTROL_KEY( KeyEditControl, ( PSI_CONTROL pc, _32 key ) )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
   int used_key = 0;
	char ch;
	if( !pe || pe->flags.bReadOnly )
		return 0;
	if( KEY_CODE(key) == KEY_TAB )
		return 0;
	if( key & KEY_PRESSED )
	{
		if( key & KEY_CONTROL_DOWN )
		{
			switch( KEY_CODE( key ) )
			{
#ifdef _WIN32
			case KEY_C:
				Copy( pe, &pc->caption.text );
				SmudgeCommon( pc );
				break;
			case KEY_X:
				Cut( pe, &pc->caption.text );
				SmudgeCommon( pc );
				break;
			case KEY_V:
				Paste( pe, &pc->caption.text );
				SmudgeCommon( pc );
				break;
#endif
			}
		}
		else switch( KEY_CODE( key ) )
		{
		case KEY_LEFT:
			{
				int oldpos = pe->cursor_pos;
				if( pe->cursor_pos )
				{
					pe->cursor_pos--;
					if( ! ( key & KEY_SHIFT_DOWN ) )
					{
						pe->select_start = -1;
						pe->select_end = -1;
					}
					else
					{
						if( oldpos == pe->select_start )
							pe->select_start--;
						else if( pe->cursor_pos == pe->select_end )
							pe->select_end--;
						else
							pe->select_start =
								pe->select_end = pe->cursor_pos;
						if( pe->select_start > pe->select_end )
							pe->select_start = pe->select_end = -1;
					}
					SmudgeCommon( pc );
				}
            used_key = 1;
				break;
			}
		case KEY_RIGHT:
			{
				int oldpos = pe->cursor_pos;
				if( pe->cursor_pos < pe->nCaptionUsed )
				{
					pe->cursor_pos++;
					if( !(key & KEY_SHIFT_DOWN ) )
					{
						pe->select_start = -1;
						pe->select_end = -1;
					}
					else
					{
						if( oldpos == (pe->select_end+1) )
						{
							pe->select_end++;
							if( pe->select_start == -1 )
								pe->select_start = 0;
						}
						else if( oldpos == pe->select_start )
							pe->select_start++;
						else
							pe->select_start =
								pe->select_end = oldpos;
						if( pe->select_start > pe->select_end )
							pe->select_start = pe->select_end = -1;
					}
					SmudgeCommon( pc );
				}
            used_key = 1;
				break;
			}
		case KEY_END:
			if( key & KEY_SHIFT_DOWN )
			{
				if( pe->select_end == pe->cursor_pos )
				{
				}
				else if( pe->select_start == pe->cursor_pos )
				{
					pe->select_start = pe->select_end;
				}
				else
				{
					pe->select_start = pe->cursor_pos;
				}
				pe->select_end = pe->nCaptionUsed-1;
			}
			else
			{
				pe->select_start = -1;
				pe->select_end = -1;
			}
			pe->cursor_pos = pe->nCaptionUsed;
			SmudgeCommon( pc );
			used_key = 1;
			break;
		case KEY_HOME:
			if( key & KEY_SHIFT_DOWN )
			{
				if( pe->select_start == pe->cursor_pos )
				{
				}
				else if(  pe->select_end == pe->cursor_pos )
				{
					pe->select_end = pe->select_start;
				}
				else
				{
					pe->select_end = pe->cursor_pos-1;
				}
				pe->select_start = 0;
			}
			else
			{
				pe->select_start = -1;
				pe->select_end = -1;
			}
			pe->cursor_pos = 0;
			SmudgeCommon( pc );
			used_key = 1;
			break;
		case KEY_DELETE:
			if( pe->select_start >= 0 && pe->select_end >= 0 )
				CutEditText( pe, &pc->caption.text );
			else
			{
				pe->select_end =
					pe->select_start = pe->cursor_pos;
				CutEditText( pe, &pc->caption.text );
			}
			SmudgeCommon( pc );
			used_key = 1;
			break;
		case KEY_BACKSPACE:
			//Log( WIDE("Backspace?!") );
			if( pe->select_start >= 0 && pe->select_end >= 0 )
				CutEditText( pe, &pc->caption.text );
			else
			{
				if( pe->cursor_pos )
				{
					pe->select_end =
						pe->select_start = pe->cursor_pos-1;
					CutEditText( pe, &pc->caption.text );
				}
			}
			SmudgeCommon( pc );
			used_key = 1;
			break;
		case KEY_ESCAPE:
			InvokeDefault( (PCONTROL)pc, INV_CANCEL );
			used_key = 1;
			break;
		case KEY_ENTER:
			InvokeDefault( (PCONTROL)pc, INV_OKAY );
			used_key = 1;
			break;
		default:
			//Log2( WIDE("Got Key: %08x(%c)"), key, key & 0xFF );
			ch = GetKeyText( key );
			if( ch )
			{
				if( (unsigned char)ch == 0xFF )
					ch = 0;
				if( pe->select_start >= 0 && pe->select_end >= 0 )
					CutEditText( pe, &pc->caption.text );
				InsertAChar( pe, &pc->caption.text, ch );
				SmudgeCommon( pc );
				//printf( WIDE("Key: %d(%c)\n"), ch,ch );
				used_key = 1;
			}
			break;
		}
	}
	return used_key;
}

//---------------------------------------------------------------------------

PSI_CONTROL SetEditControlReadOnly( PSI_CONTROL pc, LOGICAL bEnable )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	if( pe )
	{
      //lprintf( WIDE("Setting readonly attribut of control to %d"), bEnable );
		pe->flags.bReadOnly = bEnable;
	}
   return pc;
}

//---------------------------------------------------------------------------

PSI_CONTROL SetEditControlPassword( PSI_CONTROL pc, LOGICAL bEnable )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	if( pe )
	{
      //lprintf( WIDE("Setting readonly attribut of control to %d"), bEnable );
		pe->flags.bPassword = bEnable;
	}
   return pc;
}

//---------------------------------------------------------------------------

void CPROC ChangeEditCaption( PSI_CONTROL pc )
{
   ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
    if( !pc->caption.text )
    {
		 pc->caption.text = SegCreate(1);
		SetTextSize( pc->caption.text, 0 );
	}
    if( GetTextSize( pc->caption.text ) )
		 pe->cursor_pos = GetTextSize( pc->caption.text );
	 else
       pe->cursor_pos = 0;

    pe->nCaptionSize = pe->cursor_pos+1;
    pe->nCaptionUsed = pe->nCaptionSize-1;

    pe->select_start = 0;
    pe->select_end = pe->nCaptionUsed-1;
    SmudgeCommon(pc);
}

//---------------------------------------------------------------------------

/*
void SetEditFont( PCONTROL pc, Font font )
{
   SetCommonFont( (PSI_CONTROL)pc, font );
}
*/

static int CPROC FocusChanged( PCONTROL pc, LOGICAL bFocused )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	if( pe )
	{
      //lprintf( WIDE("Setting active focus here!") );
		pe->flags.bFocused = bFocused;
		if( pe->flags.bFocused )
		{
         _32 len = pc->caption.text ? GetTextSize( pc->caption.text ) : 0;
			pe->select_start = 0;
			pe->select_end = len - 1;
			pe->select_anchor = 0;
         pe->cursor_pos = len;
		}
      SmudgeCommon( pc );
	}
	return TRUE;
}

//---------------------------------------------------------------------------
#undef MakeEditControl

	int CPROC InitEditControl( PSI_CONTROL pControl );
int CPROC ConfigEditControl( PSI_CONTROL pc )
{
	return InitEditControl(pc);
}
PSI_CONTROL CPROC MakeEditControl( PSI_CONTROL pFrame, int attr
									  , int x, int y, int w, int h
									  , PTRSZVAL nID, TEXTCHAR *caption )
{
	return VMakeCaptionedControl( pFrame, EDIT_FIELD
										 , x, y, w, h
										 , nID, caption );
}

void CPROC GrabFilename( PSI_CONTROL pc, CTEXTSTR name, S_32 x, S_32 y )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	SetControlText( pc, name );
}

int CPROC InitEditControl( PSI_CONTROL pc )
{
	ValidatedControlData( PEDIT, EDIT_FIELD, pe, pc );
	if( pe )
	{
      pe->pc = pc;
		AddCommonAcceptDroppedFiles( pc, GrabFilename );
      return TRUE;
	}
   return FALSE;
}

#include <psi.h>
CONTROL_REGISTRATION
edit_control = { EDIT_FIELD_NAME
					, { {73, 21}, sizeof( EDIT ), BORDER_INVERT_THIN|BORDER_NOCAPTION }
					, InitEditControl
					, NULL
					, DrawEditControl
					, NULL //MouseEditControl
					, KeyEditControl
					, NULL
					, NULL, NULL
					, NULL
					, NULL
					, ChangeEditCaption
               , FocusChanged
};

PRIORITY_PRELOAD( RegisterEdit, PSI_PRELOAD_PRIORITY )
{
   DoRegisterControl( &edit_control );
}

PSI_EDIT_NAMESPACE_END


