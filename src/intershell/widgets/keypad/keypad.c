/*
 * Crafted by: Jim Buckeyne
 *
 * Pretty buttons, based on bag.psi image class buttons
 * and extended to the point that all that really remains is
 * the click handler... easy enough to steal, but it does
 * nice things like changes behavior based on touch screen
 * presense, and particular operational modes...
 *
 * Keypads - with numeric accumulator displays, based on
 *   shared memory regions such that many displays might
 *   accumulate into the same memory, with different
 *   visual aspects, and might get updated in return
 *   (well that's the theory) mostly it's a container
 *   for coordinating key board like events, a full alpha-
 *   numeric keyboard option exists, a 10 key entry pad (12)
 *   and soon a 16 key extended input with clear, cancel
 *   double 00, and decimal, and backspace  and(?)
 *
 * Some common external images may be defined in a theme.ini
 *  applications which don't otherwise specify graphics effects
 *  for the buttons enables them to conform to a default theme
 *  or one of multiple selections...
 *
 * (c)Fortunet 2006  (copy header include/widgets/buttons.h)
 */


#define DEFINE_DEFAULT_RENDER_INTERFACE
#define DEFINE_DEFAULT_IMAGE_INTERFACE
// this is ugly , but it works, please consider
// a library init that will grab this...
#ifndef __cplusplus_cli
#define USE_IMAGE_INTERFACE GetImageInterface()
#endif

#include <controls.h>
#include <sharemem.h>
#include <idle.h>
#include <keybrd.h>
#include <fractions.h>
#include <psi.h>
#include <timers.h>

#define KEYPAD_STRUCTURE_DEFINED
typedef struct keypad_struct *PKEYPAD;
#include "accum.h"
#include "../include/keypad.h"
#include "../include/buttons.h"

typedef struct key_space_size_tag {
   int rows, cols;
	FRACTION DisplayOffsetX;
	FRACTION DisplayX;
	FRACTION DisplayOffsetY;
	FRACTION DisplayY;

	FRACTION KeySpacingX; // 1/19th of the display...
	FRACTION KeySizingX; // 5/19ths of the display...
	FRACTION KeySpacingY; // 1/25th of the display...
	FRACTION KeySizingY; // 5/25ths of the display...
	FRACTION KeyDisplaySpacingY; // 1/25th of the display...
	FRACTION KeyDisplaySizingY; // 5/25ths of the display...
} KEY_SPACE_SIZE, *PKEY_SPACE_SIZE;

static KEY_SPACE_SIZE keypad_sizing = { 4, 3
												  , { 1, 19 }
												  , { 17, 19 }
												  , { 1, 28 }
												  , { 3, 28 }
												  , { 1, 55 } // 1/19th of the display...
												  , { 17, 55 } // 5/19ths of the display...
												  , { 1, 55 } // 1/25th of the display...
												  , { 25, 110 } // 5/25ths of the display...
												  , { 28, 1540 } // 1/25th of the display...
												  , { 298, 1540 } }; // 5/25ths of the display...

static KEY_SPACE_SIZE keyboard_sizing = { 4, 10
													 , { 1, 19 }
													 , { 17, 19 }
													 , { 1, 28 }
													 , { 2, 28 }
													 , { 1, 101 } // 1/19th of the display...
													 , { 9, 101 } // 5/19ths of the display...
													 , { 1, 55 } // 1/25th of the display...
													 , { 25, 110 } // 5/25ths of the display...
													 , { 28, 1540 } // 1/25th of the display...
													 , { 308, 1540 } }; // 5/25ths of the display...

// the amount of space between keys...
//static FRACTION DisplayOffsetX = { 1, 19 };
//static FRACTION DisplayX = { 17, 19 };
//static FRACTION DisplayOffsetY = { 1, 28 };
//static FRACTION DisplayY = { 2, 28 };

//static FRACTION KeySpacingX = { 1, 55 }; // 1/19th of the display...
//static FRACTION KeySizingX  = { 17, 55 }; // 5/19ths of the display...
//static FRACTION KeySpacingY = { 1, 55 }; // 1/25th of the display...
//static FRACTION KeySizingY  = { 10, 55 }; // 5/25ths of the display...
//static FRACTION KeyDisplaySpacingY = { 28, 1540 }; // 1/25th of the display...
//static FRACTION KeyDisplaySizingY  = { 308, 1540 }; // 5/25ths of the display...

//static FRACTION KeyBoardSpacingX = { 1, 101 }; // 1/19th of the display...
//static FRACTION KeyBoardSizingX  = { 9, 101 }; // 5/19ths of the display...
//static FRACTION KeyBoardSpacingY = { 1, 55 }; // 1/25th of the display...
//static FRACTION KeyBoardSizingY  = { 10, 55 }; // 5/25ths of the display...
//static FRACTION KeyBoardDisplaySpacingY = { 28, 1540 }; // 1/25th of the display...
//static FRACTION KeyBoardDisplaySizingY  = { 308, 1540 }; // 5/25ths of the display...
// total of fracitions is (col * 4 * spacingx ) + (col * 3 * sizingx) = 1
// total of fracitions is (row * 5 * spacingy ) + (row * 4 * sizingy) = 1

