//#define DEBUG_BANNER_DRAW_UPDATE
//#ifndef __cplusplus_cli
//#define USE_RENDER_INTERFACE banner_local.pdi
//#define USE_IMAGE_INTERFACE banner_local.pii
//#endif
#include <stdhdrs.h>
#include <render.h>
#include <sharemem.h>
#include <controls.h>
#include <timers.h>
#include <idle.h>
#include <psi.h>
#include <sqlgetoption.h>
#include <psi.h>

#define BANNER_DEFINED
#include "../include/banner.h"

	struct upd_rect{
		S_32 x, y;
      _32 w, h;
	};


typedef struct banner_tag
{
	_32 flags;
	PSI_CONTROL frame;
	PRENDERER renderer;
	_32 owners;
	// if text controls were a little more betterer this would be good...
	// they have textcolor, background color, borders, and uhmm they're missing
	// font, and centering rules... left/right/center,top/bottom/center,
	//PCONTROL message;
	PCONTROL okay, cancel;
	PCONTROL yes, no;
	struct banner_tag **me;
	_32 result;
	_32 timeout;
	_32 timer;
	_32 _b;
	PTHREAD pWaitThread;
	CDATA basecolor;
	CDATA textcolor;

	struct {
		BIT_FIELD bounds_set : 1;
	} bit_flags;

	struct upd_rect text_bounds;
	struct upd_rect old_bounds;

} BANNER;


struct banner_local_tag {
	struct {
		BIT_FIELD bInited : 1;
	} flags;
	_32 w, h;
	_32 _w, _h;
	S_32 x, y; // x/y offset for extended banner.

	Font font;
   Font explorer_font;
	Font custom_font;
   PBANNER banner;
	PIMAGE_INTERFACE pii;
	PRENDER_INTERFACE pdi;
};
static struct banner_local_tag banner_local;

//#define BANNER_X  ( ( banner_local.w * 1 ) / 10 )
//#define BANNER_Y  ( ( banner_local.h * 1 ) / 10 )
//#define BANNER_WIDTH  ( ( banner_local.w * 8 ) / 10 )
//#define BANNER_HEIGHT ( ( banner_local.h * 8 ) / 10 )
#define BANNER_X  ( banner_local.x + ( banner_local.w * 0 ) / 10 )
#define BANNER_Y  ( banner_local.y + ( banner_local.h * 0 ) / 10 )
#define BANNER_WIDTH  ( ( banner_local.w * 10 ) / 10 )
#define BANNER_HEIGHT ( ( banner_local.h * 10 ) / 10 )

#define EXPLORER_BANNER_X  ( ( banner_local.w * 0 ) / 16 )
#define EXPLORER_BANNER_Y  ( banner_local.h - ( banner_local.h * 1 ) / 16 )
#define EXPLORER_BANNER_WIDTH  ( ( banner_local.w * 16 ) / 16 )
#define EXPLORER_BANNER_HEIGHT ( ( banner_local.h * 1 ) / 16 )

//--------------------------------------------------------------------------

//CONTROL_REGISTRATION BannerControl;

//--------------------------------------------------------------------------

static void InitBannerFrame( void )
{
	if( !banner_local.flags.bInited )
	{
      TEXTCHAR font[256];
		InvokeDeadstart(); // register my control please... (fucking optimizations)
#ifndef __cplusplus_cli
		banner_local.pii = GetImageInterface();
		banner_local.pdi = GetDisplayInterface();
#endif
		GetDisplaySize( &banner_local.w, &banner_local.h );
#ifndef __NO_OPTIONS__
		SACK_GetProfileStringEx( "SACK/Widgets/Banner2", "Default Font", "arialbd.ttf", font, sizeof( font ), TRUE );
#else
		StrCpy( font, WIDE( "arialbd.ttf" ) );
#endif
		banner_local.font = RenderFontFile( font
									  , banner_local.w / 30, banner_local.h/20
									  , 3 );
		if( !banner_local.font )
		{
#ifndef __NO_OPTIONS__
			SACK_GetProfileStringEx( WIDE( "SACK/Widgets/Banner2" ), WIDE( "Alternate Font" ), WIDE( "fonts/arialbd.ttf" ), font, sizeof( font ), TRUE );
#else
			StrCpy( font, WIDE( "fonts/arialbd.ttf" ) );
#endif
			banner_local.font = RenderFontFile( font
										  , banner_local.w / 30, banner_local.h/20
														 , 3 );
		}

		banner_local.explorer_font = RenderFontFile( font
									  , banner_local.w / 60, banner_local.h/40
									  , 3 );
		banner_local.flags.bInited = TRUE;
	}
}

//--------------------------------------------------------------------------

static void CPROC SomeChoiceClicked( PTRSZVAL psv, PCONTROL pc )
{
	PBANNER banner = (PBANNER)psv;
   int choice = GetControlID( pc );
#ifdef DEBUG_BANNER_DRAW_UPDATE
	lprintf( WIDE( "SOME!" ) );
#endif
	banner->flags |= (BANNER_CLOSED);
	switch( choice )
	{
	case IDOK:
      banner->flags |= BANNER_OKAY;
	case IDCANCEL:
		break;

	case 3+IDOK:
		banner->flags |= BANNER_OKAY;
	case 3+IDCANCEL:
      banner->flags |= BANNER_EXTENDED_RESULT;
      break;
	}
	{
		PTHREAD thread = (banner)->pWaitThread;
		if( thread )
		{
			WakeThread( thread );
		}
	}
}

