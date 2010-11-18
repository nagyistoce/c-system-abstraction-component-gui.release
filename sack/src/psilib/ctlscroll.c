#include <stdhdrs.h>
#include <string.h>
#include <sharemem.h>
#include <keybrd.h>

#include "controlstruc.h"
#include <psi.h>
#define BTN_LESS 100
#define BTN_MORE 101

//---------------------------------------------------------------------------
PSI_SCROLLBAR_NAMESPACE

typedef struct scrollbar_tag
{
   _32 attr;
	Image   surface; // this is the surface of the 'bar' itself
	int min     // smallest value of bar
	  , current // first line shown?
	  , max     // highest value of var
	  , range;  // amount of distance the 'display' shows
	int x, y, b;
	int grabbed_x, grabbed_y;

	int top, bottom, width, height;

	struct {
		unsigned int bDragging : 1;
		unsigned int bHorizontal : 1;
	}scrollflags;
	PCONTROL pcTopButton, pcBottomButton;
	void (CPROC*UpdatedPos)( PTRSZVAL psv, int type, int current);
	PTRSZVAL psvUpdate;
} SCROLLBAR, *PSCROLLBAR;


//---------------------------------------------------------------------------
//	if( pc->nType == SCROLLBAR_CONTROL )
//	{
//		PSCROLLBAR psb = (PSCROLLBAR)pc;
//	}

static int CPROC RenderScrollBar( PCONTROL pc )
{
   ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		Image surface = psb->surface;
		int top, bottom;
		// render top button...
		
		ClearImageTo( surface, basecolor(pc)[SCROLLBAR_BACK] );
		if( psb->range == 0 )
         return 1;

		top = psb->current - psb->min;
		bottom = top + psb->range;

		//Log3( WIDE("top: %d bottom: %d max: %d"), top, bottom, psb->max );
	
	   if( psb->scrollflags.bHorizontal )
	   {
	   	if( psb->max )
	   	{
				top = ( psb->width * top ) / psb->max;
				bottom = ( (psb->width - 1) * bottom ) / psb->max;
			}
			else
			{
				top = 0;
				bottom = psb->width - 1;
			}
			BlatColor( surface, top, 0
								, bottom-top
								, psb->height, basecolor(pc)[NORMAL] );
	   }
	   else
	   {
	   	if( psb->max )
	   	{
				top = ( psb->height * top ) / psb->max;
				bottom = ( (psb->height - 1) * bottom ) / psb->max;
			}	
			else
			{
				top = 0;
				bottom = psb->height - 1;
			}
			BlatColor( surface, 0, top
								, psb->width
								, bottom-top, basecolor(pc)[NORMAL] );
		}
		//Log2( WIDE("top: %d bottom: %d"), top, bottom );
		psb->top = top;
		psb->bottom = bottom;

      DrawThinFrameInverted( pc );

		if( psb->scrollflags.bHorizontal )
		{
			do_vline( surface, top, 1, psb->height-2, basecolor(pc)[HIGHLIGHT] );
			do_hline( surface, 1, top, bottom, basecolor(pc)[HIGHLIGHT] );
			do_hline( surface, psb->height-2, top, bottom, basecolor(pc)[SHADOW] );
			do_vline( surface, bottom, 1, psb->height-2, basecolor(pc)[SHADOW] );
		}
		else
		{
			do_hline( surface, top, 1, psb->width-2, basecolor(pc)[HIGHLIGHT] );
			do_vline( surface, 1, top, bottom, basecolor(pc)[HIGHLIGHT] );
			do_vline( surface, psb->width-2, top, bottom, basecolor(pc)[SHADOW] );
			do_hline( surface, bottom, 1, psb->width-2, basecolor(pc)[SHADOW] );
		}

	}
   return 1;
}

//---------------------------------------------------------------------------

void MoveScrollBar( PCONTROL pc, int type )
{
   ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		switch( type )
		{
		case UPD_1UP:
			if( psb->current )
            psb->current--;
			break;
		case UPD_1DOWN:
			if( ( psb->current + psb->range ) < psb->max )
            psb->current++;
			break;
		case UPD_RANGEUP:
         if( psb->current > psb->range )
				psb->current -= psb->range;
			else
            psb->current = 0;
			break;
		case UPD_RANGEDOWN:
			psb->current += psb->range;
			if( ( psb->current + psb->range ) > psb->max )
            psb->current = psb->max - psb->range;
			break;
			//case UPD_THUMBTO:
		}
		if( psb->UpdatedPos )
			psb->UpdatedPos( psb->psvUpdate, type, psb->current );
      SmudgeCommon(pc);
	}
}