typedef struct display_struct
{
	PCONTROL control;
	Font font;
	_32 width, height;
   struct keypad_struct *keypad;
} DISPLAY, *PDISPLAY;

typedef struct keypad_struct
{
   PCOMMON frame;
	PACCUMULATOR accum;
   PUSER_INPUT_BUFFER pciEntry; // alpha keypads need this sort of buffer...
   //PTEXT entry; 
	Font font;
	struct {
		_32 bDisplay : 1;
		_32 bPassword : 1;
		_32 bEntry : 1;
		_32 bResult : 1;
		_32 bResultStatus : 1;
		_32 bHidden : 1;
		_32 bAlphaNum : 1;
		_32 bClearOnNext : 1;
	} flags;
   _32 displaywidth, displayheight;
	// last known size... when draw is triggered
	// see if we need to rescale the buttoms...
	PKEY_SPACE_SIZE key_spacing;
   _32 nKeys;
	PKEY_BUTTON *keys;
	_32 width, height;
	void (CPROC *keypad_enter_event)(PTRSZVAL psv, PSI_CONTROL keypad );
   PTRSZVAL psvEnterEvent;

	PCOMMON display;
	DeclareLink( struct keypad_struct );
} KEYPAD;

static PKEYPAD keypads;
// hmm be handy to get a frame resize message...

// when buttons are pressed...


static char *keytext[]= { "A7\0", WIDE("A8\0"), WIDE("A9\0")
								, WIDE("A4\0"), WIDE("A5\0"), WIDE("A6\0")
								, WIDE("A1\0"), WIDE("A2\0"), WIDE("A3\0")
								, WIDE("A<-\0"), WIDE("A0\0"), WIDE("A00\0")  };
static char *entrytext[]= {  "A7\0", WIDE("A8\0"), WIDE("A9\0")
								  ,  "A4\0", WIDE("A5\0"), WIDE("A6\0")
								  ,  "A1\0", WIDE("A2\0"), WIDE("A3\0")
								  ,  "~BRedAN\0", WIDE("A0\0"), WIDE("~BgreenAY\0") };
static char *entrytext2[]= {  "A7\0", WIDE("A8\0"), WIDE("A9\0")
									,  "A4\0", WIDE("A5\0"), WIDE("A6\0")
									,  "A1\0", WIDE("A2\0"), WIDE("A3\0")
									,  "AC\0", WIDE("A0\0"), WIDE("~BgreenA*\0") };

static char *entrytext3[]= {  "A7\0", WIDE("A8\0"), WIDE("A9\0")
									,  "A4\0", WIDE("A5\0"), WIDE("A6\0")
									,  "A1\0", WIDE("A2\0"), WIDE("A3\0")
									,  "AC\0", WIDE("A0\0"), WIDE("~BgreenAE\0") };
static char * keyval[] = {  "7", WIDE("8"), WIDE("9")
							  ,  "4", WIDE("5"), WIDE("6")
							  ,  "1", WIDE("2"), WIDE("3")
							  ,  "-1", WIDE("0"), WIDE("-2") };

static char *keyboardtext[]= {  WIDE("A1\0"), WIDE("A2\0"), WIDE("A3\0"), WIDE("A4\0"), WIDE("A5\0"), WIDE("A6\0"), WIDE("A7\0"), WIDE("A8\0"), WIDE("A9\0"), WIDE("A0\0")
									  ,  WIDE("AQ\0"), WIDE("AW\0"), WIDE("AE\0"), WIDE("AR\0"), WIDE("AT\0"), WIDE("AY\0"), WIDE("AU\0"), WIDE("AI\0"), WIDE("AO\0"), WIDE("AP\0")
									  ,  WIDE("AA\0"), WIDE("AS\0"), WIDE("AD\0"), WIDE("AF\0"), WIDE("AG\0"), WIDE("AH\0"), WIDE("AJ\0"), WIDE("AK\0"), WIDE("AL\0"), WIDE("A<-\0")
									  ,  WIDE("A^^^\0") , WIDE("AZ\0"), WIDE("AX\0"), WIDE("AC\0"), WIDE("AV\0"), WIDE("AB\0"), WIDE("AN\0"), WIDE("AM\0"), WIDE("AEsc\0"), WIDE("ABS\0")
};
static CTEXTSTR keyboardval[] = {  WIDE("1"), WIDE("2"), WIDE("3"), WIDE("4"), WIDE("5"), WIDE("6"), WIDE("7"), WIDE("8"), WIDE("9"), WIDE("0")
									  ,  WIDE("Q"), WIDE("W"), WIDE("E"), WIDE("R"), WIDE("T"), WIDE("Y"), WIDE("U"), WIDE("I"), WIDE("O"), WIDE("P")
									  ,  WIDE("A"), WIDE("S"), WIDE("D"), WIDE("F"), WIDE("G"), WIDE("H"), WIDE("J"), WIDE("K"), WIDE("L"), (CTEXTSTR)-1
									  ,  (char*)1, WIDE("Z"), WIDE("X"), WIDE("C"), WIDE("V"), WIDE("B"), WIDE("N"), WIDE("M"), (CTEXTSTR)-2, WIDE("\b")
};



