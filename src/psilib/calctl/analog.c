#define g global_calender_structure
#ifndef __cplusplus_cli
#define USE_IMAGE_INTERFACE (g.MyImageInterface?g.MyImageInterface:(g.MyImageInterface=GetImageInterface() ))
#define USE_RENDER_INTERFACE (g.MyDisplayInterface?g.MyDisplayInterface:(g.MyDisplayInterface=GetDisplayInterface() ))
#endif


#include <stdhdrs.h>
#include <sharemem.h>
#include <psi.h>
#include <psi/clock.h>
#include "local.h"

PSI_CLOCK_NAMESPACE 

#define CLOCK_NAME WIDE("Basic Clock Widget")
extern CONTROL_REGISTRATION clock_control;


struct analog_clock
{
	struct {
		_32 bLocked : 1;
	} flags;
   PRENDERER render;
	Image image;
	Image face;
   Image composite; // size of face, work space to add hands.  Clock face created always.
   _32 w, h;
	PSPRITE second_hand;
	PSPRITE minute_hand;
	PSPRITE hour_hand;
	struct {
		_32 xofs;
      _32 yofs;
	} face_center;
   PCLOCK_CONTROL clock;
};


void DrawClock( Image surface, PANALOG_CLOCK analog )
{
	{
		if( analog )
		{
         int remake = 0;
			if( analog->flags.bLocked )
				return;
			analog->flags.bLocked = 1;
			if( surface->height > analog->h )
			{
				remake = 1;
				analog->h = surface->height;
			}
			if( surface->width > analog->w )
			{
				remake = 1;
				analog->w = surface->width;
			}
			if( remake )
			{
				UnmakeImageFile( analog->composite );
            analog->composite = MakeImageFile( analog->w, analog->h );
			}
         ClearImageTo( analog->composite, 0 );
			//BlotScaledImageAlpha( surface, analog->composite, ALPHA_TRANSPARENT );
			BlotScaledImageSizedToAlpha( analog->composite, analog->face
												, 0, 0
												, surface->width, surface->height, ALPHA_TRANSPARENT );
         //BlotImage( analog->composite, analog->face, 0, 0);
			{
				//PANALOG_CLOCK analog = (PANALOG_CLOCK)psv;
				//Image surface = GetDisplayImage( renderer );
				rotate_scaled_sprite( analog->composite, analog->second_hand
										  , ( analog->clock->time_data.sc * 0x100000000LL ) / 60
										  + ( analog->clock->time_data.ms * 0x100000000LL ) / (1000*60)
										  , ( 0x10000 * surface->width ) / analog->face->width
										  , ( 0x10000 * surface->height ) / analog->face->height
										  );
				rotate_scaled_sprite( analog->composite, analog->minute_hand
										  , ( analog->clock->time_data.mn * 0x100000000LL ) / 60
											+ ( analog->clock->time_data.sc * 0x100000000LL ) / (60*60)
										  , ( 0x10000 * surface->width ) / analog->face->width
										  , ( 0x10000 * surface->height ) / analog->face->height
										  );
				rotate_scaled_sprite( analog->composite, analog->hour_hand
										  , ( analog->clock->time_data.hr * 0x100000000LL ) / 12
											+ ( analog->clock->time_data.mn * 0x100000000LL ) / (60*12)
										  , ( 0x10000 * surface->width ) / analog->face->width
										  , ( 0x10000 * surface->height ) / analog->face->height
										  );
			}
			//xlprintf(LOG_NOISE-1)( WIDE("Surface is %ld,%ld,%ld"), surface, surface->x, surface->y );
			BlotImage( surface, analog->composite, 0, 0 );
			//BlotImageSizedAlpha( surface, analog->composite, 0, 0, surface->width, surface->height, ALPHA_TRANSPARENT );
			//BlotScaledImageAlpha( surface, analog->composite, ALPHA_TRANSPARENT );
			analog->flags.bLocked = 0;
		}
	}
}

static void OnRevealCommon( CLOCK_NAME )( PSI_CONTROL pc )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, (PSI_CONTROL)pc );
	if( clock )
	{
		PANALOG_CLOCK analog = clock->analog_clock;
		if( analog )
		{
         RestoreDisplay( analog->render );
		}
	}
}

static void OnHideCommon( CLOCK_NAME )( PSI_CONTROL pc )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, (PSI_CONTROL)pc );
	if( clock )
	{
		PANALOG_CLOCK analog = clock->analog_clock;
		if( analog )
		{
         HideDisplay( analog->render );
		}
	}

}

static void MoveSurface( PSI_CONTROL pc )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, (PSI_CONTROL)pc );
	if( clock )
	{
		PANALOG_CLOCK analog = clock->analog_clock;
		if( analog )
		{
			Image surface = GetControlSurface( pc );
			PRENDERER r = GetFrameRenderer( GetFrame( pc ) );
			S_32 x = 0;
			S_32 y = 0;
			GetPhysicalCoordinate( pc, &x, &y, TRUE );
         MoveDisplay( analog->render, x, y );
		}
	}
}


