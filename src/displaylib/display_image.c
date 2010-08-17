//#define DIRTY_RECT_DEBUG
//#define NO_LOGGING
//#include "displaystruc.h"

//#include <display.h>
//#include "image.h"
#include <stdhdrs.h>
#include <timers.h>
#include "global.h"
#include "spacetree.h"

// this is the wrapper interface for image - this will
// observe the regions/panels and clip the operation
// potentially queing significant changes to a list which 
// may at a later time be flushed.
// This will produce least significant change updates.

// this will also only write into the container's surface!
RENDER_NAMESPACE
//-------------------------------------------------------------------------

 Image DisplayMakeSubImageEx( Image pImage
														, S_32 x, S_32 y
														, _32 width, _32 height DBG_PASS )
{
	Image newImage;
	newImage = MakeSubImageEx( pImage, x, y, width, height DBG_RELAY );
   SetImageAuxRect( newImage, (P_IMAGE_RECTANGLE)newImage );
	//Log7( WIDE("made a subimage: %p (%d,%d)-(%d,%d) %d x %d")
	//	 , newImage, x, y, x + width, y + height, width, height );
	if( pImage && ( pImage->flags & IF_FLAG_IS_PANEL ) )
	{
      //Log( WIDE("Cloning PANEL flag...") );
		newImage->flags |= IF_FLAG_IS_PANEL;
	}
	return newImage;
}

//-------------------------------------------------------------------------

// results in the panel-root that contains this image
// the rectangle passed (if any) contains the actual rectangle
// of this image within the parent image.
// any operations need to have this rectangle's x and y added 
// to them. 

Image FindImageRoot( Image _this, P_IMAGE_RECTANGLE realrect )
{
	if( realrect )
	{
		realrect->x = 0;
		realrect->y = 0;
      //Log2( WIDE("This image's dims (%d,%d)"), _this->real_width, _this->real_height );
		realrect->width = _this->real_width;
		realrect->height = _this->real_height;
	}
	if( !(_this->flags & IF_FLAG_IS_PANEL) )
	{
		return _this; // can use _this image natural.
	}
	while( _this && 
	       !( _this->flags & IF_FLAG_PANEL_ROOT ) && 
	       _this->pParent )
   {
		if( realrect )
		{
         //Log2( WIDE("Adding offset (%d,%d) to realrect base"), _this->real_x, _this->real_y );
			realrect->x += _this->real_x;
			realrect->y += _this->real_y;
		}
		if( ( _this = _this->pParent ) && realrect && realrect->width )
		{
			if( realrect->x > _this->real_width ||
			    realrect->y > _this->real_height ||
	   		 (realrect->x + (S_32)realrect->width) < 0 ||
		   	 (realrect->y + (S_32)realrect->height) < 0 )
			{
				//Log4( WIDE("Failing find! (%d>%d) or (%d>%d)")
				//	 , realrect->x, _this->real_width
				//	 , realrect->y, _this->real_height );
				realrect->width = 0;
			   realrect->height = 0;
			   return NULL;
			}
		}
	}
	if( realrect )
	{
	// add the real parent's position to the rectangle
	// _this will return absolute managed coordinates from 
	// upper left of real display surface.
		realrect->x += _this->real_x;
		realrect->y += _this->real_y;
	}
	return _this;
}

//-------------------------------------------------------------------------

#if 0
LOGICAL IntersectRectangles( P_IMAGE_RECTANGLE result, P_IMAGE_RECTANGLE pdr1, P_IMAGE_RECTANGLE pdr2 )
{
	if( !result )
		return FALSE;
	if( !pdr1 )
	{
		if( pdr2 )
			*result = *pdr2;
		else
			return FALSE;
	}
	else if( !pdr2 )
	{
		if( pdr1 )
			*result = *pdr1;
	}
	else
	{
		if( ( pdr1->x > ( pdr2->x + pdr2->width ) ) 
		  ||( pdr1->y > ( pdr2->y + pdr2->height ) ) 
        ||( ( pdr1->x + pdr1->width ) < pdr2->x ) 
        ||( ( pdr1->y + pdr1->height ) < pdr2->y ) ) 
			return FALSE;
		*result = *pdr1;
		if( result->x < pdr2->x )
		{
			result->width -= pdr2->x - pdr1->x;
			result->x = pdr2->x;
		}
		if( result->y < pdr2->y )
		{
			result->height -= pdr2->y - pdr1->y;
			result->y = pdr2->y;
		}
		if( ( result->x + result->width ) > ( pdr2->x + pdr2->width ) )
			result->width = pdr2->x + pdr2->width - result->x;
		if( ( result->y + result->height ) > ( pdr2->y + pdr2->height ) )
			result->height = pdr2->y + pdr2->height - result->y;
	}
	//Log4( WIDE("Result of %d,%d,%d,%d"), pdr1->x, pdr1->y, pdr1->width, pdr1->height );
	//Log4( WIDE("And       %d,%d,%d,%d"), pdr2->x, pdr2->y, pdr2->width, pdr2->height );
	//Log4( WIDE("Is        %d,%d,%d,%d"), result->x, result->y, result->width, result->height );
	return TRUE;	
}
#endif
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//  R.I.P.  Here lies the stubbed code to the image library to include
// panel clipping.
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//#define while( LoopHeader( image ) )(image) while( LoopHeader( image ) )

//	      Log8( WIDE("Bound to be : (%d,%d) - (%d,%d)  in (%d,%d)-(%d,%d)")
//	          , noderect.x, noderect.y, noderect.x + noderect.width, noderect.y + noderect.height
//	          , realrect.x, realrect.y, realrect.x + realrect.width, realrect.y + realrect.height );

#define LOOP_TRAILER  	   ; } 	\
	LeaveCriticalSec( &g.csSpaceRoot ); \
	FixImagePosition( image );

#define LOOP_HEADER(i) { \
	int result; \
	void *finddata = NULL; \
	EnterCriticalSec( &g.csSpaceRoot );   \
	while( ( result = LoopHeader( i, &finddata ) ) ) \
	   if( result < 0 ){ continue; } else