//CONTROL_REGISTRATION keypad_display, keypad_control;
int CPROC InitKeypad( PSI_CONTROL pc );
static int CPROC KeypadDraw( PCOMMON frame );
int CPROC InitKeypadDisplay( PCOMMON pc );
static int CPROC DrawKeypadDisplay( PCONTROL pc );


CONTROL_REGISTRATION keypad_control = { "Keypad Control"
												  , { { 240, 320 }, sizeof( KEYPAD ), BORDER_NONE }
												  , InitKeypad
												  , NULL
                                      , KeypadDraw
};
CONTROL_REGISTRATION keypad_display = { "Keypad Display"
												  , { { 180, 20 }, sizeof( DISPLAY ), BORDER_INVERT|BORDER_THIN }
												  , InitKeypadDisplay
												  , NULL
												  , DrawKeypadDisplay
};
void SetDisplayPadKeypad( PSI_CONTROL pc, PKEYPAD keypad );  //added this because this function is called in at least one place before it is defined gcc compiler version 3.4.4 doesn't like it but it compiles fine on 3.3.x.

static void InvokeEnterEvent( PCOMMON pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		if( keypad->keypad_enter_event )
		{
         keypad->keypad_enter_event( keypad->psvEnterEvent, pc );
		}
	}
}

void SetKeypadEnterEvent( PCOMMON pc, void (CPROC *event)(PTRSZVAL,PSI_CONTROL), PTRSZVAL psv )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		keypad->keypad_enter_event = event;
      keypad->psvEnterEvent = psv;
	}
}

int CPROC InitKeypadDisplay( PCOMMON pc )
{
	ValidatedControlData( PDISPLAY, keypad_display.TypeID, display, pc );
	if( display )
	{
      Image surface = GetControlSurface( pc );
		display->width = surface->width;
		display->height = surface->height;
		display->font = RenderFontFile( WIDE("./Crysta.ttf")
												, (display->height - 5) * 8 / 8
												, display->height - 5, 1 );
      if( !display->font )
			display->font = RenderFontFile( WIDE("./fonts/Crysta.ttf")
													, (display->height - 5) * 8 / 8
													, display->height - 5, 1 );
		return 1;
	}
   return 0;
}



static int CPROC DrawKeypadDisplay( PCONTROL pc )
{
		char text[128];
		_32 width, height;
		ValidatedControlData( PDISPLAY, keypad_display.TypeID, display, pc );
      PKEYPAD pKeyPad = display->keypad;
		Image surface = GetControlSurface( pc );
		ClearImageTo( surface, Color( 129, 129, 149 ) );
		if( GetAccumulatorText( pKeyPad->accum, text, sizeof( text ) ) )
		{
			if( pKeyPad->flags.bPassword )
			{
				char *from, *to;
				for( from = to = text; from[0]; from++ )
				{
					if( from[0] == '$' ) continue;
					if( from[0] == '.' ) continue;
					if( to == text && from[0] == '0' ) continue;
					to[0] = '#';
					to++;
				}
				to[0] = 0;
			}
			GetStringSizeFont( text, &width, &height, display->font );
			PutStringFont( surface
							 , display->width - ( width + 3 )
							 , ( display->height - height ) / 2
							 , Color( 0, 0, 0 ), 0
							 , text
							 , display->font );
		}
      return 1;
}

static void KeypadAccumUpdated( PTRSZVAL psvKeypad, PACCUMULATOR accum )
{
	PKEYPAD keypad = (PKEYPAD)psvKeypad;
   if( keypad->display )
		SmudgeCommon( (PCOMMON)keypad->display );
	//if( keypad->flags.bDisplay )
	//	DrawDisplay( psvKeypad, keypad->display.control );
}