//--------------------------------------------------------------------------

static void CPROC OkayChoiceClicked( PTRSZVAL psv, PCONTROL pc )
{
	PBANNER banner = (PBANNER)psv;
#ifdef DEBUG_BANNER_DRAW_UPDATE
	lprintf( WIDE( "OKAY!" ) );
#endif
	banner->flags |= (BANNER_CLOSED|BANNER_OKAY);
	{
		PTHREAD thread = (banner)->pWaitThread;
		if( thread )
		{
			WakeThread( thread );
		}
	}
}

//--------------------------------------------------------------------------

static void CPROC CancelChoiceClicked( PTRSZVAL psv, PCONTROL pc )
{
	PBANNER banner = (PBANNER)psv;
#ifdef DEBUG_BANNER_DRAW_UPDATE
	lprintf( WIDE( "CANCEL!" ) );
#endif
	banner->flags |= (BANNER_CLOSED);
	{
		PTHREAD thread = (banner)->pWaitThread;
		if( thread )
		{
			WakeThread( thread );
		}
	}
}

//--------------------------------------------------------------------------

#define BANNER_NAME WIDE("Large font simple banner 2")
static int OnKeyCommon( BANNER_NAME )( PSI_CONTROL pc, _32 key )
{
	PBANNER *ppBanner = (PBANNER*)GetCommonUserData( pc );
	PBANNER banner;
	if( !ppBanner || !(*ppBanner ) )
		return 0; // no cllick, already closed.
#ifdef DEBUG_BANNER_DRAW_UPDATE
	lprintf( WIDE( "..." ) );
#endif
	if( IsKeyPressed( key ) )
	{
		banner = (*ppBanner);
		if( banner )
		{
			if( KEY_CODE( key ) == KEY_ENTER )
			{
#ifdef DEBUG_BANNER_DRAW_UPDATE
				lprintf( WIDE( "enter.." ) );
#endif
				if( ( banner->flags & BANNER_OPTION_OKAYCANCEL )
					&& ( banner->flags & BANNER_OPTION_YESNO ) )
				{
				}
				else if( ( banner->flags & BANNER_OPTION_OKAYCANCEL )
					|| ( banner->flags & BANNER_OPTION_YESNO ) )
               banner->flags |= BANNER_OKAY;
 			}
			if( !(banner->flags & BANNER_DEAD )
				&& ( banner->flags & BANNER_CLICK ) )
			{
				banner->flags |= (BANNER_CLOSED);
				{
					PTHREAD thread = banner->pWaitThread;
					if( thread )
					{
						WakeThread( thread );
					}
				}
				// test for obutton too so any keypress also clearsit.
				//RemoveBannerEx( ppBanner DBG_SRC );
			}
		}
	}
   return 1;
}

//--------------------------------------------------------------------------

static int OnMouseCommon( BANNER_NAME )
//static int CPROC ClickHandler
( PCONTROL pc, S_32 x, S_32 y, _32 b )
{
	//ValidatedControlData();
	PBANNER *ppBanner = (PBANNER*)GetCommonUserData( pc );
	PBANNER banner;
	if( !ppBanner || !(*ppBanner ) )
		return 0; // no cllick, already closed.
	banner = (*ppBanner);
	if( banner )
	{
		if( !(banner->flags & BANNER_DEAD )
			&& ( banner->flags & BANNER_CLICK ) )
		{
         // test for obutton too so any keypress also clearsit.
			if( ( !(b&(MK_OBUTTON)) && ( banner->_b & (MK_OBUTTON) ) )
				|| ( !(b&(MK_LBUTTON)) && ( banner->_b & (MK_LBUTTON) ) ) )
			{
#ifdef DEBUG_BANNER_DRAW_UPDATE
				lprintf( WIDE("Remove banner!") );
#endif
				RemoveBanner2Ex( ppBanner DBG_SRC );
			}
			banner->_b = b;
		}
	}
	return TRUE;
}

//--------------------------------------------------------------------------