#define MY_ALPHA 32
//__inline
static int LoopHeaderEx( Image image, void **finddata DBG_PASS )
#define LoopHeader(i,fd) LoopHeaderEx( i,fd DBG_SRC )
{
#define DBG_LOOP_RELAY DBG_SRC
//#define DBG_LOOP_RELAY DBG_RELAY
   static CRITICALSECTION cs;
	static PSPACENODE found;
	static Image root;
	//SPACEPOINT min, max;
	//static void *finddata;
	static IMAGE_RECTANGLE realrect;
	IMAGE_RECTANGLE noderect;
   EnterCriticalSec( &cs );
#ifdef DIRTY_RECT_DEBUG
	_xlprintf( 1 DBG_LOOP_RELAY )( WIDE("--------------- Iterating... %p"), found  );
#endif
	if( !found )
	{
      //Log1( DBG_FILELINEFMT "Nothing found - setting find..." DBG_LOOP_RELAY,0 );
		if( !(root = FindImageRoot( image, &realrect )) )
		{
			LeaveCriticalSec( &cs );
			return 0;
		}
		noderect.width = ( noderect.x = realrect.x ) + realrect.width - 1;
		noderect.height = ( noderect.y = realrect.y ) + realrect.height - 1;
												 //Log( WIDE("Building find list...") );
 
		if( !g.pSpaceRoot )
		{
         lprintf( WIDE("This should never really happen... but if it does... maybe we're gracefully exiting?") );
			//DebugBreak();
			LeaveCriticalSec( &cs );
			return 0;
		}
		found = FindRectInSpaceEx( g.pSpaceRoot
										 , (P_IMAGE_POINT)&noderect.x
										 , (P_IMAGE_POINT)&noderect.width
										 , finddata
										  DBG_LOOP_RELAY);
	}
	else
	{
		noderect.width = ( noderect.x = realrect.x ) + realrect.width - 1;
		noderect.height = ( noderect.y = realrect.y ) + realrect.height - 1;
		found = FindRectInSpaceEx( NULL
										 , (P_IMAGE_POINT)&noderect.x
										 , (P_IMAGE_POINT)&noderect.width
										 , finddata
										  DBG_LOOP_RELAY);
	}
	for( ; found;
		 ( noderect.width = ( ( noderect.x = realrect.x ) + realrect.width - 1 ) )
		 , ( noderect.height = ( ( noderect.y = realrect.y ) + realrect.height - 1 ) )
		 , found = FindRectInSpaceEx( NULL
										  , (P_IMAGE_POINT)&noderect.x, (P_IMAGE_POINT)&noderect.width
										  , finddata
											DBG_LOOP_RELAY)
		)
	{
		//lprintf( WIDE("Had at least one found... %p"), found );
		IMAGE_RECTANGLE rect, imagerect;
		PPANEL thispanel = (PPANEL)GetNodeData( found );
#ifdef DIRTY_RECT_DEBUG
		Log1( WIDE("Found; %p"), found );
#endif
		rect = noderect;
		noderect.width  -= noderect.x - 1;
		noderect.height -= noderect.y - 1;
		if( thispanel->common.RealImage == root )
		{
		// mark it as potentiallyl dirty...
		// and the node is within the area on panel which is dirty...
        // rect = noderect;
#ifdef DIRTY_RECT_DEBUG
			{
				SPACEPOINT min, max;
				min[0] = noderect.x;
				min[1] = noderect.y;
				max[0] = noderect.width + noderect.x;
				max[1] = noderect.height + noderect.y;
				//DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_CYAN, MY_ALPHA/2 ) );
			}
#endif
			GetImageAuxRect( root, &imagerect );
			if( thispanel->common.flags.dirty_rect_valid )
			{
				imagerect = thispanel->common.dirty;
				//noderect.width  -= noderect.x - 1;
				// height = max-(min-1) max+ -(min - 1)
				//noderect.height -= noderect.y - 1;
				//GetImageAuxRect( root, &imagerect );
							 //#if 0
#ifdef DIRTY_RECT_DEBUG
					{
						SPACEPOINT min, max;
						min[0] = thispanel->common.dirty.x;
						min[1] = thispanel->common.dirty.y;
						max[0] = thispanel->common.dirty.width + thispanel->common.dirty.x;
						max[1] = thispanel->common.dirty.height + thispanel->common.dirty.y;
                  //if( image == g.SoftSurface )
						//	DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_BLUE, MY_ALPHA ) );
					}
					{
						SPACEPOINT min, max;
						min[0] = noderect.x;
						min[1] = noderect.y;
						max[0] = noderect.width + noderect.x;
                  max[1] = noderect.height + noderect.y;
                  //DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_YELLOW, MY_ALPHA ) );
					}
#endif
//#endif
				if( !IntersectRectangle( &rect, &thispanel->common.dirty, &noderect ) )
				{
#ifdef DIRTY_RECT_DEBUG
					lprintf( WIDE("Node NOT intersects... (%d,%d)-(%d,%d)  (%d,%d)-(%d,%d)  = (%d,%d)-(%d,%d)")
                       , noderect.x
                       , noderect.y
                       , noderect.width
                       , noderect.height
                       , thispanel->common.dirty.x
                       , thispanel->common.dirty.y
                       , thispanel->common.dirty.width
							 , thispanel->common.dirty.height
                       , rect.x
                       , rect.y
                       , rect.width
                       , rect.height
							 );
					//lprintf( WIDE("Node is outside the box.  Ignoring.") );
					{
						SPACEPOINT min, max;
						min[0] = noderect.x;
						min[1] = noderect.y;
						max[0] = noderect.width + rect.x;
                  max[1] = noderect.height + rect.y;
                  //DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_GREEN, MY_ALPHA + 32 ) );
					}
#endif
               continue;
				}
				else
				{
//#if 0
#ifdef DIRTY_RECT_DEBUG
					{
						SPACEPOINT min, max;
						min[0] = noderect.x;
						min[1] = noderect.y;
						max[0] = noderect.width + noderect.x;
                  max[1] = noderect.height + noderect.y;
                  //DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_BLUE, MY_ALPHA ) );
					}
					{
						SPACEPOINT min, max;
						min[0] = thispanel->common.dirty.x;
						min[1] = thispanel->common.dirty.y;
						max[0] = thispanel->common.dirty.width + thispanel->common.dirty.x;
                  max[1] = thispanel->common.dirty.height + thispanel->common.dirty.y;
                  //DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_RED, MY_ALPHA ) );
					}
#endif
#ifdef DIRTY_RECT_DEBUG

					{
						SPACEPOINT min, max;
						min[0] = rect.x;
						min[1] = rect.y;
						max[0] = rect.width + rect.x;
                  max[1] = rect.height + rect.y;
                  //DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_GREEN, MY_ALPHA ) );
					}
#endif
//#endif
#ifdef DIRTY_RECT_DEBUG
								 //#if 0
					lprintf( WIDE("Node intersects... (%d,%d)-(%d,%d)  (%d,%d)-(%d,%d)  = (%d,%d)-(%d,%d)")
                       , noderect.x
                       , noderect.y
                       , noderect.width
                       , noderect.height
                       , thispanel->common.dirty.x
                       , thispanel->common.dirty.y
                       , thispanel->common.dirty.width
							 , thispanel->common.dirty.height
                       , rect.x
                       , rect.y
                       , rect.width
                       , rect.height
							 );
#endif
//#endif
				}
			}
#ifdef DIRTY_RECT_DEBUG
			else
			{
				//GetImageAuxRect( root, &imagerect );
			//GetImageAuxRect( root, &rect );
            rect = noderect;
				{
					SPACEPOINT min, max;
					min[0] = rect.x;
					min[1] = rect.y;
					max[0] = rect.width + rect.x;
					max[1] = rect.height + rect.y;
					DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_RED, MY_ALPHA ) );
				}
				lprintf( WIDE("No dirty rect found on thispanel->") );
 			}
#endif
#ifdef DIRTY_RECT_DEBUG
         lprintf( WIDE("Marking node dirty.") );
				{
					SPACEPOINT min, max;
					min[0] = rect.x;
					min[1] = rect.y;
					max[0] = rect.width + rect.x;
					max[1] = rect.height + rect.y;
					//DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_RED, MY_ALPHA * 2 ) );
				}
#endif
			//MarkNodeDirty( found, &rect );
         imagerect = rect;
         // imagerect is the dirty rect if it was diryt...
			if( IntersectRectangle( &rect, &noderect, &imagerect ) )
			{
#ifdef DIRTY_RECT_DEBUG
					{
						SPACEPOINT min, max;
						min[0] = rect.x;
						min[1] = rect.y;
						max[0] = rect.width + rect.x;
                  max[1] = rect.height + rect.y;
                  //DrawSpace( NULL, min, max, SetAlpha( BASE_COLOR_GREEN, MY_ALPHA ) );
					}				//lprintf( WIDE("result bound... (%d,%d)-(%d,%d)"), rect.x, rect.y, rect.width, rect.height );
#endif
				MarkNodeDirty( found, &rect );
            rect.x -= realrect.x;
            rect.y -= realrect.y;
				SetImageBound( image, &rect );
				//lprintf( DBG_FILELINEFMT "Resulting NOW %ld %ld %ld %ld" DBG_LOOP_RELAY
				//		, rect.x
				//		, rect.y
				//		, rect.width
				//		, rect.height
				//		 );
				// lprintf( WIDE("This is the key to drawing.  If you continue, no application drawing will be ddone.") );
            //continue;
				LeaveCriticalSec( &cs );
#ifdef DIRTY_RECT_DEBUG
				lprintf( "marked node dirty, allowing draw into region" );
#endif
				return 1;
			}
			else
			{
#ifdef DIRTY_RECT_DEBUG
				lprintf( WIDE("Intersection failed: %p (%d,%d) -(%d,%d)  (%d,%d)-(%d,%d)")
                    , image
						  , noderect.x, noderect.y, noderect.width, noderect.height
						 , imagerect.x, imagerect.y, imagerect.width, imagerect.height );
#endif
				LeaveCriticalSec( &cs );
            lprintf( "found region, but failing draw..." );
				return -1;
			}
		}