//---------------------------------------------------------------------------

static int CPROC ScrollBarMouse( PCONTROL pc, S_32 x, S_32 y, _32 b )
{
   ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		if( psb->scrollflags.bDragging )
		{
			if( !(b & MK_LBUTTON ) )
				psb->scrollflags.bDragging = FALSE;
			else
			{
			   int desired_top;
			   if( psb->scrollflags.bHorizontal )
			   {
			   	// this is effectively -1 * the delta - so the scroll
			   	// goes 'increasing' to the right
			   	x -= psb->height;
					desired_top = ( psb->grabbed_x - x );
					desired_top = ( desired_top * psb->max ) / psb->width;
					if( desired_top > ( psb->max - psb->range ) )
						desired_top = psb->max - psb->range;
				   else if( desired_top < psb->min )
				   	desired_top = psb->min;
					if( psb->current != desired_top )
					{
						psb->current = desired_top;
                  MoveScrollBar( pc, UPD_THUMBTO );
					}
			   }
			   else
			   {
			   	y -= psb->width;
					desired_top = ( y - psb->grabbed_y );
					desired_top = ( desired_top * psb->max ) / psb->height;
					if( desired_top > ( psb->max - psb->range ) )
						desired_top = psb->max - psb->range;
				   else if( desired_top < psb->min )
				   	desired_top = psb->min;
					if( psb->current != desired_top )
					{
						psb->current = desired_top;
						MoveScrollBar( pc, UPD_THUMBTO );
					}
			   }
				// do something here...
			}
		}
		else if( ( b & MK_LBUTTON ) && !(psb->b & MK_LBUTTON ) )
		{
			if( psb->scrollflags.bHorizontal ) 
			{
				x -= psb->height;
				if( x < psb->top )
				{
					MoveScrollBar( pc, UPD_RANGEUP );
				} else if( x > psb->bottom )
				{
					MoveScrollBar( pc, UPD_RANGEDOWN );
				}
				else // was on the thumb...
				{
		         psb->grabbed_x = x - psb->top; // top/left *shrug*
	   	      psb->grabbed_y = y - psb->top;
					psb->scrollflags.bDragging = TRUE;
				}
			}
			else
			{
				y -= psb->width;
				if( y < psb->top )
				{
					MoveScrollBar( pc, UPD_RANGEUP );
				} else if( y > psb->bottom )
				{
					MoveScrollBar( pc, UPD_RANGEDOWN );
				}
				else // was on the thumb...
				{
		         psb->grabbed_x = x - psb->top; // top/left *shrug*
	   	      psb->grabbed_y = y - psb->top;
					psb->scrollflags.bDragging = TRUE;
				}
			}
		}
		psb->x = x;
		psb->y = y;
		psb->b = b;
	}	
   return 1;

}

//---------------------------------------------------------------------------

static void CPROC BottomPushed( PTRSZVAL psvBar, PSI_CONTROL pc )
{
   MoveScrollBar( (PCONTROL)psvBar, UPD_1DOWN );
}

//---------------------------------------------------------------------------

static void CPROC DrawBottomButton( PTRSZVAL psv, PSI_CONTROL pc )
{
	Image surface = pc->Surface;
	if( surface )
	{
		int mx = pc->surface_rect.width/2;
		int cx = pc->surface_rect.width/4
	, cy = pc->surface_rect.height/3;
      ClearImageTo( surface, basecolor(pc)[NORMAL] );
		if( IsButtonPressed( pc ) )
		{
			cx++;
         cy++;
		}
#define fline(s,x1,y1,x2,y2,c,a) do_lineAlpha(s,x1,y1,x2,y2,SetAlpha(c,a))
		fline( surface, mx, 2*cy+1, cx, cy+1, basecolor(pc)[SHADOW], 255 );
		fline( surface, mx, 2*cy+0, cx, cy+0, basecolor(pc)[SHADE], 128 );
		fline( surface, mx, 2*cy+2, cx, cy+2, basecolor(pc)[SHADE], 128 );
		fline( surface, mx, 2*cy+2, mx+(mx-cx)  , cy+2, basecolor(pc)[HIGHLIGHT], 255 );
		fline( surface, mx, 2*cy+0, mx+(mx-cx)  , cy+0, basecolor(pc)[SHADOW], 128 );
		fline( surface, mx, 2*cy+1, mx+(mx-cx)  , cy+1, basecolor(pc)[SHADE], 128 );
//			do_line( surface, cx - 3 - w, cy - 2, cx, cy + 1, basecolor(pc)[SHADE] );
//			do_line( surface, cx - 4 - w, cy - 2, cx, cy + 2, basecolor(pc)[SHADOW] );
//			do_line( surface, cx - 4 - w, cy - 1, cx, cy + 3, basecolor(pc)[SHADE] );
//		do_line( surface, cx, cy+3, cx+4, cy-1, basecolor(pc)[HIGHLIGHT] );
//		do_line( surface, cx, cy+2, cx+4, cy-2, basecolor(pc)[SHADE] );
//		do_line( surface, cx, cy+1, cx+3, cy-2, basecolor(pc)[SHADOW] );
	}
}