static int resize_keys( PKEYPAD keypad )
{
   unsigned int w, h;
	int row, col, rows, cols;
	FRACTION posy, posx, tmp;
	S_32 keyx, keyy, keywidth, keyheight;
	// we might get called here to resize
	// when we don't really have any children at all... and no spacing factor to
   // apply.
	if( keypad && keypad->key_spacing )
	{
		PKEY_SPACE_SIZE pKeySizing = keypad->key_spacing;
		w = keypad->width;
		h = keypad->height;
		rows = pKeySizing->rows;
		cols = pKeySizing->cols;
		keywidth = ReduceFraction( ScaleFraction( &tmp, w, &pKeySizing->KeySizingX ) );
		if( keypad->flags.bDisplay )
		{
			keyheight = ReduceFraction( ScaleFraction( &tmp, h, &pKeySizing->KeyDisplaySizingY ) );
			posy = pKeySizing->DisplayOffsetY;
			AddFractions( &posy, &pKeySizing->DisplayY );
			AddFractions( &posy, &pKeySizing->KeyDisplaySpacingY );
		}
		else
		{
			keyheight = ReduceFraction( ScaleFraction( &tmp, h, &pKeySizing->KeySizingY ) );
			posy = pKeySizing->KeySpacingY;
		}
		if( keypad->flags.bDisplay )
		{
			int displayx, displayy, displaywidth, displayheight;
			displayx = ReduceFraction( ScaleFraction( &tmp, keypad->width, &pKeySizing->DisplayOffsetX ) );
			displayy = ReduceFraction( ScaleFraction( &tmp, keypad->height, &pKeySizing->DisplayOffsetY ) );
			displaywidth = ReduceFraction( ScaleFraction( &tmp, keypad->width, &pKeySizing->DisplayX ) );
			displayheight = ReduceFraction( ScaleFraction( &tmp, keypad->height, &pKeySizing->DisplayY ) );
			MoveSizeCommon( keypad->display
							  , displayx, displayy
							  , displaywidth, displayheight
							  );
		}
		for( row = 0; row < rows; row++ )
		{
			keyy = ReduceFraction( ScaleFraction( &tmp, keypad->height, &posy ) );
			posx = pKeySizing->KeySpacingX;
			for( col = 0; col < cols; col++ )
			{
				keyx = ReduceFraction( ScaleFraction( &tmp, keypad->width, &posx ) );
				{
					//char tmp[32];
					//sLogFraction( tmp, &posx );
					//Log3( WIDE("Position x: %s - %d %d"), tmp, keyx, keypad->width );
					//sLogFraction( tmp, &posy );
					//Log3( WIDE("Position y: %s - %d %d"), tmp, keyy, keypad->height );
				}
				MoveSizeCommon( GetKeyCommon( keypad->keys[row * cols + col] )
								  , keyx, keyy
								  , keywidth, keyheight );
				AddFractions( AddFractions( &posx, &pKeySizing->KeySizingX )
								, &pKeySizing->KeySpacingX );
				;
				{
					//char tmp[32];
					//sLogFraction( tmp, &posx );
					//Log3( WIDE("NEW Position x: %s - %d %d"), tmp, keyx, keypad->width );
				}
			}
			if( keypad->flags.bDisplay )
			{
				AddFractions( AddFractions( &posy, &pKeySizing->KeyDisplaySizingY )
								, &pKeySizing->KeyDisplaySpacingY );
			}
			else
			{
				AddFractions( AddFractions( &posy, &pKeySizing->KeySizingY )
								, &pKeySizing->KeySpacingY );
			}
		}
	}
//cpg27dec2006 keypad\keypad.c(354): Warning! W107: Missing return value for function 'resize_keys'

   return 1;//cpg27dec2006
}


static int CPROC KeypadDraw( PCOMMON frame )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, frame );
   lprintf( WIDE("attempt Drawing blue keypad backgorund...") );
	if( keypad )
	{
		Image surface = GetControlSurface( frame );
		if( surface->width != keypad->width ||
			surface->height != keypad->height )
		{
			keypad->width = surface->width;
			keypad->height = surface->height;
			// need to resize the buttons hereupon.
			resize_keys( keypad );
         // and the display
		}
		// there is no border - so if we have a border this
		// needs to be drawn ourselves....
		lprintf( WIDE("Drawing blue keypad backgorund...") );
      BlatColorAlpha( surface, 0, 0, surface->width, surface->height, AColor( 0, 0, 64, 150 ) );
		//ClearImageTo( surface, Color( 0, 0, 64 ) );
		//UpdateControl( frame );
		return TRUE;
	}
   return FALSE;
}

static void CPROC KeyboardHandler( PTRSZVAL psv
											, _32 key );