static void OnMoveCommon( CLOCK_NAME )( PSI_CONTROL pc, LOGICAL changing )
{
	if( !changing )
      MoveSurface( pc );
}

static void OnMotionCommon( CLOCK_NAME )( PSI_CONTROL pc, LOGICAL changing )
{
	if( !changing )
      MoveSurface( pc );
}



static void OnSizeCommon( CLOCK_NAME )( PSI_CONTROL pc, LOGICAL changing )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, (PSI_CONTROL)pc );
	if( !changing && clock )
	{
		PANALOG_CLOCK analog = clock->analog_clock;
		if( analog )
		{
			Image surface = GetControlSurface( pc );
         SizeDisplay( analog->render, surface->width, surface->height );
		}
	}
}


void CPROC DrawClockLayers( PTRSZVAL psv, PRENDERER renderer )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, (PSI_CONTROL)psv );
	if( clock )
	{
      PANALOG_CLOCK analog = clock->analog_clock;
		Image surface = GetDisplayImage( renderer );
		DrawClock( surface, analog );
      UpdateDisplay( renderer );
	}

}

void DrawAnalogClock( PSI_CONTROL pc )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, pc );
	if( clock )
	{
		PANALOG_CLOCK analog = clock->analog_clock;
      Image surface = GetControlSurface( pc );
		//Redraw( analog->render );
      DrawClock( surface, analog );
	}
}
void MakeClockAnalogEx( PSI_CONTROL pc, CTEXTSTR imagename, struct clock_image_thing *description )
{
	ValidatedControlData( PCLOCK_CONTROL, clock_control.TypeID, clock, pc );
	if( clock )
	{
		PANALOG_CLOCK analog = clock->analog_clock;
		if( !analog )
		{
			if( !imagename )
            return; // no image? well then no analog clock for you.
			analog = (PANALOG_CLOCK)Allocate( sizeof( ANALOG_CLOCK ) );
			MemSet( analog,0,sizeof( ANALOG_CLOCK ) );
			analog->clock = clock;
			analog->image = LoadImageFile( imagename );
			if( !analog->image )
			{
				Release( analog );
            return;
			}
			analog->face = MakeSubImage( analog->image, 0, 0, 358,358 );
			analog->composite = MakeImageFile( 358, 358 );
			analog->w = 358;
			analog->h = 358;
			analog->second_hand = MakeSpriteImage( MakeSubImage( analog->image, 400-20, 0, 40, 358 ) );
			analog->minute_hand = MakeSpriteImage( MakeSubImage( analog->image, 400-20+40, 0, 40, 358 ) );
			analog->hour_hand = MakeSpriteImage( MakeSubImage( analog->image, 400-20+40+40, 0, 40, 358 ) );
			analog->face_center.xofs = 178;
			analog->face_center.yofs = 179;
			SetSpritePosition( analog->second_hand, analog->face_center.xofs, analog->face_center.yofs );
			SetSpritePosition( analog->minute_hand, analog->face_center.xofs, analog->face_center.yofs );
			SetSpritePosition( analog->hour_hand, analog->face_center.xofs, analog->face_center.yofs );
			SetSpriteHotspot( analog->second_hand, 20, 179 );
			SetSpriteHotspot( analog->minute_hand, 20, 179 );
			SetSpriteHotspot( analog->hour_hand, 20, 179 );

         if( 0 )
			{
				Image surface = GetControlSurface( pc );
				PRENDERER r = GetFrameRenderer( GetFrame( pc ) );
				S_32 x = 0;
				S_32 y = 0;
				GetPhysicalCoordinate( pc, &x, &y, FALSE );
	            lprintf( WIDE( "Making clock uhm... %d %d %d %d over %p" ), x, y,surface->width
																	 , surface->height );
				analog->render = OpenDisplayAboveSizedAt( DISPLAY_ATTRIBUTE_LAYERED
                                                     |DISPLAY_ATTRIBUTE_NO_MOUSE
																	  |DISPLAY_ATTRIBUTE_CHILD // mark that this is uhmm intended to not be a alt-tabbable window
																	 , surface->width
																	 , surface->height
																	 , x, y
																	 , r // r may not exist yet... we might just be over a control that is frameless... later we'll relate as child
																	 );
            UpdateDisplay( analog->render );
				SetRedrawHandler( analog->render, DrawClockLayers, (PTRSZVAL)pc );
			}
			clock->analog_clock = analog;
         //EnableSpriteMethod( GetFrameRenderer( GetFrame( pc ) ), DrawAnalogHands, (PTRSZVAL)analog );
		}
      else
		{
			if( !imagename )
			{
            UnmakeImageFile( analog->composite );
            UnmakeImageFile( analog->face );
            UnmakeSprite( analog->second_hand, TRUE );
            UnmakeSprite( analog->minute_hand, TRUE );
				UnmakeSprite( analog->hour_hand, TRUE );
				Release( analog );
            clock->analog_clock = NULL;
			}
			// reconfigure
		}
	}

}


void MakeClockAnalog( PSI_CONTROL pc )
{
   MakeClockAnalogEx( pc, WIDE("images/Clock.png"), NULL );
}
PSI_CLOCK_NAMESPACE_END




