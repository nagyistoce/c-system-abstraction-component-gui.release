//Requires a GUI environment, so if you are building with
//__NO_GUI__=1 this will never work.

#include <stdhdrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <systray.h>
#define USE_IMAGE_INTERFACE g.pii
#define USE_RENDER_INTERFACE g.pdi
#include <render.h>
#include <image.h>
#include <timers.h>

typedef struct global_tag {
	PIMAGE_INTERFACE pii;
	PRENDER_INTERFACE pdi;
	struct
	{
		BIT_FIELD bBlacking : 1;
	}flags;

	_32 tick_to_switch;

	int show_time; // how long to show the iamge after fading in.
   int fade_in; // how long fade out is..

   int nImages; // convenience counter of the length of images.
	PLIST images; // list of Image's

   int current_image; // overall counter, stepped appropriately.
	// each display has a current image...
   int next_up; // index of the display that is next to show
	int currents[2];
   int is_up[2];
	PRENDERER displays[2];
} GLOBAL;
static GLOBAL g;
Image imgGraphic;
//Image surface;

_32 width, height;
#define MAX_STEPS 16
int _ix[MAX_STEPS], _iy[MAX_STEPS];
int _n;
int ix, iy;
int dx, dy;
int wrapped ;
int not_first;



void CPROC Output( PTRSZVAL psv, PRENDERER display )
{
	if( g.is_up[psv] )
	{
		Image surface = GetDisplayImage( display );
		Image imgGraphic;
		imgGraphic = (Image)GetLink( &g.images, g.currents[psv] );
		BlotScaledImage( surface, imgGraphic );
		//lprintf( "Ouptut image to surface (once) %d", psv );
		UpdateDisplay( display );
	}
}

int target_out;
INDEX current_image;
int target_in;
int target_in_start;

void CPROC tick( PTRSZVAL psv )
{
	_32 now = GetTickCount();

	if( target_in_start )
	{
		if( now >= target_in_start )
		{
			if( !target_in )
			{
				target_in = target_in_start + g.fade_in;
            g.currents[g.next_up] = g.current_image;
				g.current_image++;
				if( g.current_image >= g.nImages )
					g.current_image = 0;
				//lprintf( "(!)Setting fade to..> %d", 255 - (255*(now-target_in_start))/(target_in-target_in_start) );
				SetDisplayFade( g.displays[g.next_up], 255 - (255*(now-target_in_start))/(target_in-target_in_start) );
            g.is_up[g.next_up] = 1;
				UpdateDisplay( g.displays[g.next_up] );
				//ForceDisplayFront( g.displays[g.next_up] );
			}
			else
			{
			if( target_in && now > target_in )
			{
				SetDisplayFade( g.displays[g.next_up], 0 );
				UpdateDisplay( g.displays[g.next_up] );
				target_in_start = now + g.show_time;
				target_in = 0;
				g.next_up++;
				if( g.next_up >= 2 ) // two displayss
					g.next_up = 0;

				// hide the old display...
				//lprintf( "hiding old display..." );
            g.is_up[g.next_up] = 0;
				HideDisplay( g.displays[g.next_up] );
			}
			else
			{

				//lprintf( "Setting fade to..> %d", 255 - (255*(now-target_in_start))/(target_in-target_in_start) );
				SetDisplayFade( g.displays[g.next_up], 255 - (255*(now-target_in_start))/(target_in-target_in_start) );
				UpdateDisplay( g.displays[g.next_up] );
			}
			}
		}
	}
}

int main( int argc, char **argv )
{
	int x = 0;
	int y = 0;
   _32 width, height;
   int w, h;
	g.pdi = GetDisplayInterface();
	g.pii = GetImageInterface();
   RegisterIcon( NULL );
	GetDisplaySize( &width, &height );
   w = width; h = height;


   y = x  = 0;

	{
      int state = 0;
		int arg;
		g.fade_in = 500;
      g.show_time = 1000;
		for( arg = 1; arg < argc; arg++ )
		{

			if( argv[arg][0] == '-' )
			{
				switch( state )
				{
				case 0:
					x = atoi( argv[arg]+1 );
               break;
				case 1:
					y = atoi( argv[arg]+1 );
               break;
				case 2:
					w = atoi( argv[arg]+1 );
               break;
				case 3:
					h = atoi( argv[arg]+1 );
               break;
				case 4:
					g.show_time = atoi( argv[arg]+1 );
               break;
				case 5:
					g.fade_in = atoi( argv[arg]+1 );
               break;
				}
            state++;
			}
			else
			{
				Image x = LoadImageFile( argv[arg] );
				if( x )
				{
					g.nImages++;
               AddLink( &g.images, x );
				}
			}
		}
	}

   if( g.nImages )
	{
		g.displays[0] = OpenDisplaySizedAt( DISPLAY_ATTRIBUTE_LAYERED|DISPLAY_ATTRIBUTE_CHILD|DISPLAY_ATTRIBUTE_NO_MOUSE|DISPLAY_ATTRIBUTE_NO_AUTO_FOCUS
														  , w //width
														  , h //height
														  , x //0
														  , y //0
											 );
		g.displays[1] = OpenDisplaySizedAt( DISPLAY_ATTRIBUTE_LAYERED|DISPLAY_ATTRIBUTE_CHILD|DISPLAY_ATTRIBUTE_NO_MOUSE|DISPLAY_ATTRIBUTE_NO_AUTO_FOCUS
														  , w //width
														  , h //height
														  , x //0
														  , y //0
											 );
		SetRedrawHandler( g.displays[0], Output, 0 );
		SetRedrawHandler( g.displays[1], Output, 1 );

      target_in_start = GetTickCount();
		AddTimer( 33, tick, 0 );

		while( 1 )
			WakeableSleep( 10000 );
	}
	else
	{
	}
   return 0;
}