static void CPROC KeyPressed( PTRSZVAL psv, PKEY_BUTTON key );
static int new_flags;
static int _InitKeypad( PSI_CONTROL frame )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, frame );
	PKEY_SPACE_SIZE pKeySizing;
	S_32 keyx, keyy, keywidth, keyheight;
	int row, col, rows, cols;
	FRACTION posy, posx, tmp;//, curposy;
   int w, h;
   //flags = new_flags; // remaining data is passed normally...
   keypad->frame = frame;
   // this is the earliest that logging can take place (above here exist declarations...)
   //lprintf( WIDE("Making a keypad at %d,%d %d,%d"), x, w, w, h );
	LinkThing( keypads, keypad );
	{
		PRENDERER render = GetFrameRenderer( GetFrame( frame ) );
      BindEventToKey( render, KEY_PAD_0, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_1, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_2, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_3, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_4, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_5, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_6, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_7, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_8, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_9, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_INSERT, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_DELETE, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_DELETE, KEY_MOD_EXTENDED|KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_END, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_DOWN, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PGDN, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_LEFT, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_CENTER, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_RIGHT, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_HOME, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_UP, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PGUP, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_DELETE, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_ENTER, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_MINUS, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_DELETE, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
      BindEventToKey( render, KEY_PAD_ENTER, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_PAD_MINUS, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_0, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_1, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_2, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_3, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_4, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_5, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_6, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_7, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_8, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		BindEventToKey( render, KEY_9, KEY_MOD_ALL_CHANGES, KeyboardHandler, (PTRSZVAL)frame );
		if( new_flags & KEYPAD_FLAG_ALPHANUM )
		{
			BindEventToKey( render, KEY_0, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_1, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_2, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_3, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_4, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_5, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_6, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_7, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_8, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_9, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_A, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_B, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_C, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_D, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_E, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_F, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_G, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_H, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_I, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_J, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_K, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_L, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_M, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_N, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_O, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_P, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_Q, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_R, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_S, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_T, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_U, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_V, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_W, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_X, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_Y, 0, KeyboardHandler, (PTRSZVAL)frame );
			BindEventToKey( render, KEY_Z, 0, KeyboardHandler, (PTRSZVAL)frame );
         pKeySizing = &keyboard_sizing;
		}
		else
		{
         pKeySizing = &keypad_sizing;
		}
	}
	// there will be one and only one keypad...
	{
      Image surface = GetControlSurface( frame );
		w = keypad->width = surface->width;
		h = keypad->height = surface->height;
	}
	keypad->flags.bDisplay = new_flags & KEYPAD_FLAG_DISPLAY;
	keypad->flags.bPassword = (new_flags & KEYPAD_FLAG_PASSWORD)!=0;
   keypad->flags.bEntry = (new_flags & KEYPAD_FLAG_ENTRY) != 0;
	keypad->flags.bAlphaNum = (new_flags & KEYPAD_FLAG_ALPHANUM) != 0;
   keypad->key_spacing = pKeySizing;
	keypad->accum = GetAccumulator( WIDE("Keypad Entry")
											, keypad->flags.bAlphaNum?ACCUM_TEXT
													 : (keypad->flags.bEntry?0:ACCUM_DOLLARS) );
	SetAccumulatorUpdateProc( keypad->accum, KeypadAccumUpdated, (PTRSZVAL)keypad );
	AddCommonDraw( frame, KeypadDraw );

	keywidth = ReduceFraction( ScaleFraction( &tmp
														 , keypad->width
														 , &pKeySizing->KeySizingX ) );

	rows = pKeySizing->rows;
	cols = pKeySizing->cols;
	//rows = 4;
	//cols = 3;
	if( new_flags & KEYPAD_FLAG_DISPLAY )
	{
		keyheight = ReduceFraction( ScaleFraction( &tmp
															  , keypad->height
															  , &pKeySizing->KeyDisplaySizingY ) );
		posy = pKeySizing->DisplayOffsetY;
		AddFractions( &posy, &pKeySizing->DisplayY );
		AddFractions( &posy, &pKeySizing->KeyDisplaySpacingY );
	}
	else
	{
		keyheight = ReduceFraction( ScaleFraction( &tmp, h, &pKeySizing->KeySizingY ) );
		posy = pKeySizing->KeySpacingY;
	}
	Log2( WIDE("Rendering Font sized: %ld, %ld"), keywidth / 2, keyheight / 2 );
	keypad->font = RenderFontFile( WIDE("./arialbd.ttf")
										  , keywidth - 22, keyheight - 10, 3 );
   if( !keypad->font )
		keypad->font = RenderFontFile( WIDE("./fonts/arialbd.ttf")
											  , keywidth - 22, keyheight - 10, 3 );

	{
		int displayx, displayy, displaywidth, displayheight;
      displayx = ReduceFraction( ScaleFraction( &tmp, w, &pKeySizing->DisplayOffsetX ) );
      displayy = ReduceFraction( ScaleFraction( &tmp, h, &pKeySizing->DisplayOffsetY ) );
      displaywidth = ReduceFraction( ScaleFraction( &tmp, w, &pKeySizing->DisplayX ) );
		displayheight = ReduceFraction( ScaleFraction( &tmp, h, &pKeySizing->DisplayY ) );
		if( new_flags & KEYPAD_FLAG_DISPLAY )
		{
			keypad->display = MakeControl( frame
												  , keypad_display.TypeID
												  , displayx, displayy
												  , displaywidth, displayheight
												  , 1000
												  );
         SetDisplayPadKeypad( keypad->display, keypad );
		}
	}
    keypad->keys = NewArray( PKEY_BUTTON,( keypad->nKeys = rows * cols ) );
	for( row = 0; row < rows; row++ )
	{
		keyy = ReduceFraction( ScaleFraction( &tmp, h, &posy ) );
		posx = pKeySizing->KeySpacingX;
		for( col = 0; col < cols; col++ )
		{
			keyx = ReduceFraction( ScaleFraction( &tmp, w, &posx ) );
			{
				//char tmp[32];
            //sLogFraction( tmp, &posx );
				//Log3( WIDE("Position x: %s - %d %d"), tmp, keyx, w );
            //sLogFraction( tmp, &posy );
				//Log3( WIDE("Position y: %s - %d %d"), tmp, keyy, h );
			}
			keypad->keys[row * cols + col] = MakeKeyEx( frame
																 , keyx, keyy
																 , keywidth, keyheight
																 , 0
																 , NULL //glare
																 , NULL // up
																	, NULL //down
                                                    , NULL //mask
																 , KEY_BACKGROUND_COLOR
																 , Color( 220, 220, 12 )
																  // if flags is DISPLAY only (not password)
																 , (keypad->flags.bAlphaNum)?keyboardtext[row * cols + col]
																 :(keypad->flags.bPassword)?entrytext[row * cols + col]
																  :(keypad->flags.bEntry)?entrytext3[row * cols + col]
																	 :keytext[row * cols + col]
																 , keypad->font
																 , KeyPressed, (PTRSZVAL)frame
																 , keypad->flags.bAlphaNum
																  ? keyboardval[row*cols + col]
																  : keyval[row * cols + col]
													  );
			AddFractions( AddFractions( &posx, &pKeySizing->KeySizingX )
							, &pKeySizing->KeySpacingX );
			;
			{
				//char tmp[32];
            //sLogFraction( tmp, &posx );
				//Log3( WIDE("NEW Position x: %s - %d %d"), tmp, keyx, w );
			}
		}
		if( new_flags & KEYPAD_FLAG_DISPLAY )
		{
			AddFractions( AddFractions( &posy, &pKeySizing->KeyDisplaySizingY )
						  , &pKeySizing->KeyDisplaySpacingY );
		}
		else
		{
			AddFractions( AddFractions( &posy, &pKeySizing->KeySizingY )
						  , &pKeySizing->KeySpacingY );
		}
	}
   return TRUE;
}

int CPROC InitKeypad( PSI_CONTROL pc )
{
   return _InitKeypad( pc );
}

PRELOAD( KeypadDisplayRegister)
{
   DoRegisterControl( &keypad_display );
	DoRegisterControl( &keypad_control );
}

#if 0
#define BUFFER_SIZE 80

struct {
	char *buffer;
	int used_chars;

}

void EnterKeyIntoBuffer( char **ppBuffer, char c )
{
	if( !ppBuffer )
		return;
	if( !(*ppBuffer ) )
	{
      (*ppBuffer) = Allocate( BUFFER_SIZE );
	}
}
#endif
static void CPROC KeyPressed( PTRSZVAL psv, PKEY_BUTTON key )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, pKeyPad, (PSI_CONTROL)psv );
	//PKEYPAD pKeyPad = (PKEYPAD)psv;
	char *string;
   S_32 value;