#ifdef DIRTY_RECT_DEBUG
		else
		{
         lprintf( WIDE("thispanel->common.RealImage %p != %p"), thispanel->common.RealImage, root );
		}
#endif
		//Log1( WIDE("Found; %p"), found );
	}
	LeaveCriticalSec( &cs );
	return 0;
}

void DisplayPlot ( Image image, S_32 x, S_32 y, CDATA c )
{
   IMAGE_RECTANGLE rect;
	SPACEPOINT p;
	PSPACENODE found;
	Image root;
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      (*g.ImageInterface->_plot)( image, x, y, c );
      return;
	}
	root = FindImageRoot( image, &rect );
	p[0] = x + rect.x;
	p[1] = y + rect.y;
   // let's just patch this here for a moment...
  	//plot( image, x, y, c );
	if( ( found = FindPointInSpace( g.pSpaceRoot, p, NULL ) ) )
	{
		// first panel found, with this point in it.
		// any other panel we'd have to have allowed this point
		// to pass through - thereby calling the background function.
		PPANEL panel = (PPANEL)GetNodeData( found );
		if( panel->common.RealImage == root )
			(*g.ImageInterface->_plot)( image, x, y, c );
	}
}

void  CPROC (*pDisplayPlot)( Image image, S_32 x, S_32 y, CDATA c ) = DisplayPlot;

//-------------------------------------------------------------------------
 CDATA  DisplayGetPixel ( Image image, S_32 x, S_32 y )
{
   IMAGE_RECTANGLE rect;
	SPACEPOINT p;
	PSPACENODE found;
	Image root;
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      return (*g.ImageInterface->_getpixel)( image, x, y );
	}
	root = FindImageRoot( image, &rect );
	p[0] = x + rect.x;
	p[1] = y + rect.y;
	while( ( found = FindPointInSpace( g.pSpaceRoot, p, NULL ) ) )
	{
		// first panel found, with this point in it.
		// any other panel we'd have to have allowed this point
		// to pass through - thereby calling the background function.
		PPANEL panel = (PPANEL)GetNodeData( found );
		if( panel->common.RealImage == root )
		{
			return (*g.ImageInterface->_getpixel)( image, x, y );
		}
	}
   return 0; // this could be considered an error - 
             // a REAL black pixel would have alpha >0 therefore not be 0.
}

CDATA CPROC  (*pDisplayGetPixel)( Image image, S_32 x, S_32 y ) = DisplayGetPixel;
//-------------------------------------------------------------------------

 void  DisplayPlotAlpha ( Image image, S_32 x, S_32 y, CDATA c )
{
	SPACEPOINT p;
	PSPACENODE found;
	Image root;
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		(*g.ImageInterface->_plotalpha)( image, x, y, c );
		return;
	}
	root = FindImageRoot( image, NULL );
	p[0] = x + root->x;
	p[1] = y + root->y;
	found = g.pSpaceRoot;
	while( ( found = FindPointInSpace( found, p, NULL ) ) )
	{
		// first panel found, with this point in it.
		// any other panel we'd have to have allowed this point
		// to pass through - thereby calling the background function.
		PPANEL panel = (PPANEL)GetNodeData( found );
		if( panel->common.RealImage == root )
		{
			(*g.ImageInterface->_plotalpha)( image, x, y, c );
			break;
		}
		break;
	}
}