static void DrawBannerCaption( PSI_CONTROL pc, PBANNER banner, Image surface, TEXTCHAR *caption, CDATA color, int yofs, int height, int left, int explorer )
{
	_32 y = 0;
   S_32 minx = BANNER_WIDTH;
	_32 w, h, maxw = 0;
	_32 char_h;
   int had_bounds;
	CTEXTSTR start = caption;
	CTEXTSTR end;
	if( banner->bit_flags.bounds_set )
	{
      // be kinda nice to be able to result this to the control to get a partial update to screen...
      //lprintf( "Clearing box %d %d %d %d", banner->text_bounds.x, banner->text_bounds.y
		//			, banner->text_bounds.w, banner->text_bounds.h );
		banner->old_bounds = banner->text_bounds;
		BlatColor( surface, banner->text_bounds.x, banner->text_bounds.y
					, banner->text_bounds.w, banner->text_bounds.h, banner->basecolor );
	}
	else
	{
      had_bounds = 0;
		banner->old_bounds.x = 0;
		banner->old_bounds.y = 0;
		banner->old_bounds.w = surface->width;
		banner->old_bounds.h = surface->height;
		BlatColor( surface, 0, 0, surface->width, surface->height, banner->basecolor );
	}
	//ClearImageTo( surface, GetBaseColor( NORMAL ) );
	GetStringRenderSizeFontEx( caption, strlen( caption ), &maxw, &h, &char_h
							 , banner_local.custom_font?banner_local.custom_font:explorer?banner_local.explorer_font:banner_local.font );
	y = yofs - (h/2);
   banner->text_bounds.y = y - 2;
	banner->text_bounds.h = h + 4;

	while( start )
	{
      S_32 x;
		end = StrChr( start, '\n' );
		if( !end )
		{
			end = start + StrLen(start);
		}
		w = GetStringSizeFontEx( start, end-start, NULL, &h
									  , banner_local.custom_font?banner_local.custom_font:explorer?banner_local.explorer_font:banner_local.font );
		x = ( BANNER_WIDTH - (left?maxw:w) ) /2;

		if( (x-2) < minx )
			minx = x - 2;
		if( (w+4) > maxw )
         maxw = w + 4;
		PutStringFontEx( surface
							, x+2
							, y+2
							, SetAlpha( ~color, 0x80 ), 0
							, start, end-start
							, banner_local.custom_font?banner_local.custom_font:explorer?banner_local.explorer_font:banner_local.font );
		PutStringFontEx( surface
							, x-2
							, y-2
							, SetAlpha( ~color, 0x80 ), 0
							, start, end-start
							, banner_local.custom_font?banner_local.custom_font:explorer?banner_local.explorer_font:banner_local.font );
		PutStringFontEx( surface
							, x
							, y
							, color, 0
							, start, end-start
							, banner_local.custom_font?banner_local.custom_font:explorer?banner_local.explorer_font:banner_local.font );
		y += h;
		if( end[0] )
			start = end+1;
		else
			start = NULL;
	}
	banner->text_bounds.x = minx;
	banner->text_bounds.w = maxw;
	banner->bit_flags.bounds_set = 1;

	{
		S_32 rx, ry;
		S_32 rx_right, ry_bottom;
      S_32 rx_tmp;
		_32 rw, rh;

      rx_right = banner->text_bounds.x + banner->text_bounds.w;
		rx_tmp = banner->old_bounds.x + banner->old_bounds.w;
		if( rx_tmp > rx_right )
         rx_right = rx_tmp;

      ry_bottom = banner->text_bounds.y + banner->text_bounds.h;
		rx_tmp = banner->old_bounds.y + banner->old_bounds.h;
		if( rx_tmp > ry_bottom )
         ry_bottom = rx_tmp;


		if( banner->text_bounds.x < banner->old_bounds.x )
		{
			rx = banner->text_bounds.x;
         rw = rx_right - rx;
		}
		else
		{
			rx = banner->old_bounds.x;
         rw = rx_right - rx;
		}

		if( banner->text_bounds.y < banner->old_bounds.y )
		{
			ry = banner->text_bounds.y;
         rh = ry_bottom - ry;
		}
		else
		{
			ry = banner->old_bounds.y;
         rh = ry_bottom - ry;
		}

		SetUpdateRegion( pc, rx, ry, rw, rh );
	}
}



//--------------------------------------------------------------------------

static int OnDrawCommon( BANNER_NAME )( PCONTROL pc )
{
	TEXTCHAR caption[256];
	PBANNER *ppBanner = (PBANNER*)GetCommonUserData( pc );
	if( !ppBanner )
		return 0;
	else
	{
		PBANNER banner = (*ppBanner);
		if( banner )
		{
			Image image = GetControlSurface( pc );
			GetControlText( pc, caption, sizeof( caption )/sizeof(TEXTCHAR) );
#ifdef DEBUG_BANNER_DRAW_UPDATE
			lprintf( WIDE( "--------- BANNER DRAW %s -------------" ), caption );
#endif
			DrawBannerCaption( pc, banner, image
								  , caption
								  , banner->textcolor
								  , (banner->flags & ( BANNER_OPTION_YESNO|BANNER_OPTION_OKAYCANCEL ))
									? ( (image->height * 1 ) / 3 )
									: ((image->height)/2)
								  , 0
								  , banner->flags & BANNER_OPTION_LEFT_JUSTIFY
                          , banner->flags & BANNER_EXPLORER
								  );
		}
	}
   return TRUE;
}

CONTROL_REGISTRATION banner_control = { BANNER_NAME
												  , { { 0, 0 }, 0
													 , BORDER_BUMP
													 | BORDER_NOMOVE
													 | BORDER_NOCAPTION
													 | BORDER_FIXED }
};
PRIORITY_PRELOAD( RegisterBanner2, 65 ) { DoRegisterControl( &banner_control ); }
//--------------------------------------------------------------------------