//---------------------------------------------------------------------------

static void CPROC DrawRightButton( PTRSZVAL psv, PCONTROL pc )
{
	Image surface = pc->Surface;
	if( surface )
	{
		int cx = pc->surface_rect.width/2
	, cy = pc->surface_rect.height/2;
      ClearImageTo( surface, basecolor(pc)[NORMAL] );
		if( IsButtonPressed( pc ) )
			cx++;
		do_line( surface, cy - 2, cx - 3, cy + 1, cx  , basecolor(pc)[SHADE] );
		do_line( surface, cy - 2, cx - 4, cy + 2, cx  , basecolor(pc)[SHADOW] );
		do_line( surface, cy - 1, cx - 4, cy + 3, cx  , basecolor(pc)[SHADE] );
		do_line( surface, cy + 3, cx    , cy - 1, cx+4, basecolor(pc)[HIGHLIGHT] );
		do_line( surface, cy + 2, cx    , cy - 2, cx+4, basecolor(pc)[SHADE] );
		do_line( surface, cy + 1, cx    , cy - 2, cx+3, basecolor(pc)[SHADOW] );
	}
}

//---------------------------------------------------------------------------

static void CPROC TopPushed( PTRSZVAL psvBar, PCONTROL pc )
{
   MoveScrollBar( (PCONTROL)psvBar, UPD_1UP );
}

//---------------------------------------------------------------------------

static void CPROC DrawTopButton( PTRSZVAL psv, PCONTROL pc )
{
	Image surface = pc->Surface;
	if( surface )
	{
		CDATA c;
		int mx = pc->surface_rect.width/2;
		int cx = pc->surface_rect.width/4
		  , cy = pc->surface_rect.height/3;
      ClearImageTo( surface, basecolor(pc)[NORMAL] );
		if( IsButtonPressed( pc ) )
			cx++;
		c = basecolor(pc)[SHADE];
		//c = SetAlpha( c, 128 );
		fline( surface, mx, cy+1, mx+(mx-cx), 2*cy+1, basecolor(pc)[SHADOW], 255 );
		fline( surface, mx, cy, mx+(mx-cx), 2*cy, basecolor(pc)[SHADE], 128 );
		fline( surface, mx, cy+2, mx+(mx-cx), 2*cy+2, basecolor(pc)[SHADE], 128 );
		fline( surface, mx, cy+2, cx, 2*cy+2, basecolor(pc)[HIGHLIGHT], 255 );
		fline( surface, mx, cy+0, cx, 2*cy+0, basecolor(pc)[SHADOW], 128 );
		fline( surface, mx, cy+1, cx, 2*cy+1, basecolor(pc)[SHADE], 128 );
	}
}

//---------------------------------------------------------------------------

static void CPROC DrawLeftButton( PTRSZVAL psv, PCONTROL pc )
{
	Image surface = pc->Surface;
	if( surface )
	{
		int cx = pc->surface_rect.width/2
	, cy = pc->surface_rect.height/2;
      // hmm hope clearimage uses a blatalpha..
      ClearImageTo( surface, basecolor(pc)[NORMAL] );
		if( IsButtonPressed( pc ) )
			cx++;
		do_line( surface, cy - 3, cx  , cy+1, cx+4, basecolor(pc)[SHADE] );
		do_line( surface, cy - 2, cx  , cy+2, cx+4, basecolor(pc)[SHADOW] );
		do_line( surface, cy - 1, cx  , cy+2, cx+3, basecolor(pc)[SHADE] );
		do_line( surface, cy + 2, cx-3, cy-1, cx  , basecolor(pc)[HIGHLIGHT] );
		do_line( surface, cy + 2, cx-4, cy-2, cx  , basecolor(pc)[SHADE] );
		do_line( surface, cy + 1, cx-4, cy-3, cx  , basecolor(pc)[SHADOW] );
		//plot( surface, cx + 4, cy + 2, basecolor(pc)[HIGHLIGHT] );
	}
}

//---------------------------------------------------------------------------