//cpg27dec2006 keypad\keypad.c(657): Warning! W1178: Type qualifier mismatch
//cpg27dec2006 keypad\keypad.c(657): Note! N2003: source conversion type is 'char const *'
//cpg27dec2006 keypad\keypad.c(657): Note! N2004: target conversion type is 'char *'
	//cpg27dec2006 	string = GetKeyValue( key );
   string = (char *)GetKeyValue( key );
	if( pKeyPad->flags.bAlphaNum )
	{
		if( string == (char*)1 )
		{
         // shift state..
		}
		else if( string == (char*)-1 )
		{
         // wake waiting thread...
         pKeyPad->flags.bResult = 1;
			pKeyPad->flags.bResultStatus = 1;
		}
		else if( string == (char*)-2 )
		{
         // wake waiting thread...
         pKeyPad->flags.bResult = 1;
			pKeyPad->flags.bResultStatus = 0;
		}
		else
		{
			KeyTextIntoAccumulator( pKeyPad->accum, string );
		}
	}
	else
	{
		value = atol( string );
		if( pKeyPad->flags.bPassword || !pKeyPad->flags.bDisplay || pKeyPad->flags.bEntry )
		{
			if( value == -1 )
			{
				if( pKeyPad->flags.bEntry )
				{
					if( pKeyPad->flags.bPassword )
						ClearAccumulatorDigit( pKeyPad->accum, 10 );
					else
						ClearAccumulator( pKeyPad->accum );
				}
				else
				{
					pKeyPad->flags.bResult = 1;
					pKeyPad->flags.bResultStatus = 0;
				}
			}
			else if( value == -2 )
			{
				InvokeEnterEvent( (PSI_CONTROL)psv );
				pKeyPad->flags.bResult = 1;
				pKeyPad->flags.bResultStatus = 1;
			}
			else
			{
				if( pKeyPad->flags.bClearOnNext )
				{
               pKeyPad->flags.bClearOnNext = 0;
					pKeyPad->flags.bResult = 0;
					pKeyPad->flags.bResultStatus = 0;
					ClearAccumulator( pKeyPad->accum );
				}
				KeyIntoAccumulator( pKeyPad->accum, value, 10 );
			}
		}
		else
		{
			if( value == -1 )
			{
				if( pKeyPad->flags.bClearOnNext )
				{
               pKeyPad->flags.bClearOnNext = 0;
					pKeyPad->flags.bResult = 0;
					pKeyPad->flags.bResultStatus = 0;
					ClearAccumulator( pKeyPad->accum );
				}
				ClearAccumulatorDigit( pKeyPad->accum, 10 );
			}
			else if( value == -2 )
			{
				if( pKeyPad->flags.bClearOnNext )
				{
               pKeyPad->flags.bClearOnNext = 0;
					pKeyPad->flags.bResult = 0;
					pKeyPad->flags.bResultStatus = 0;
					ClearAccumulator( pKeyPad->accum );
				}

				KeyIntoAccumulator( pKeyPad->accum, 0, 10 );
				KeyIntoAccumulator( pKeyPad->accum, 0, 10 );
			}
			else
			{
				if( pKeyPad->flags.bClearOnNext )
				{
               pKeyPad->flags.bClearOnNext = 0;
					pKeyPad->flags.bResult = 0;
					pKeyPad->flags.bResultStatus = 0;
					ClearAccumulator( pKeyPad->accum );
				}
				KeyIntoAccumulator( pKeyPad->accum, value, 10 );
			}
		}
	}
}