void  CPROC (*pDisplayPlotAlpha)( Image image, S_32 x, S_32 y, CDATA c ) = DisplayPlotAlpha;

//-------------------------------------------------------------------------

 void  DisplayLine ( Image image, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		(*g.ImageInterface->_do_line)( image, x, y, xto, yto, color );
		return;
	}

	LOOP_HEADER( image )
			(*g.ImageInterface->_do_line)( image
						, x
						, y
						, xto
						, yto, color );
	LOOP_TRAILER
}

void  CPROC (*pDisplayLine)( Image image, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA c ) = DisplayLine;
//-------------------------------------------------------------------------

 void  DisplayLineV ( Image image, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color, void(*proc)(Image Image, S_32 x, S_32 y, _32 d ) )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		//do_lineExV( image, x, y, xto, yto, color, proc );
		return;
	}

	LOOP_HEADER( image )
		//do_lineExV( image, x, y
		//			 , xto, yto, color, proc );
	LOOP_TRAILER
}

void  CPROC (*pDisplayLineV)( Image image, S_32 x, S_32 y
										, S_32 xto, S_32 yto
										, CDATA c
										, void(*proc)(Image Image, S_32 x, S_32 y, _32 d ) ) = DisplayLineV;
//-------------------------------------------------------------------------

 void  DisplayLineAlpha ( Image image, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      do_lineAlpha( image, x, y, xto, yto, color );
      return;
	}

	LOOP_HEADER( image )
		do_lineAlpha( image, x, y, xto, yto, color );
	LOOP_TRAILER
}

void  CPROC (*pDisplayLineAlpha)( Image image, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA c ) = DisplayLineAlpha;
//-------------------------------------------------------------------------

 void  DisplayHLine ( Image image, S_32 y, S_32 xfrom, S_32 xto, CDATA color )
{
	if( !image )
		return;
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      //Log4( WIDE("Do hline on bare %p at %d (%d-%d)"), image,  y, xfrom, xto );
      (*g.ImageInterface->_do_hline)( image, y, xfrom, xto, color );
      return;
	}

	LOOP_HEADER( image )
	{
      //Log4( WIDE("Do hline on %p at %d (%d-%d)"), image,  y, xfrom, xto );
		(*g.ImageInterface->_do_hline)( image, y, xfrom, xto, color );
	}
	LOOP_TRAILER
}

void  CPROC (*pDisplayHLine)( Image image, S_32 x, S_32 y, S_32 yto, CDATA c ) = DisplayHLine;
//-------------------------------------------------------------------------

 void  DisplayHLineAlpha ( Image image, S_32 y, S_32 xfrom, S_32 xto, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      (*g.ImageInterface->_do_hlineAlpha)( image, y, xfrom, xto, color );
      return;
	}

	LOOP_HEADER( image )
		(*g.ImageInterface->_do_hlineAlpha)( image, y, xfrom, xto, color );
	LOOP_TRAILER
}

void  CPROC (*pDisplayHLineAlpha)( Image image, S_32 x, S_32 y, S_32 yto, CDATA c ) = DisplayHLineAlpha;
//-------------------------------------------------------------------------

 void  DisplayVLine ( Image image, S_32 x, S_32 yfrom, S_32 yto, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      lprintf( WIDE("DisplayVLine Natural image - no bound check.") );
      (*g.ImageInterface->_do_vline)( image, x, yfrom, yto, color );
      return;
	}

		//lprintf( WIDE("DisplayVLine Panel image") );
	LOOP_HEADER( image )
	{
		//lprintf( WIDE("DisplayVLine Panel image - bound check(found).") );
		(*g.ImageInterface->_do_vline)( image, x, yfrom, yto, color );
	}
	LOOP_TRAILER
}

void  CPROC (*pDisplayVLine)( Image image, S_32 x, S_32 yfrom, S_32 yto, CDATA c ) = DisplayVLine;
//-------------------------------------------------------------------------

 void  DisplayVLineAlpha ( Image image, S_32 x, S_32 yfrom, S_32 yto, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      (*g.ImageInterface->_do_vlineAlpha)( image, x, yfrom, yto, color );
      return;
	}

	LOOP_HEADER( image )
		(*g.ImageInterface->_do_vlineAlpha)( image, x, yfrom, yto, color );
	LOOP_TRAILER
}