void SetScrollParams( PCONTROL pc, int min, int cur, int range, int max )
{
   ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		if( max < min )
		{
			int tmp;
			tmp = min;
			min = max;
			max = tmp;
		}
		psb->min = min;
		psb->max = max;
		psb->current = cur;
		psb->range = range;
		//Log( WIDE("Set scroll params - therefore render") );
      SmudgeCommon(pc);
	}
}

//---------------------------------------------------------------------------

PSI_CONTROL SetScrollBarAttributes( PSI_CONTROL pc, int attr )
{
	ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		psb->attr = attr;
		if( attr & SCROLL_HORIZONTAL )
		{
			psb->scrollflags.bHorizontal = 1;
         MoveImage( psb->surface, pc->rect.height, 0 );
			ResizeImage( psb->surface
						  , psb->width = pc->rect.width - 2*pc->rect.height
						  , psb->height = pc->rect.height );
			MoveSizeCommon( psb->pcTopButton
							  , 0, 0
							  , pc->rect.height, pc->rect.height
							  );
			MoveSizeCommon( psb->pcBottomButton
							  , pc->rect.width - pc->rect.height, 0
							  , pc->rect.height, pc->rect.height
                       );
			SetButtonDrawMethod( psb->pcTopButton, DrawLeftButton, (PTRSZVAL)psb );
			SetButtonDrawMethod( psb->pcBottomButton, DrawRightButton, (PTRSZVAL)psb );
		}
		else
		{
			psb->scrollflags.bHorizontal = 0;
			MoveImage( psb->surface, 0, pc->rect.width );
			ResizeImage( psb->surface
						  , psb->width = pc->rect.width
						  , psb->height = pc->rect.height - 2*pc->rect.width );
			MoveSizeCommon( psb->pcTopButton
							  , 0, 0
							  , pc->rect.width, pc->rect.width
							  );
			MoveSizeCommon( psb->pcBottomButton
							  , 0, pc->rect.height-pc->rect.width
							  , pc->rect.width, pc->rect.width
                       );

			SetButtonDrawMethod( psb->pcBottomButton, DrawBottomButton, (PTRSZVAL)psb );
			SetButtonDrawMethod( psb->pcTopButton, DrawTopButton, (PTRSZVAL)psb );
		}
	}
   return pc;
}

//CONTROL_PROC_DEF( SCROLLBAR_CONTROL, SCROLLBAR, ScrollBar, ()  )
int CPROC ConfigureScrollBar( PSI_CONTROL pc )
{
   //ARG( _32, attr );
	ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		psb->min = 0;
		psb->max = 1;
		psb->current = 0;
		psb->range = 1;
		{
			psb->surface = MakeSubImage( pc->Surface
												, 0, pc->rect.width
												, psb->width = pc->rect.width
												, psb->height = pc->rect.height - 2*pc->rect.width );
			psb->pcTopButton = MakePrivateControl( pc, CUSTOM_BUTTON
															 , 0, 0
															 , pc->rect.width, pc->rect.width
															 , BTN_LESS );
         SetButtonPushMethod( psb->pcTopButton, TopPushed, (PTRSZVAL)pc );
			psb->pcTopButton->flags.bNoFocus = TRUE;

			psb->pcBottomButton = MakePrivateControl( pc, CUSTOM_BUTTON
																 , 0, pc->rect.height-pc->rect.width
																 , pc->rect.width, pc->rect.width
																 , BTN_MORE );
         SetButtonPushMethod( psb->pcBottomButton, BottomPushed, (PTRSZVAL)pc );
			psb->pcBottomButton->flags.bNoFocus = TRUE;

         if( psb->width > psb->height )
				SetScrollBarAttributes( pc, SCROLL_HORIZONTAL );
         else
				SetScrollBarAttributes( pc, 0 );
		}
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
// flags may indicate - horizontal not vertical...

void SetScrollUpdateMethod( PCONTROL pc
					, void (CPROC*UpdateProc)(PTRSZVAL psv, int type, int current)
					, PTRSZVAL data )
{
   ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		psb->UpdatedPos = UpdateProc;
		psb->psvUpdate = data;
	}
}

static void CPROC ResizeScrollbar( PSI_CONTROL pc )
{
	ValidatedControlData( PSCROLLBAR, SCROLLBAR_CONTROL, psb, pc );
	if( psb )
	{
		S_32 width = 15;
		ScaleCoords( (PSI_CONTROL)pc, &width, NULL );
		// resize the scrollbar accordingly...
		//lprintf( WIDE( "Getting a resize on the scrollbar..." ) );
		if( psb->attr & SCROLL_HORIZONTAL )
		{
			MoveSizeCommon( psb->pcTopButton, 0, 0
							  , pc->rect.height, pc->rect.height );
			MoveSizeCommon( psb->pcBottomButton, pc->rect.width - pc->rect.height, 0
							  , pc->rect.height, pc->rect.height );
		}
		else
		{
			MoveSizeCommon( psb->pcTopButton, 0, 0
							  , pc->rect.width, pc->rect.width );
			MoveSizeCommon( psb->pcBottomButton, 0, pc->rect.height-pc->rect.width
							  , pc->rect.width, pc->rect.width );
		}
 	}
}