KEYPAD_PROC( void, KeyIntoKeypad )( PSI_CONTROL pc, _64 value )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, pKeyPad, pc );
	if( pKeyPad )
	{
      ClearAccumulator( pKeyPad->accum );
		KeyIntoAccumulator( pKeyPad->accum, value, 10 );
		InvokeEnterEvent( pc );
	}
}

KEYPAD_PROC( void, KeypadInvertValue )( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, pKeyPad, pc );
	if( pKeyPad )
	{
		SetAccumulator( pKeyPad->accum, -GetAccumulatorValue( pKeyPad->accum ) );
	}
}


static void CPROC KeyboardHandler( PTRSZVAL psv
											, _32 key )
{
	PKEY_BUTTON button;
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, pKeyPad, (PSI_CONTROL)psv );
	{
		pKeyPad = keypads;
		while( pKeyPad )
		{
			if( !pKeyPad->flags.bHidden )
				break;
			pKeyPad = NextLink( pKeyPad );
		}
	}
	if( IsKeyPressed( key ) )
	{
      if( pKeyPad ) switch( KEY_CODE(key) )
		{
		case KEY_0:
		case KEY_PAD_0:
		case KEY_INSERT:
			KeyPressed( psv, button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 1] );
			break;
		case KEY_1:
		case KEY_END:
		case KEY_PAD_1:
         KeyPressed( psv, button = pKeyPad->keys[2 * pKeyPad->key_spacing->cols + 0] );
			break;
		case KEY_2:
		case KEY_DOWN:
		case KEY_PAD_2:
         KeyPressed( psv, button = pKeyPad->keys[2 * pKeyPad->key_spacing->cols + 1] );
			break;
		case KEY_3:
		case KEY_PGDN:
		case KEY_PAD_3:
         KeyPressed( psv, button = pKeyPad->keys[2 * pKeyPad->key_spacing->cols + 2] );
			break;
		case KEY_4:
		case KEY_LEFT:
		case KEY_PAD_4:
         KeyPressed( psv, button = pKeyPad->keys[1 * pKeyPad->key_spacing->cols + 0] );
			break;
		case KEY_5:
		case KEY_CENTER:
		case KEY_PAD_5:
         KeyPressed( psv, button = pKeyPad->keys[1 * pKeyPad->key_spacing->cols + 1] );
			break;
		case KEY_6:
		case KEY_RIGHT:
		case KEY_PAD_6:
         KeyPressed( psv, button = pKeyPad->keys[1 * pKeyPad->key_spacing->cols + 2] );
			break;
		case KEY_7:
		case KEY_HOME:
		case KEY_PAD_7:
         KeyPressed( psv, button = pKeyPad->keys[0 * pKeyPad->key_spacing->cols + 0] );
			break;
		case KEY_8:
		case KEY_UP:
		case KEY_PAD_8:
         KeyPressed( psv, button = pKeyPad->keys[0 * pKeyPad->key_spacing->cols + 1] );
			break;
		case KEY_9:
		case KEY_PGUP:
		case KEY_PAD_9:
         KeyPressed( psv, button = pKeyPad->keys[0 * pKeyPad->key_spacing->cols + 2] );
         break;
		//case KEY_ENTER:
		case KEY_PAD_ENTER:
         KeyPressed( psv, button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 2] );
         break;
		case KEY_PAD_DELETE:
		case KEY_DELETE:
         KeyPressed( psv, button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 0] );
         break;
		case KEY_PAD_MINUS:
         KeyPressed( psv, button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 0] );
         break;
		}
		{
			PCOMMON pc = GetKeyCommon( button );
			PressButton( pc, TRUE );
		}
	}
	else // key is being released...
	{
      if( pKeyPad ) switch( KEY_CODE(key) )
		{
		case KEY_0:
		case KEY_PAD_0:
		case KEY_INSERT:
         button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 1];
			break;
		case KEY_1:
		case KEY_END:
		case KEY_PAD_1:
         button = pKeyPad->keys[2 * pKeyPad->key_spacing->cols + 0];
			break;
		case KEY_2:
		case KEY_DOWN:
		case KEY_PAD_2:
         button = pKeyPad->keys[2 * pKeyPad->key_spacing->cols + 1];
			break;
		case KEY_3:
		case KEY_PGDN:
		case KEY_PAD_3:
         button = pKeyPad->keys[2 * pKeyPad->key_spacing->cols + 2];
			break;
		case KEY_4:
		case KEY_LEFT:
		case KEY_PAD_4:
         button = pKeyPad->keys[1 * pKeyPad->key_spacing->cols + 0];
			break;
		case KEY_5:
		case KEY_CENTER:
		case KEY_PAD_5:
         button = pKeyPad->keys[1 * pKeyPad->key_spacing->cols + 1];
			break;
		case KEY_6:
		case KEY_RIGHT:
		case KEY_PAD_6:
         button = pKeyPad->keys[1 * pKeyPad->key_spacing->cols + 2];
			break;
		case KEY_7:
		case KEY_HOME:
		case KEY_PAD_7:
         button = pKeyPad->keys[0 * pKeyPad->key_spacing->cols + 0];
			break;
		case KEY_8:
		case KEY_UP:
		case KEY_PAD_8:
         button = pKeyPad->keys[0 * pKeyPad->key_spacing->cols + 1];
			break;
		case KEY_9:
		case KEY_PGUP:
		case KEY_PAD_9:
         button = pKeyPad->keys[0 * pKeyPad->key_spacing->cols + 2];
         break;
		//case KEY_ENTER:
		case KEY_PAD_ENTER:
			button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 2];
         //InvokeEnterEvent( (PSI_CONTROL)psv );
         break;
		case KEY_PAD_DELETE:
		case KEY_DELETE:
         button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 0];
         break;
		case KEY_PAD_MINUS:
         button = pKeyPad->keys[3 * pKeyPad->key_spacing->cols + 0];
         break;
		}
		{
			PCOMMON pc = GetKeyCommon( button );
			PressButton( pc, FALSE );
		}
	}
}