void  CPROC (*pDisplayVLineAlpha)( Image image, S_32 x, S_32 y, S_32 yto, CDATA c ) = DisplayVLineAlpha;
//-------------------------------------------------------------------------


 void  DisplayBlatColor ( Image image, S_32 x, S_32 y, _32 w, _32 h, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      //lprintf( WIDE("natural image.") );
      BlatColor( image, x, y, w, h, color );
      return;
	}
   //lprintf( WIDE("blatcolor...") );
	LOOP_HEADER( image )
	{
      //lprintf( WIDE("Uhmm blatsection %d,%d %d,%d"), x, y, w, h );
		BlatColor( image, x, y, w, h, color );
	}
	LOOP_TRAILER
}

 void  DisplayBlatColorAlpha ( Image image, S_32 x, S_32 y, _32 w, _32 h, CDATA color )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
      BlatColorAlpha( image, x, y, w, h, color );
      return;
	}

	LOOP_HEADER( image )
		BlatColorAlpha( image, x, y, w, h, color );
	LOOP_TRAILER
}

 void  DisplayBlotImageEx ( Image dest
						, Image image
						, S_32 xd, S_32 yd
						, _32 transparency
						, _32 method
						, ... )
{
	CDATA r,_g,b;
	va_list colors;
	va_start( colors, method );
	r = va_arg( colors, CDATA );
	_g = va_arg( colors, CDATA );
	b = va_arg( colors, CDATA );
	if( !( dest->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		BlotImageEx( dest, image, xd, yd, transparency, method
					  ,r,_g,b
					  );
      return;
	}

	LOOP_HEADER( dest )
		BlotImageEx( dest, image
					  , xd, yd
					  , transparency
					  , method
					  , r,_g,b
					  );
	LOOP_TRAILER
}

//-------------------------------------------------------------------------

 void  DisplayBlotImageSizedEx ( Image dest
						, Image image
						, S_32 xd, S_32 yd
						, S_32 xs, S_32 ys, _32 ws, _32 hs
						, _32 transparency
						, _32 method
						, ... )
{
	CDATA r,_g,b;
	va_list colors;
	va_start( colors, method );
	r = va_arg( colors, CDATA );
	_g = va_arg( colors, CDATA );
	b = va_arg( colors, CDATA );
					  
	if( !( dest->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		BlotImageSizedEx( dest, image, xd, yd, xs, ys, ws, hs, transparency, method
							 , r,_g,b
							 );
		return;
	}
   //Log( WIDE("Doing update in portions...") );
	LOOP_HEADER( dest )
	{
		BlotImageSizedEx( dest, image
							 , xd, yd
							 , xs, ys
							 , ws, hs
							 , transparency
							 , method
							 , r,_g,b
							 );
	}
	LOOP_TRAILER
}

//-------------------------------------------------------------------------

 void  DisplayBlotScaledImageSizedEx ( Image dest
						, Image image
						, S_32 xd, S_32 yd, _32 wd, _32 hd
						, S_32 xs, S_32 ys, _32 ws, _32 hs
						, _32 transparency
						, _32 method
						, ... )
{
	CDATA r,_g,b;
	va_list colors;
	va_start( colors, method );
	r = va_arg( colors, CDATA );
	_g = va_arg( colors, CDATA );
	b = va_arg( colors, CDATA );
	//lprintf( WIDE("%p has panel flag: %d"), dest, dest->flags & IF_FLAG_IS_PANEL );
	if( !( dest->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		//lprintf( WIDE("(bare)BlotScaledImageSized %p to %p  (%d,%d)-(%d,%d) too (%d,%d)-(%d,%d)")
		//	  , image, dest, xs, ys, xs+ws, ys+hs, xd, yd, xd+wd, yd+hd );

		BlotScaledImageSizedEx( dest, image
									 , xd, yd
									 , wd, hd
									 , xs, ys
									 , ws, hs, transparency, method
									 ,r,_g,b
									 );
      return;
	}
	//lprintf( WIDE("Therefore try to do this in sections...") );
   LOOP_HEADER( dest )
	{
		//lprintf( WIDE("(panl)BlotScaledImageSized %p to %p  (%d,%d)-(%d,%d) too (%d,%d)-(%d,%d)")
		//		 , image, dest, xs, ys, xs+ws, ys+hs, xd, yd, xd+wd, yd+hd );
		BlotScaledImageSizedEx( dest, image
									 , xd, yd
									 , wd, hd
									 , xs, ys
									 , ws, hs
									 , transparency, method
                            ,r,_g,b
									  );
	}
	LOOP_TRAILER
}

//-------------------------------------------------------------------------

 void  DisplayPutCharacterFont			( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, TEXTCHAR c, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutCharacterFont( image, x, y, fore, back, c, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutCharacterFont( image, x, y, fore, back, c, font );
		LOOP_TRAILER
	}
	return;
}

//-------------------------------------------------------------------------

 void  DisplayPutCharacterVerticalFont	( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, TEXTCHAR c, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutCharacterVerticalFont( image, x, y, fore, back, c, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutCharacterVerticalFont( image, x, y, fore, back, c, font );
		LOOP_TRAILER;
	}
	return; // should return string length along character span
}

//-------------------------------------------------------------------------

 void  DisplayPutCharacterInvertFont			( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, TEXTCHAR c, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutCharacterInvertFont( image, x, y, fore, back, c, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutCharacterInvertFont( image, x, y, fore, back, c, font );
		LOOP_TRAILER;
	}
	return; // should return string length along character span
}

//-------------------------------------------------------------------------

 void  DisplayPutCharacterVerticalInvertFont	( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, TEXTCHAR c, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutCharacterVerticalInvertFont( image, x, y, fore, back, c, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutCharacterVerticalInvertFont( image, x, y, fore, back, c, font );
		LOOP_TRAILER;
	}

	return; // should return string length along character span
}

//-------------------------------------------------------------------------

 void  DisplayPutStringFontEx				( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, CTEXTSTR pc, _32 nLen, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutStringFontEx( image, x, y, fore, back, pc, nLen, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutStringFontEx( image, x, y, fore, back, pc, nLen, font );
		LOOP_TRAILER;
	}
	return; // should return string length along character span
}

//-------------------------------------------------------------------------

 void  DisplayPutStringVerticalFontEx	( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, CTEXTSTR pc, _32 nLen, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutStringVerticalFontEx( image, x, y, fore, back, pc, nLen, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutStringVerticalFontEx( image, x, y, fore, back, pc, nLen, font );
		LOOP_TRAILER;
	}
	return; // should return string length along character span
}

//-------------------------------------------------------------------------

 void  DisplayPutStringInvertFontEx				( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, CTEXTSTR pc, _32 nLen, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		PutStringInvertFontEx( image, x, y, fore, back, pc, nLen, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutStringInvertFontEx( image, x, y, fore, back, pc, nLen, font );
		LOOP_TRAILER;
	}
	return; // should return string length along character span
}

//-------------------------------------------------------------------------

 void  DisplayPutStringInvertVerticalFontEx	( Image image, S_32 x, S_32 y, CDATA fore, CDATA back, CTEXTSTR pc, _32 nLen, Font font )
{
	if( !( image->flags & IF_FLAG_IS_PANEL ) )
	{ // natural images can use the natural functions
		//return
			PutStringInvertVerticalFontEx( image, x, y, fore, back, pc, nLen, font );
	}
	else
	{
		LOOP_HEADER( image )
			PutStringInvertVerticalFontEx( image, x, y, fore, back, pc, nLen, font );
		LOOP_TRAILER;
	}
	return; // should return string length along character span
}


//-------------------------------------------------------------------------

 void  DisplaySetImageBound ( Image pImage, P_IMAGE_RECTANGLE bound )
{
   //lprintf( WIDE("Setting boundary (aux image %p rect)=(%d,%d)-(%d,%d)"), pImage, bound->x, bound->y, bound->width, bound->height );
   SetImageAuxRect( pImage, bound );
   SetImageBound( pImage, bound );
}

//-------------------------------------------------------------------------

 void  DisplayFixImagePosition ( Image pImage )
{
	// Passing the first part of the image as a rectangle
   // will be the true, maximum image rectangle....
	SetImageAuxRect( pImage, (P_IMAGE_RECTANGLE)pImage );
   FixImagePosition( pImage );
}

//-------------------------------------------------------------------------

static void SetImageFlag( Image image )
{
	while( image )
	{
      image->flags |= IF_FLAG_IS_PANEL;
		SetImageFlag( image->pChild );
      image = image->pElder;
	}
}

//-------------------------------------------------------------------------

static void ClearImageFlag( Image image )
{
	while( image )
	{
      image->flags &= ~IF_FLAG_IS_PANEL;
		SetImageFlag( image->pChild );
      image = image->pElder;
	}
}

//-------------------------------------------------------------------------

 void  DisplayAdoptSubImage ( Image pFoster, Image pOrphan )
{
	AdoptSubImage( pFoster, pOrphan );
	if( pFoster->flags & IF_FLAG_IS_PANEL )
      SetImageFlag( pOrphan );
}

//-------------------------------------------------------------------------

 void  DisplayOrphanSubImage ( Image pImage )
{
	OrphanSubImage( pImage );
	if( pImage->flags & IF_FLAG_IS_PANEL )
      ClearImageFlag( pImage );
}


//-------------------------------------------------------------------------

void DoNothing( void )
{
}

//-------------------------------------------------------------------------

static IMAGE_INTERFACE DisplayImageInterface = {
    // these should all point directly
	// to the image library from whence they came.
	(void CPROC (*)(Image, _32))DoNothing
   , NULL //SetBlotMethod
	, NULL //BuildImageFileEx
	, NULL //MakeImageFileEx
	, DisplayMakeSubImageEx
   , NULL //RemakeImageEx
   , NULL //LoadImageFileEx
   , NULL //UnmakeImageFileEx
   , DisplaySetImageBound //SetImageBound
   , DisplayFixImagePosition
	, NULL //ResizeImageEx
	, NULL //MoveImage
	 // these are the drawing methods - subject to clipping
	 // by this library.
	, DisplayBlatColor
	, DisplayBlatColorAlpha                   
	, DisplayBlotImageEx                      
	, DisplayBlotImageSizedEx
   , DisplayBlotScaledImageSizedEx

	, &pDisplayPlot
   , &pDisplayPlotAlpha
   , &pDisplayGetPixel

	,&pDisplayLine      
	//,&pDisplayLineV
	,&pDisplayLineAlpha 
	,&pDisplayHLine     
	,&pDisplayVLine     
	,&pDisplayHLineAlpha
   ,&pDisplayVLineAlpha

   ,NULL //GetDefaultFont                           [5~
   ,NULL //GetFontHeight
   ,NULL //GetStringSizeFontEx

	,DisplayPutCharacterFont                 
	,DisplayPutCharacterVerticalFont         
	,DisplayPutCharacterInvertFont            
   ,DisplayPutCharacterVerticalInvertFont

	,DisplayPutStringFontEx                  
	,DisplayPutStringVerticalFontEx          
	,DisplayPutStringInvertFontEx            
   ,DisplayPutStringInvertVerticalFontEx
   /* these from here down are filled in by GetInterface */
   ,NULL //GetMaxStringLengthFont
   ,NULL //GetImageSize
   ,NULL
};

#undef DropImageInterface
#undef GetImageInterface

// otherwise getimageinterface below will be wrong...
static POINTER CPROC _DisplayGetImageInterface( void )
{
	InitDisplay();
   DisplayImageInterface._SetBlotMethod        = SetBlotMethod;
	DisplayImageInterface._BuildImageFileEx	  = BuildImageFileEx;
	DisplayImageInterface._MakeImageFileEx	     = MakeImageFileEx;
	//DisplayImageInterface._MakeSubImageEx	     = MakeSubImageEx;
   DisplayImageInterface._RemakeImageEx		  = RemakeImageEx;
   DisplayImageInterface._LoadImageFileEx	     = LoadImageFileEx;
   DisplayImageInterface._UnmakeImageFileEx    = UnmakeImageFileEx;
   DisplayImageInterface._SetImageBound		  = DisplaySetImageBound;
   DisplayImageInterface._FixImagePosition	  = DisplayFixImagePosition;
	DisplayImageInterface._ResizeImageEx		  = ResizeImageEx;
	DisplayImageInterface._MoveImage			     = MoveImage;

	DisplayImageInterface._GetDefaultFont        = GetDefaultFont;
   DisplayImageInterface._GetFontHeight			= GetFontHeight;
	DisplayImageInterface._GetStringSizeFontEx	= GetStringSizeFontEx;

	DisplayImageInterface._GetMaxStringLengthFont = GetMaxStringLengthFont;
	DisplayImageInterface._GetImageSize           = GetImageSize;

   DisplayImageInterface._LoadFont = LoadFont;
   DisplayImageInterface._UnloadFont = UnloadFont;
   /* these really have no meaning unless client/server is in place */
	DisplayImageInterface._BeginTransferData = NULL;
	DisplayImageInterface._ContinueTransferData = NULL;
	DisplayImageInterface._DecodeTransferredImage = NULL;
	DisplayImageInterface._DecodeMemoryToImage = g.ImageInterface->_DecodeMemoryToImage;
	DisplayImageInterface._AcceptTransferredFont = NULL;

	DisplayImageInterface._ColorAverage = g.ImageInterface->_ColorAverage;
   DisplayImageInterface._IntersectRectangle = IntersectRectangle;
	DisplayImageInterface._GetImageAuxRect = GetImageAuxRect;
	DisplayImageInterface._SetImageAuxRect = SetImageAuxRect;
	DisplayImageInterface._OrphanSubImage = DisplayOrphanSubImage;
	DisplayImageInterface._AdoptSubImage = DisplayAdoptSubImage;
	DisplayImageInterface._MergeRectangle = MergeRectangle;
   DisplayImageInterface._GetImageAuxRect = GetImageAuxRect;
   DisplayImageInterface._SetImageAuxRect = SetImageAuxRect;
	DisplayImageInterface._GetImageSurface = GetImageSurface;
   DisplayImageInterface._InternalRenderFont     = InternalRenderFont;
	DisplayImageInterface._InternalRenderFontFile = InternalRenderFontFile;
	DisplayImageInterface._RenderScaledFontData         = RenderScaledFontData;
   DisplayImageInterface._RenderFontFileEx       = RenderFontFileEx;
   DisplayImageInterface._DestroyFont            = DestroyFont;
	DisplayImageInterface._GetFontRenderData      = GetFontRenderData;
	DisplayImageInterface._SetFontRendererData      = SetFontRendererData;
   DisplayImageInterface._global_font_data         = g.ImageInterface->_global_font_data;
   DisplayImageInterface._GetGlobalFonts         = g.ImageInterface->_GetGlobalFonts;

	DisplayImageInterface._MakeSpriteImageFileEx = g.ImageInterface->_MakeSpriteImageFileEx;
	DisplayImageInterface._MakeSpriteImageEx = g.ImageInterface->_MakeSpriteImageEx;
	DisplayImageInterface._rotate_scaled_sprite = g.ImageInterface->_rotate_scaled_sprite;
	DisplayImageInterface._rotate_sprite = g.ImageInterface->_rotate_sprite;
	DisplayImageInterface._BlotSprite = g.ImageInterface->_BlotSprite;
	DisplayImageInterface._SetSpriteHotspot = g.ImageInterface->_SetSpriteHotspot;
	DisplayImageInterface._SetSpritePosition = g.ImageInterface->_SetSpritePosition;
	DisplayImageInterface._DecodeMemoryToImage = g.ImageInterface->_DecodeMemoryToImage;




	//InitMemory();


	return (POINTER)&DisplayImageInterface;
}

 PIMAGE_INTERFACE  GetImageInteface(void )
{
   return (PIMAGE_INTERFACE)_DisplayGetImageInterface();
}

static void CPROC _DisplayDropImageInterface( POINTER p )
{
}

#undef DropImageInterface
 void  DropImageInterface ( PIMAGE_INTERFACE p )
{
// do stuff here methinks...
   _DisplayDropImageInterface( p );
}

#ifndef DISPLAY_SERVICE
PRELOAD( DisplayImageRegisterInterface )
{
   lprintf( WIDE("Registering display image interface") );
   RegisterInterface( "display_image", _DisplayGetImageInterface, _DisplayDropImageInterface );
}
#endif

void DoSetImagePanelFlag( Image first, Image parent, Image image )
{
	//lprintf( WIDE("Setting panel image flag on %p under %p (%d)")
	//		 , image, parent, parent->flags&IF_FLAG_IS_PANEL );
	if( parent->flags & IF_FLAG_IS_PANEL )
	{
		while( image )
		{
			//lprintf( WIDE("Setting panel image flag on %p under %p"), image, parent );
			image->flags |= IF_FLAG_IS_PANEL;
			if( image->pChild )
				DoSetImagePanelFlag( first, image, image->pChild );
         if( image != first )
				image = image->pElder;
			else
            break;
		}
	}
}

void DoClearImagePanelFlag( Image first, Image image )
{
	while( image )
	{
      //lprintf( WIDE("clearing panel image flag on %p"), image );
		image->flags &= ~IF_FLAG_IS_PANEL;
		if( image->pChild )
			DoClearImagePanelFlag( first, image->pChild );
      if( first != image )
			image = image->pElder;
		else
			break;
	}
}

void SetImagePanelFlag( Image parent, Image image )
{
   DoSetImagePanelFlag( image, parent, image );
}

void ClearImagePanelFlag( Image image )
{
   DoClearImagePanelFlag( image, image );
}

RENDER_NAMESPACE_END

//--------------------------------------------------------
// $Log: display_image.c,v $
// Revision 1.63  2005/06/30 13:00:20  d3x0r
// Massive performance updates...
//
// Revision 1.65  2005/06/24 15:11:46  jim
// Merge with branch DisplayPerformance, also a fix for watcom compilation
//
// Revision 1.64.2.2  2005/06/22 23:12:10  jim
// checkpoint...
//
// Revision 1.64.2.1  2005/06/22 17:31:43  jim
// Commit test optimzied display
//
// Revision 1.62  2005/06/28 18:27:45  d3x0r
// Added some blocking for debug disabling
//
// Revision 1.61  2005/06/19 05:15:39  d3x0r
// Add copying the panel flag bit to adopted images, and clearing of orphaned images... debugging still included...
//
// Revision 1.60  2005/06/17 21:29:14  d3x0r
// Checkpoint... Seems that dirty rects can be used to minmize drawing esp. around moving/rsizing of windows
//
// Revision 1.59  2005/05/31 21:57:00  d3x0r
// Progress optimizing redraws... still have several issues...
//
// Revision 1.58  2005/05/30 20:15:09  d3x0r
// Remove unused vars.
//
// Revision 1.57  2005/05/30 20:14:03  d3x0r
// Add tracked images - going to have to (i think) have server events come back to update image size/position, otherwise the application failes to get accurate information?... well - working out some redraw issues also.
//
// Revision 1.56  2005/05/30 12:31:31  d3x0r
// Cleanup some extra allocations...
//
// Revision 1.55  2005/05/25 16:50:12  d3x0r
// Synch with working repository.
//
// Revision 1.64  2005/05/23 22:30:00  jim
// minor fix for watcom.
//
// Revision 1.63  2005/05/23 21:57:57  jim
// Updates to newest message service.  Also abstraction of images to indexes instead of PIMAGE pointers in messages...
//
// Revision 1.62  2005/05/12 23:08:17  jim
// Remove much noisy logging.
//
// Revision 1.61  2005/05/12 21:04:42  jim
// Fixed several conflicts that resulted in various deadlocks.  Also endeavored to clean all warnings.
//
// Revision 1.60  2005/05/06 21:42:51  jim
// Didn't save Makefile...
//
// Revision 1.59  2005/03/30 11:36:36  panther
// Remove a lot of debugging messages...
//
// Revision 1.58  2005/03/16 22:28:26  panther
// Updates for extended interface definitions... Fixes some issues with client/server display usage...
//
// Revision 1.57  2005/03/14 16:57:20  panther
// On service registration - don't register natural interfaces?  double load if display.dll is part of interface.conf...
//
// Revision 1.56  2005/03/12 23:30:56  panther
// Fill in merge rect and set/get aux rect methods
//
// Revision 1.55  2005/01/18 10:48:19  panther
// Define image interface export so there's no conflict between image and display_image
//
// Revision 1.54  2005/01/18 03:49:05  panther
// register IsTopMost
//
// Revision 1.53  2004/10/25 10:39:58  d3x0r
// Linux compilation cleaning requirements...
//
// Revision 1.52  2004/08/17 01:17:32  d3x0r
// Looks like new drawing check code works very well.  Flaw dragging from top left to bottom right for root panel... can lose mouse ownership, working on this problem... fixed export of coloraverage
//
// Revision 1.51  2004/08/16 06:21:39  d3x0r
// Checkpoint - debugging region redraw...  need prioritized ordering...perhaps binary tree?
//
// Revision 1.50  2004/03/04 01:09:50  d3x0r
// Modifications to force slashes to wlib.  Updates to Interfaces to be gotten from common interface manager.
//
// Revision 1.49  2003/12/15 09:34:13  panther
// Need to fix spacetree tracking more - need to lockout more than one scanner at a time.
// Fixed issues with settting an image boundary and blotting
// images into said rect.
//
// Revision 1.48  2003/12/14 06:14:40  panther
// Fix BlotImageSized iteration, added logging, commented out
//
// Revision 1.47  2003/12/04 10:40:51  panther
// Add to sync a more definitive sync
//
// Revision 1.46  2003/11/22 23:40:32  panther
// Fix includes so there's no conflict with winuser.h
//
// Revision 1.45  2003/10/13 02:50:47  panther
// Font's don't seem to work - lots of logging added back in
// display does work - but only if 0,0 biased, cause the SDL layer sucks.
//
// Revision 1.44  2003/09/21 16:25:28  panther
// Removed much noisy logging, all in the interest of sheet controls.
// Fixed some linking of services.
// Fixed service close on dead client.
//
// Revision 1.43  2003/09/21 11:49:04  panther
// Fix service refernce counting for unload.  Fix Making sub images hidden.
// Fix Linking of services on server side....
// cleanup some comments...
//
// Revision 1.42  2003/09/15 17:06:37  panther
// Fixed to image, display, controls, support user defined clipping , nearly clearing correct portions of frame when clearing hotspots...
//
// Revision 1.41  2003/08/27 08:00:31  panther
// Fill in image interface table better
//
// Revision 1.40  2003/08/13 16:10:24  panther
// Fix debug transport of UpdateDisplayPortionEx
//
// Revision 1.39  2003/08/12 08:43:12  panther
// Fix loading of image, render service support
//
// Revision 1.38  2003/08/11 10:50:36  panther
// Repair build on windows? probably need to split library and build RENDER_INTERFACe, IMAGE_INTERFACE, both
//
// Revision 1.37  2003/08/01 07:56:12  panther
// Commit changes for logging...
//
// Revision 1.36  2003/07/24 22:48:29  panther
// Definitions to make watcom happy
//
// Revision 1.35  2003/06/24 11:44:37  panther
// Okay disable compiling service under windows... apparently compiles under windows now - not sure about usability
//
// Revision 1.34  2003/04/23 11:36:40  panther
// Multiple instances are not supported.  Global name space is not supported.  Revert all g-> to g.
//
// Revision 1.33  2003/03/31 04:18:49  panther
// Okay - drawing dispatch seems to work now.  Client/Server timeouts occr... rapid submenus still fail
//
// Revision 1.32  2003/03/31 02:07:05  panther
// fix returning root root images...
//
// Revision 1.31  2003/03/31 01:11:28  panther
// Tweaks to work better under service application
//
// Revision 1.30  2003/03/30 21:38:54  panther
// Fix MSG_ definitions.  Fix lack of anonymous unions
//
// Revision 1.29  2003/03/30 19:59:13  panther
// Don't send lose focus to closing display
//
// Revision 1.28  2003/03/30 06:25:44  panther
// Reordered some code... error checking added
//
// Revision 1.27  2003/03/29 22:52:00  panther
// New render/image layering ability.  Added support to Display for WIN32 usage (native not SDL)
//
// Revision 1.26  2003/03/23 14:56:00  panther
// handle setmouseposition, getdiisplayposition ...
//
// Revision 1.25  2003/02/23 22:44:58  panther
// Removed more heavy logging messages
//
// Revision 1.24  2003/02/23 18:25:47  panther
// Add font/image tracking on service side.  Removed noisy messages.
//
// Revision 1.23  2003/02/23 03:25:15  panther
// Added packed attribute - removed it Added back in discreetly.  Added logging option to memlib.  Added logging to msg services/server
//
// Revision 1.22  2003/02/21 22:12:02  panther
// Cleanup some warnings.
//
// Revision 1.21  2003/02/18 06:23:46  panther
// Improved key mappings - removed some logging
//
// Revision 1.20  2003/02/17 02:58:23  panther
// Changes include - better support for clipped images in displaylib.
// More events handled.
// Modifications to image structure and better unification of clipping
// ideology.
// Better definition of image and render interfaces.
// MUCH logging has been added and needs to be trimmed out.
//
// Revision 1.19  2003/02/11 07:56:04  panther
// Minor display patches/test program, server
//
// Revision 1.18  2003/02/10 01:23:53  panther
// Well - collect mouse and redraw events.  Collect inbound mouse evevents on service side
//
// Revision 1.17  2003/02/09 04:02:46  panther
// Client loss cleanup seems to work, All functions seem to be implmented.  Nice checkpoint
//
// Revision 1.16  2003/02/08 15:11:38  panther
// Begin setting callback methods client/server
//
// Revision 1.15  2003/02/07 13:58:56  panther
// Keep images and Renderers in a private structures on client side, send appropriate data internal
//
// Revision 1.14  2003/02/06 11:10:26  panther
// Migration to new interface headers
//
// Revision 1.13  2003/02/05 10:32:31  panther
// Client/Server code compiles...
//
// Revision 1.12  2002/12/12 15:01:44  panther
// Fix Image structure mods.
//
// Revision 1.11  2002/11/24 22:29:33  panther
// Now everything builds from clean under windows (VC, LCC).
//
// Revision 1.10  2002/11/24 21:37:41  panther
// Mods - network - fix server->accepted client method inheritance
// display - fix many things
// types - merge chagnes from verious places
// ping - make function result meaningful yes/no
// controls - fixes to handle lack of image structure
// display - fixes to handle moved image structure.
//
// Revision 1.9  2002/11/20 21:57:01  jim
// Modified all places which directly referenced ImageFile structure.
// Display library is still allowed intimate knowledge
//
// Revision 1.8  2002/11/19 01:35:27  panther
// Added GetImageSize method, since there is no real local information
// about the image, and like fonts, this could be cached locally.
// Cleaned up warnings and incompatible and missing routines in view.
//
// Revision 1.7  2002/11/18 22:03:01  panther
// Okay - updated to the latest image and video interfaces.
// Also - modified routines exported here to provide direct linking
// to related view libraries.
//
// Revision 1.6  2002/11/06 12:43:01  panther
// Updated display interface method, cleaned some code in the display image
// interface.  Have to establish who own's 'focus' and where windows
// are created.  The creation method REALLY needs the parent's window.  Which
// is a massive change (kinda)
//
// Revision 1.5  2002/11/05 09:55:55  panther
// Seems to work, added a sample configuration file.
// Depends on some changes in configscript.  Much changes accross the
// board to handle moving windows... now to display multiples.
//
// Revision 1.4  2002/10/30 15:51:01  panther
// Modified display to handle controls, Stripped out regions.  Project now
// works wonderfully on Windows (kinda) now we go to Linux and see about
// the input events there.
//
// Revision 1.3  2002/10/29 09:31:34  panther
// Trimmed out Carriage returns, added CVS Logging.
// Fixed compilation under Linux.
//
//