void CPROC killbanner( PTRSZVAL psv )
{
	PBANNER *ppBanner = (PBANNER*)psv;
	{
      //lprintf( WIDE( "killing banner..." ) );
      RemoveBanner2Ex( ppBanner DBG_SRC );
	}
}

// BANNER_OPTION_YES_NO
// BANNER_OPTION_OKAY_CANCEL ... etc ? sound flare, colors, blinkings...
// all kinda stuff availabel...

int CreateBanner2Extended( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor, int lines, int cols, int display )
{
	//PBANNER newBanner;
	S_32 x, y;
   _32 w, h;
	PBANNER banner;
	LOGICAL bUpdateLocked = FALSE;
	InitBannerFrame();
#ifdef DEBUG_BANNER_DRAW_UPDATE
	lprintf( WIDE("Create banner '%s' %p %p %p"), text, parent, ppBanner, ppBanner?*ppBanner:NULL );
#endif
	if( !ppBanner )
		ppBanner = &banner_local.banner;
	banner = *ppBanner;
	if( !(banner) )
	{
		banner = New( BANNER );
		MemSet( banner, 0, sizeof( BANNER ) );
		banner->flags |= BANNER_WAITING; // clear this off when done
		// but please attempt to prevent destruction during creation....
		// setting this as waiting will inhibit that, but it's not really
		// waiting, so we'll have to check and validate at the end...
		*ppBanner = banner;
	}
	banner_local._w = banner_local.w;
	banner_local._h = banner_local.h;
	x = 0;
	y = 0;
	banner->textcolor = textcolor?textcolor:0xFF0d0d0d;
	banner->basecolor = basecolor?basecolor:0x01135393;

	GetDisplaySizeEx( display, &x, &y, &w, &h );



	if( lines || cols )
	{
      TEXTCHAR font[256];
		if( !lines )
			lines = 20;
		if( !cols )
			cols = 30;
#ifndef __NO_OPTIONS__
		SACK_GetProfileStringEx( WIDE( "SACK/Widgets/Banner" ), WIDE( "Default Font" ), WIDE( "arialbd.ttf" ), font, sizeof( font ), TRUE );
#else
		StrCpy( font, WIDE( "arialbd.ttf" ) );
#endif
		banner_local.custom_font = RenderFontFile( font
															  , w /cols
															  , h/lines
															  , 3 );
		if( !banner_local.custom_font )
		{
#ifndef __NO_OPTIONS__
			SACK_GetProfileStringEx( WIDE( "SACK/Widgets/Banner" ), WIDE( "Alternate Font" ), WIDE( "fonts/arialbd.ttf" ), font, sizeof( font ), TRUE );
#else
			StrCpy( font, WIDE( "fonts/arialbd.ttf" ) );
#endif
			banner_local.custom_font = RenderFontFile( font
																  , w /cols
																  , h/lines
																  , 3 );
		}
	}
	else
	{
		DestroyFont( &banner_local.custom_font );
		banner_local.custom_font = 0;
	}

	if( !banner->renderer )
	{
		banner->renderer = OpenDisplayAboveSizedAt( DISPLAY_ATTRIBUTE_LAYERED
																, (options & BANNER_EXPLORER)?EXPLORER_BANNER_WIDTH:w
																, (options & BANNER_EXPLORER)?EXPLORER_BANNER_HEIGHT:h
																, (options & BANNER_EXPLORER)?EXPLORER_BANNER_X:x
																, (options & BANNER_EXPLORER)?EXPLORER_BANNER_Y:y
																, parent );
		//DebugBreak();
		if( (parent && IsTopmost( parent ) )|| ( options & BANNER_TOP ) )
		{
			if( options & BANNER_ABSOLUTE_TOP )
				MakeAbsoluteTopmost( banner->renderer );
         else
				MakeTopmost( banner->renderer );
		}
	}
	if( !banner->frame )
	{
//		GetDisplaySize( &banner_local.w, &banner_local.h );
		banner->frame = MakeCaptionedControl( NULL, banner_control.TypeID
														, (options & BANNER_EXPLORER)?EXPLORER_BANNER_X:x
														, (options & BANNER_EXPLORER)?EXPLORER_BANNER_Y:y
														, (options & BANNER_EXPLORER)?EXPLORER_BANNER_WIDTH:w
														, (options & BANNER_EXPLORER)?EXPLORER_BANNER_HEIGHT:h
														, 0
														, text );

		//banner->frame = CreateFrameFromRenderer( text
		//													, BORDER_WANTMOUSE | BORDER_BUMP | BORDER_NOMOVE | BORDER_NOCAPTION | BORDER_FIXED
		//													, banner->renderer
		//													);
		AttachFrameToRenderer( banner->frame, banner->renderer );
		SetCommonUserData( banner->frame, (PTRSZVAL)ppBanner );
		banner->owners = 1;
	}
	else
	{
		banner->owners++;
#ifdef DEBUG_BANNER_DRAW_UPDATE
		lprintf( WIDE("Using exisiting banner text (created twice? ohohoh count!)") );
#endif
		//if( banner_local._w != banner_local.w || banner_local._h != banner_local.h )
#if 0
			MoveSizeCommon( banner->frame
							  , (options & BANNER_EXPLORER)?EXPLORER_BANNER_X:x
							  , (options & BANNER_EXPLORER)?EXPLORER_BANNER_Y:y
							  , (options & BANNER_EXPLORER)?EXPLORER_BANNER_WIDTH:w
							  , ((options & BANNER_EXPLORER))?EXPLORER_BANNER_HEIGHT:h
							  );
#endif
		EnableCommonUpdates( banner->frame, FALSE );
		bUpdateLocked = TRUE;
		SetCommonText( banner->frame, text );
	}
	if( !options ) // basic options...
      options = BANNER_CLICK|BANNER_TIMEOUT;
	SetBanner2OptionsEx( ppBanner, options, timeout );
	if( banner->owners == 1 )
	{
		DisplayFrame( banner->frame );
	}
	if( bUpdateLocked )
	{
		EnableCommonUpdates( banner->frame, TRUE );
      SmudgeCommon( banner->frame ); // do this so it actually draws out...
	}
	//else
	//   SmudgeCommon( banner->frame );
	banner->flags &= ~BANNER_WAITING;
	if( banner->flags & BANNER_CLOSED )
	{
#ifdef DEBUG_BANNER_DRAW_UPDATE
		lprintf( WIDE("-----------------------! BANNER DESTROYED EVEN AS IT WAS CREATED!") );
#endif
		RemoveBanner2Ex( ppBanner DBG_SRC );
		return FALSE;
	}
	banner->me = ppBanner;
#ifdef DEBUG_BANNER_DRAW_UPDATE
	lprintf( WIDE("Created banner %p"), banner );
#endif
	if( !(options & BANNER_NOWAIT ) )
		return WaitForBanner2Ex( ppBanner );
	return TRUE;
}