void SetDisplayPadKeypad( PSI_CONTROL pc, PKEYPAD keypad )
{
	ValidatedControlData( PDISPLAY, keypad_display.TypeID, display, pc );
	if( display )
	{
		display->keypad = keypad;
		if( keypad )
		{
			if( keypad->flags.bAlphaNum )
			{
            DestroyFont( &display->font );
				display->font = RenderFontFile( WIDE("arial.ttf")
														, (display->height - 5) * 8 / 8
														, display->height - 5, 1 );
				if( !display->font )
					display->font = RenderFontFile( WIDE("fonts/arial.ttf")
															, (display->height - 5) * 8 / 8
															, display->height - 5, 1 );

			}
		}
	}
}


void SetNewKeypadFlags( int flags )
{
   new_flags = flags;
}

PSI_CONTROL MakeKeypad( PCOMMON parent
							 , S_32 x, S_32 y, _32 w, _32 h
							  // show current value display...
							 , _32 ID
							 , _32 flags
                       , CTEXTSTR accumulator_name
							 )
{
	PSI_CONTROL frame;
   new_flags = flags; // remaining data is passed normally...
	frame = MakeControl( parent, keypad_control.TypeID, x, y, w, h, ID );
	{
		ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, frame );
		if( keypad && accumulator_name )
		{
			keypad->accum = GetAccumulator( accumulator_name
													, keypad->flags.bAlphaNum?ACCUM_TEXT
													 :(keypad->flags.bEntry?0:ACCUM_DOLLARS)
													);
			SetAccumulatorUpdateProc( keypad->accum, KeypadAccumUpdated, (PTRSZVAL)keypad );
		}
	}
   return frame;
}


static void OnHideCommon( WIDE("Keypad Control") )( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		keypad->flags.bHidden = 1;
		keypad->flags.bResult = 1;
		keypad->flags.bResultStatus = 0;
	}
}

static void OnRevealCommon( WIDE("Keypad Control") )( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		keypad->flags.bHidden = 0;
		keypad->flags.bResult = 0;
	}
}


int GetKeyedText( PSI_CONTROL pc, TEXTSTR buffer, int buflen )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		return GetAccumulatorText( keypad->accum, buffer, buflen );
	}
   return 0;
}

S_64 GetKeyedValue( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		return GetAccumulatorValue( keypad->accum );
	}
   return 0;
}

void ClearKeyedEntry( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		keypad->flags.bResult = 0;
		keypad->flags.bResultStatus = 0;
		ClearAccumulator( keypad->accum );
	}
}

void ClearKeyedEntryOnNextKey( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
      keypad->flags.bClearOnNext = 1;
	}
}

void CancelKeypadWait( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad )
	{
		keypad->flags.bResult = 1;
		keypad->flags.bResultStatus = 0;
	}
}

int WaitForKeypadResult( PSI_CONTROL pc )
{
	ValidatedControlData( PKEYPAD, keypad_control.TypeID, keypad, pc );
	if( keypad->flags.bPassword || !keypad->flags.bDisplay || keypad->flags.bEntry )
	{
		while( !keypad->flags.bResult )
			if( !Idle() )
            WakeableSleep( 100 );
      keypad->flags.bResult = 0;
      return keypad->flags.bResultStatus;
	}
   return -1;
}

void CPROC HotkeyPressed( PTRSZVAL psv, PKEY_BUTTON button )
{
}


// so... lets allow a menu button external control...

// okay this isn't enough either... it'll be way too hard to configure (without evomenu framework)
PSI_CONTROL MakeKeypadHotkey( PSI_CONTROL frame
										, S_32 x
										, S_32 y
										, _32 w
										, _32 h
										, char *keypad
									 )
{
   PKEY_BUTTON hotkey;
	hotkey = MakeKeyEx( frame
							, x, y
							, w, h
							, 0
							, NULL //glare
							, NULL // up
							, NULL //down
							, NULL //mask
							, KEY_BACKGROUND_COLOR
							, Color( 220, 220, 12 )
							 // if flags is DISPLAY only (not password)
							, WIDE("") // key text
							, NULL
							, HotkeyPressed, 0
							, WIDE("") // value
							);

	return NULL;
}