CONTROL_REGISTRATION
scroll_bar = { SCROLLBAR_CONTROL_NAME
				 , { { 18, 18 }, sizeof( SCROLLBAR ), BORDER_NONE }
				 , ConfigureScrollBar
				 , NULL
				 , RenderScrollBar
				 , ScrollBarMouse
};
PRIORITY_PRELOAD( RegisterScrollBar,PSI_PRELOAD_PRIORITY )
{
   DoRegisterControl( &scroll_bar );
	SimpleRegisterMethod( PSI_ROOT_REGISTRY WIDE("/control/") SCROLLBAR_CONTROL_NAME WIDE("/rtti")
							  , ResizeScrollbar
							  , WIDE("void"), WIDE("resize"), WIDE("(PSI_CONTROL)") );
}

PSI_SCROLLBAR_NAMESPACE_END

//---------------------------------------------------------------------------
// $Log: ctlscroll.c,v $
// Revision 1.28  2005/05/17 18:37:33  jim
// remove noisy logging.
//
// Revision 1.27  2005/03/22 12:41:58  panther
// Wow this transparency thing is going to rock! :) It was much closer than I had originally thought.  Need a new class of controls though to support click-masks.... oh yeah and buttons which have roundable scaleable edged based off of a dot/circle
//
// Revision 1.26  2005/03/07 00:13:08  panther
// Create custom buttons for top/bottom it correct parameters.
//
// Revision 1.25  2005/02/10 16:55:51  panther
// Fixing warnings...
//
// Revision 1.24  2004/12/16 10:46:04  panther
// Scrollbars work again!
//
// Revision 1.23  2004/12/16 10:32:45  panther
// Scroll scrollbar to font... handle rendering the top and bottom buttons better... next to restore function to scrollbars.
//
// Revision 1.22  2004/10/24 20:09:47  d3x0r
// Sync to psilib2... stable enough to call it mainstream.
//
// Revision 1.6  2004/10/10 09:07:58  d3x0r
// Short a couple frames on some of the updates... but looks like progress is positive.
//
// Revision 1.5  2004/10/08 13:07:42  d3x0r
// Okay beginning to look a lot like PRO-GRESS
//
// Revision 1.4  2004/10/07 04:37:16  d3x0r
// Okay palette and listbox seem to nearly work... controls draw, now about that mouse... looks like my prior way of cheating is harder to step away from than I thought.
//
// Revision 1.3  2004/10/06 09:52:16  d3x0r
// checkpoint... total conversion... now how does it work?
//
// Revision 1.2  2004/09/27 20:44:28  d3x0r
// Sweeping changes only a couple modules left...
//
// Revision 1.1  2004/09/19 19:22:31  d3x0r
// Begin version 2 psilib...
//
// Revision 1.21  2004/08/24 17:18:00  d3x0r
// Fix last couple c files for new control_proc_def macro
//
// Revision 1.20  2003/10/13 02:50:47  panther
// Font's don't seem to work - lots of logging added back in
// display does work - but only if 0,0 biased, cause the SDL layer sucks.
//
// Revision 1.19  2003/10/12 02:47:05  panther
// Cleaned up most var-arg stack abuse ARM seems to work.
//
// Revision 1.18  2003/09/11 13:09:25  panther
// Looks like we maintained integrety while overhauling the Make/Create/Init/Config interface for controls
//
// Revision 1.17  2003/09/04 11:18:32  panther
// Fix scrollbar/listbox issues - current item - insertion of sorted items, min/max/range
//
// Revision 1.16  2003/07/24 23:47:22  panther
// 3rd pass visit of CPROC(cdecl) updates for callbacks/interfaces
//
// Revision 1.15  2003/06/04 11:39:08  panther
// Added some logging for listbox/scrollbar controls
//
// Revision 1.14  2003/05/01 21:31:57  panther
// Cleaned up from having moved several methods into frame/control common space
//
// Revision 1.13  2003/04/30 16:12:05  panther
// Fix button def
//
// Revision 1.12  2003/03/30 19:40:14  panther
// Encapsulate pick color data better.
//
// Revision 1.11  2003/03/25 08:45:56  panther
// Added CVS logging tag
//