int CreateBanner2Exx( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor )
{
   return CreateBanner2Extended( parent, ppBanner, text, options, timeout, textcolor, basecolor, 0, 0, 0 );
}

int CreateBanner2Ex( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout )
{
   return CreateBanner2Exx( parent, ppBanner, text, options, timeout, 0, 0 );
}
//--------------------------------------------------------------------------

int CreateBanner2( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text )
{
	int retval = 0;

	retval = CreateBanner2Ex( parent, ppBanner, text, BANNER_CLICK|BANNER_TIMEOUT, 5000 );

#ifdef DEBUG_BANNER_DRAW_UPDATE
   lprintf(WIDE("CreateBanner is done, retval is %d"), retval);
#endif
	return retval;
}

//--------------------------------------------------------------------------

#undef RemoveBanner2
// provided for migratory compatibility
void RemoveBanner2( PBANNER banner )
{
	RemoveBanner2Ex( &banner DBG_SRC );
}

void RemoveBanner2Ex( PBANNER *ppBanner DBG_PASS )
{
	PBANNER banner;
   //_lprintf( DBG_RELAY )( "Removing something.." );
	if( !ppBanner )
	{
      //lprintf( "remove banner local.." );
		ppBanner = &banner_local.banner;
	}
	banner = ppBanner?(*ppBanner ):(banner_local.banner);
	//#ifdef DEBUG_BANNER_DRAW_UPDATE
	{
		TEXTCHAR buf[256];
		if( banner )
			GetControlText( banner->frame, buf, sizeof( buf )/sizeof(TEXTCHAR) );
#ifdef DEBUG_BANNER_DRAW_UPDATE
		_xlprintf(LOG_ALWAYS DBG_RELAY)( WIDE("Remove banner %p %p %d %s")
												 , banner, banner_local.banner
												 , banner_local.banner?banner_local.banner->owners:123, buf  );
#endif
	}
	//#endif
	if( !banner )
	{
		banner = banner_local.banner;
	}
	if( banner )
	{
		if( banner->flags & BANNER_WAITING )
		{
#ifdef DEBUG_BANNER_DRAW_UPDATE
			lprintf( WIDE("Marking banner for later removal") );
#endif
			banner->flags |= BANNER_CLOSED;
		}
		else
		{
			if( --banner->owners )
			{
				// no - remove is intended to kill any banner exisiting at
            // this as a banner...
#ifdef DEBUG_BANNER_DRAW_UPDATE
				lprintf( WIDE("Wiating for last owner to remove banner") );
#endif
            //return;
			}
#ifdef DEBUG_BANNER_DRAW_UPDATE
			lprintf( WIDE("Banner is not waiting") );
#endif
			DestroyCommon( &banner->frame );
			if( banner->timer )
			{
            //lprintf( "Removingi timer..." );
				RemoveTimer( banner->timer );
				banner->timer = 0;
			}
         // stand alone banners?
         if( banner->me )
				(*banner->me) = NULL;
#ifdef DEBUG_BANNER_DRAW_UPDATE
			lprintf( WIDE("Releasing banner...") );
#endif

			Release( banner );
			(*ppBanner) = NULL;
			if( banner_local.banner == banner )
				banner_local.banner = NULL;
		}
	}
}


//--------------------------------------------------------------------------

void SetBanner2Text( PBANNER banner, TEXTCHAR *text )
{
	if( !banner )
	{
		if( !banner_local.banner )
         CreateBanner2( NULL, &banner_local.banner, text );
		banner = banner_local.banner;
	}
	if( banner )
		SetCommonText( banner->frame, text );
}

//--------------------------------------------------------------------------

static void CPROC BannerTimeout( PTRSZVAL ppsv )
{
	PBANNER *ppBanner = (PBANNER*)ppsv;
   int delta = (int)(*ppBanner)->timeout - (int)timeGetTime();
	//PBANNER banner = ppBanner?(*ppBanner):NULL;
	if( delta < 0 )
	{
		PTHREAD thread = ( ppBanner && (*ppBanner) )?(*ppBanner)->pWaitThread:NULL;
		RemoveBanner2Ex( ppBanner DBG_SRC );
		// timer goes away after this.
		if( thread )
		{
			WakeThread( thread );
		}
	}
	else
	{
		//lprintf( "Timer hasn't actually expired yet... %d %d", (*ppBanner)->timer, delta );
		RescheduleTimerEx( (*ppBanner)->timer, delta );
	}
}

//--------------------------------------------------------------------------
/*
void CPROC DrawButton( PTRSZVAL psv, PCONTROL pc )
{
	Image Surface = GetControlSurface( pc );
	ClearImageTo( pb->common.common.Surface, GetBaseColor( NORMAL ) );
   DrawStrong( banner->font,
}
*/
//--------------------------------------------------------------------------

void SetBanner2OptionsEx( PBANNER *ppBanner, _32 flags, _32 extra  )
{
   PBANNER banner;
	if( !ppBanner )
		ppBanner  = &banner_local.banner;
   banner = (*ppBanner);
	if( !banner )
	{
		banner = banner_local.banner;
	}
	if( banner )
	{
		banner->flags = flags;
		if( flags & BANNER_TIMEOUT )
		{
			banner->timeout = timeGetTime() + extra;
			if( !banner->timer )
			{
				banner->timer = AddTimerEx( extra, 0, BannerTimeout, (PTRSZVAL)ppBanner );
			}
			else
			{
				RescheduleTimerEx( banner->timer, extra );
			}
		}
		else
		{
			if( banner->timer )
			{
				RemoveTimer( banner->timer );
				banner->timer = 0;
			}
		}
		if( (banner->flags & (BANNER_OPTION_YESNO|BANNER_OPTION_OKAYCANCEL)) == (BANNER_OPTION_YESNO|BANNER_OPTION_OKAYCANCEL) )
		{
			banner->cancel = MakeButton( banner->frame
												, 5
												, ( ( BANNER_HEIGHT * 5 ) / 6 ) - 5 - 10
												, ( BANNER_WIDTH / 2) - 10
												, ((BANNER_HEIGHT * 1 ) / 6 ) - 20
												, 3+IDCANCEL
												, WIDE( "Cancel" ), 0
												, SomeChoiceClicked
												, (PTRSZVAL)banner );
         SetButtonColor( banner->cancel, 0xdf9a051d );
			banner->okay = MakeButton( banner->frame
											 , 5 + ( BANNER_WIDTH / 2) - 5
											 , ( ( BANNER_HEIGHT * 5 ) / 6 ) - 5 - 10
											 , ( BANNER_WIDTH / 2) - 10
											 , ((BANNER_HEIGHT * 1 ) / 6 ) - 20
											 , 3+IDOK
											 , WIDE( "Okay!" ), 0
											 , SomeChoiceClicked
											 , (PTRSZVAL)banner );
         SetButtonColor( banner->okay, 0xdf18986c );
			banner->yes = MakeButton( banner->frame
												, 5, ( ( BANNER_HEIGHT * 4 ) / 6 ) - 5 - 10
											, ( BANNER_WIDTH / 2) - 10
											, ((BANNER_HEIGHT * 1 ) / 6 ) - 20
												, IDCANCEL
												, WIDE( "No" ), 0
												, SomeChoiceClicked
												, (PTRSZVAL)banner );
         SetButtonColor( banner->yes, 0xdf9a051d );
			banner->no = MakeButton( banner->frame
										  , 5 + ( BANNER_WIDTH / 2) - 5
										  , ( ( BANNER_HEIGHT * 4 ) / 6 ) - 5 - 10
											 , ( BANNER_WIDTH / 2) - 10, ((BANNER_HEIGHT * 1 ) / 6 ) - 20
											 , IDOK
											 , WIDE( "Yes" ), 0
											 , SomeChoiceClicked
											 , (PTRSZVAL)banner );
         SetButtonColor( banner->no, 0xdf18986c );
         //SetBaseColor( TEXTCOLOR, 0xffFFFFFF );
         SetControlColor( banner->okay, TEXTCOLOR, 0xffFFFFFF );
         SetControlColor( banner->cancel, TEXTCOLOR, 0xffFFFFFF );
         SetControlColor( banner->yes, TEXTCOLOR, 0xffFFFFFF );
         SetControlColor( banner->no, TEXTCOLOR, 0xffFFFFFF );

		}
      //if( flags & BANNER_TOP )
		//	MakeTopmost( banner->renderer );
		else if( banner->flags & (BANNER_OPTION_YESNO|BANNER_OPTION_OKAYCANCEL) )
		{
			banner->cancel = MakeButton( banner->frame
												, 5, ( ( BANNER_HEIGHT * 2 ) / 3 ) - 5 - 10
												, ( BANNER_WIDTH / 2) - 10, ((BANNER_HEIGHT * 1 ) / 3 ) - 20
												, IDCANCEL
												, ( flags & BANNER_OPTION_YESNO )?WIDE( "No" ):WIDE( "Cancel" ), 0
												, CancelChoiceClicked
												, (PTRSZVAL)banner );
         SetButtonColor( banner->cancel, 0xdf9a051d );
			banner->okay = MakeButton( banner->frame
											 , 5 + ( BANNER_WIDTH / 2) - 5, ( ( BANNER_HEIGHT * 2 ) / 3 ) - 5 - 10
											 , ( BANNER_WIDTH / 2) - 10, ((BANNER_HEIGHT * 1 ) / 3 ) - 20
											 , IDOK
											 , ( flags & BANNER_OPTION_YESNO )?WIDE( "Yes" ):WIDE( "Okay!" ), 0
											 , OkayChoiceClicked
											 , (PTRSZVAL)banner );
         SetButtonColor( banner->okay, 0xdf18986c );
         SetControlColor( banner->okay, TEXTCOLOR, 0xffFFFFFF );
         SetControlColor( banner->cancel, TEXTCOLOR, 0xffFFFFFF );
			//SetCommonMouse( banner->frame, NULL, 0 );
		}
		else
		{
			if( banner->okay )
			{
				DestroyCommon( &banner->okay );
				// this is taken care of by DestroyCommon- which is why we pass the address of banner->okay
				//banner->okay = NULL;
			}
			if( banner->cancel )
			{
				DestroyCommon( &banner->cancel );
				// this is taken care of by DestroyCommon- which is why we pass the address of banner->cancel
				//banner->cancel = NULL;
			}
			//SetFrameMouse( banner->frame, ClickHandler, (PTRSZVAL)banner );
		}
#if REQUIRE_PSI(1,1)
		SetFrameFont( banner->frame, banner_local.font );
#endif
		SmudgeCommon( banner->frame );
 	}
}

//--------------------------------------------------------------------------

#undef WaitForBanner2
int WaitForBanner2( PBANNER banner )
{
   return WaitForBanner2Ex( &banner );
}

int WaitForBanner2Ex( PBANNER *ppBanner )
{
	if( !ppBanner )
      ppBanner = &banner_local.banner;
	if( *ppBanner )
	{
		int result = 0;
#ifdef DEBUG_BANNER_DRAW_UPDATE
		lprintf( WIDE("------- BANNER BEGIN WAITING --------------") );
#endif
		(*ppBanner)->flags |= BANNER_WAITING;
#ifdef DEBUG_BANNER_DRAW_UPDATE
		lprintf( WIDE("------- BANNER BEGIN WAITING  REALLY --------------") );
#endif
		(*ppBanner)->pWaitThread = MakeThread();
		while( (*ppBanner) && ( !((*ppBanner)->flags & BANNER_CLOSED) ) )
		{
			if( !Idle() )
				WakeableSleep( 250 );
		}
		if( *ppBanner )
		{
			(*ppBanner)->flags &= ~BANNER_WAITING;
			result = (((*ppBanner)->flags & BANNER_OKAY )?1:0) | (((*ppBanner)->flags & BANNER_EXTENDED_RESULT )?2:0);
#ifdef DEBUG_BANNER_DRAW_UPDATE
			lprintf( WIDE("Remove (*ppBanner)!") );
#endif
			RemoveBanner2Ex( ppBanner DBG_SRC );
			(*ppBanner) = NULL;
		}
      return result;
	}
   return FALSE;
}

//--------------------------------------------------------------------------

Font GetBanner2Font( void )
{
   InitBannerFrame();
   return banner_local.font;
}

//--------------------------------------------------------------------------

_32 GetBanner2FontHeight( void )
{
   InitBannerFrame();
   return GetFontHeight( banner_local.font );
}

//--------------------------------------------------------------------------

PRENDERER GetBanner2Renderer( PBANNER banner )
{
	if( !banner )
		banner = banner_local.banner;
	if( !banner )
		return NULL;
	return banner->renderer;
}

PSI_CONTROL GetBanner2Control( PBANNER banner )
{
	if( !banner )
		banner = banner_local.banner;
	if( !banner )
		return NULL;
	return banner->frame;
}

//--------------------------------------------------------------------------

struct banner_confirm_local
{
   CTEXTSTR name;
	struct {
		BIT_FIELD bNoResult : 1;
		BIT_FIELD key_result : 1;
		BIT_FIELD key_result_yes : 1;
		BIT_FIELD key_result_no : 1;
		BIT_FIELD bLog : 1;
	} flags;
   PTHREAD pWaiting;
   PBANNER banner;
	DoConfirmProc dokey;
};

typedef struct banner_confirm_local *PCONFIRM_BANNER;
typedef struct banner_confirm_local CONFIRM_BANNER;
static PLIST confirmation_banners;

struct thread_params {
   CTEXTSTR type;
   int yesno;
	CTEXTSTR msg;
	DoConfirmProc dokey;
   int received;
};


static PCONFIRM_BANNER GetWaitBanner2( CTEXTSTR name )
{
	INDEX idx;
	PCONFIRM_BANNER cb;
	LIST_FORALL( confirmation_banners, idx, PCONFIRM_BANNER, cb )
	{
		if( StrCaseCmp( name, cb->name ) == 0 )
		{
         break;
		}
	}
	if( !cb )
	{
		cb = New( CONFIRM_BANNER );
      MemSet( cb, 0, sizeof( CONFIRM_BANNER ) );
		cb->name = StrDup( name );
      AddLink( &confirmation_banners, cb );
	}
   return cb;
}

void Banner2AnswerYes( CTEXTSTR type )
{
	PCONFIRM_BANNER cb = GetWaitBanner2( type );
	if( cb->banner )
	{
		cb->flags.key_result_yes = 1;
		cb->flags.key_result_no = 0;
		cb->flags.key_result = 1;
		RemoveBanner2( cb->banner );
	}
}
void Banner2AnswerNo( CTEXTSTR type )
{
	PCONFIRM_BANNER cb = GetWaitBanner2( type );
	if( cb->banner )
	{
		cb->flags.key_result_no = 1;
		cb->flags.key_result_yes = 0;
		cb->flags.key_result = 1;
		RemoveBanner2( cb->banner );
	}
}

static PTRSZVAL CPROC Confirm( PTHREAD thread )
{
	struct thread_params *parms = ( struct thread_params * )GetThreadParam( thread );
	CTEXTSTR msg;
	int result;
   int yesno;
	PCONFIRM_BANNER cb = GetWaitBanner2( parms->type );
   // only one ...
	cb->dokey = parms->dokey;
	msg = parms->msg;
   yesno = parms->yesno;
	parms->received = 1;
   cb->pWaiting = thread;
	result = CreateBanner2Ex( NULL, &cb->banner, parms->msg, BANNER_TOP|(yesno?BANNER_OPTION_YESNO:(BANNER_CLICK|BANNER_TIMEOUT))
								  , (yesno?0:5000) );
	if( cb->flags.bLog )
		lprintf( "returned from banner..." );

	if( cb->flags.key_result )
	{
      // consume the result
      cb->flags.key_result = 0;
		if( cb->flags.key_result_yes )
		{
			if( cb->dokey )
				cb->dokey();
			cb->dokey = NULL;
			return 1;
		}
      return 0;
	}
	else
	{
		if( cb->flags.bLog )
         lprintf( "Banner yesno was clicked..." );
		if( result )
		{
         lprintf( "Okay do that ... invoke pending event." );
			if( cb->dokey )
				cb->dokey();
			cb->dokey = NULL;
		}
	}
	if( cb->flags.bNoResult )
	{
		if( cb->flags.bLog )
			lprintf( "Result already consumed..." );
      return -1;
	}

	if( cb->flags.bLog )
		lprintf( "Yes no is %d", result );
   cb->flags.bNoResult = 1;
   return result;


}

static int Banner2ThreadConfirmEx( CTEXTSTR type, CTEXTSTR msg, DoConfirmProc dokey, int yesno )
{
	struct thread_params parms;
   parms.type = type;
   parms.received = 0;
	parms.dokey = dokey;
	parms.msg = msg;
	parms.yesno = yesno;
	ThreadTo( Confirm, (PTRSZVAL)&parms );
	while( !parms.received )
      Relinquish();
	return 0;
}

int Banner2ThreadConfirm( CTEXTSTR type, CTEXTSTR msg, DoConfirmProc dokey )
{
   return Banner2ThreadConfirmEx( type, msg, dokey, TRUE );
}

//--------------------------------------------------------------------------

#ifdef __cplusplus_cli
#include <vcclr.h>

namespace SACK {
	public ref class Banner
	{
	public:
		//PBANNER banner;
		static Banner()
		{
			// don't really do anything....

		}

		static void Message( System::String^ message )
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars(message);
			size_t convertedChars = 0;
			size_t  sizeInBytes = ((message->Length + 1) * 2);
			errno_t err = 0;
			char    *ch = NewArray(TEXTCHAR,sizeInBytes);


			err = wcstombs_s(&convertedChars, 
                    ch, sizeInBytes,
                    wch, sizeInBytes);

			CreateBanner2Ex( NULL, NULL, ch, BANNER_NOWAIT|BANNER_DEAD, 0 );
			Release( ch );
		}
		static void Remove( void )
		{
			RemoveBanner2Message();
			//BannerMessageNoWait( string );
		}

	}  ;
}


#endif
