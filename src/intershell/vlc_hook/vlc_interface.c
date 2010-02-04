//#define DEFINE_DEFAULT_IMAGE_INTERFACE
//#define USE_RENDER_INTERFACE l.r
#define DEBUG_LOCK_UNLOCK

#define INVERT_DATA
#define VLC_INTERFACE_SOURCE

#include <stdhdrs.h>
#include <system.h>
#include <sqlgetoption.h>
#include <deadstart.h>
#include <idle.h>
#include <controls.h>
#include <vlc/vlc.h>
#include <vlc/mediacontrol.h>
#include <filesys.h>
#include <sack_types.h>
#include "vlcint.h"

struct vlc_release
{
	_32 tick;
   struct my_vlc_interface *pmyi;
};

static struct {
	PRENDER_INTERFACE r;
	TEXTSTR vlc_path;
   TEXTSTR vlc_config;
	PLIST vlc_releases;
   CRITICALSECTION buffer;
	CRITICALSECTION creation;
	struct
	{
		BIT_FIELD bLogTiming : 1;
		BIT_FIELD bHoldUpdates : 1; // rude global flag, stops all otuputs on all surfaces...
		BIT_FIELD bUpdating : 1; // set while an update is happening...
	} flags;
}l;

void ReleaseInstance( struct my_vlc_interface *pmyi );

void CPROC Cleaner( PTRSZVAL psv )
{
	struct vlc_release *release;
	INDEX idx;
	LIST_FORALL( l.vlc_releases, idx, struct vlc_release *, release )
	{
		if( ( release->tick + 250 ) < tickGetTick() )
		{
			ReleaseInstance( release->pmyi );
         SetLink( &l.vlc_releases, idx, NULL );
		}
	}
}

PRELOAD( InitInterfaces )
{
	TEXTCHAR vlc_path[256];
	snprintf( vlc_path, sizeof( vlc_path ), "%s/vlc", OSALOT_GetEnvironmentVariable( "MY_LOAD_PATH" ) );
#ifndef __NO_OPTIONS__
	SACK_GetPrivateProfileString( "vlc/config", "vlc path", vlc_path, vlc_path, sizeof( vlc_path ), "video.ini" );
	l.vlc_path = StrDup( vlc_path );
#else
   l.vlc_path = "vlc";
#endif
	l.vlc_config = "c:\\ftn3000\\etc\\vlc.cfg";
#ifndef __NO_OPTIONS__
	SACK_GetPrivateProfileString( "vlc/config", "vlc config", l.vlc_config, vlc_path, sizeof( vlc_path ), "video.ini" );
	l.vlc_config = StrDup( vlc_path );
#else
#endif

   l.flags.bLogTiming = SACK_GetPrivateProfileInt( "vlc/config", "log timing", 0, "video.ini" );
   lprintf( "path is %s", vlc_path );
	{
		int n;
		for( n = 0; l.vlc_path[n]; n++ )
			if( l.vlc_path[n] == '/' )
				l.vlc_path[n] = '\\';
	}
	l.r = GetDisplayInterface();
   AddTimer( 100, Cleaner, 0 );
}

typedef void (CPROC *mylibvlc_callback_t )( const libvlc_event_t *, void * );


static struct vlc_interface
{
#define declare_func(a,b,c) a (CPROC *b) c
#define setup_func(a,b,c) vlc.b=(a(CPROC*)c)LoadFunction( lib, #b )

	declare_func( void, libvlc_exception_init, ( libvlc_exception_t * ) );
	declare_func( libvlc_instance_t *,libvlc_new,( int , const char *const *, libvlc_exception_t *) );

	declare_func( libvlc_media_t *, libvlc_media_new, (
                                   libvlc_instance_t *,
                                   const char *,
                                   libvlc_exception_t * ) );
	declare_func( libvlc_media_player_t *, libvlc_media_player_new_from_media, ( libvlc_media_t *, libvlc_exception_t * ) );
	declare_func( void, libvlc_media_release,(
                                   libvlc_media_t * ) );
	declare_func( void, libvlc_media_player_play, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	declare_func( void, libvlc_media_player_pause, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	declare_func( void, libvlc_media_player_stop, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	declare_func( void, libvlc_release, ( libvlc_instance_t * ) );
	declare_func( void, libvlc_media_player_release, ( libvlc_media_player_t * ) );
	declare_func( _32, libvlc_media_player_get_position, ( libvlc_media_player_t *, libvlc_exception_t *) );
   declare_func( libvlc_time_t, libvlc_media_player_get_length, ( libvlc_media_player_t *, libvlc_exception_t *) );
	declare_func( libvlc_time_t, libvlc_media_player_get_time, ( libvlc_media_player_t *, libvlc_exception_t *) );
   declare_func( void, libvlc_media_player_set_time, ( libvlc_media_player_t *, libvlc_time_t, libvlc_exception_t *) );
   declare_func( mediacontrol_Instance *, mediacontrol_new_from_instance,( libvlc_instance_t*,
																								  mediacontrol_Exception * ) );
   declare_func( mediacontrol_Exception *,
					 mediacontrol_exception_create,( void ) );
   declare_func( mediacontrol_StreamInformation *,
					 mediacontrol_get_stream_information,( mediacontrol_Instance *self,
																	  mediacontrol_PositionKey a_key,
																	  mediacontrol_Exception *exception ) );
   declare_func( void,
					 libvlc_media_list_add_media, ( libvlc_media_list_t *,
															 libvlc_media_t *,
															 libvlc_exception_t * ) );
   declare_func( libvlc_media_list_t *,
					 libvlc_media_list_new,( libvlc_instance_t *, libvlc_exception_t * ) );
   declare_func( void,
					 libvlc_media_list_release, ( libvlc_media_list_t * ) );
   declare_func( void,
    libvlc_media_list_player_set_media_player,(
                                     libvlc_media_list_player_t * ,
                                     libvlc_media_player_t * ,
                                     libvlc_exception_t * ) );
   declare_func( void,
    libvlc_media_list_player_set_media_list,(
                                     libvlc_media_list_player_t * ,
                                     libvlc_media_list_t * ,
															libvlc_exception_t * ) );
   declare_func( void,
    libvlc_media_list_player_play, ( libvlc_media_list_player_t *,
												libvlc_exception_t * ) );
   declare_func( void,
    libvlc_media_list_player_pause, ( libvlc_media_list_player_t *,
												libvlc_exception_t * ) );
   declare_func( void,
    libvlc_media_list_player_stop, ( libvlc_media_list_player_t *,
												libvlc_exception_t * ) );
   declare_func( void,
    libvlc_media_list_player_next, ( libvlc_media_list_player_t *,
												libvlc_exception_t * ) );
   declare_func( libvlc_media_list_player_t *,
    libvlc_media_list_player_new, ( libvlc_instance_t * ,
											  libvlc_exception_t * ) );
   declare_func( void,
    libvlc_media_list_player_release, ( libvlc_media_list_player_t * ) );
	declare_func( libvlc_media_player_t *, libvlc_media_player_new, ( libvlc_instance_t *, libvlc_exception_t * ) );
	declare_func( void,  libvlc_media_list_player_play_item_at_index, (libvlc_media_list_player_t *, int , libvlc_exception_t *) );
   declare_func( libvlc_state_t,
    libvlc_media_list_player_get_state, ( libvlc_media_list_player_t * ,
                                        libvlc_exception_t *  ) );
   declare_func( void, libvlc_event_attach, ( libvlc_event_manager_t *,
                                         libvlc_event_type_t,
                                         mylibvlc_callback_t,
                                         void *,
															libvlc_exception_t * ) );
   declare_func( void, libvlc_event_detach, ( libvlc_event_manager_t *p_event_manager,
                                         libvlc_event_type_t i_event_type,
                                         mylibvlc_callback_t f_callback,
                                         void *user_data,
															libvlc_exception_t *p_e ) );
   declare_func( libvlc_event_manager_t *,
					 libvlc_media_event_manager,( libvlc_media_t * p_md,
														  libvlc_exception_t * p_e ) );
   declare_func( libvlc_event_manager_t *, libvlc_media_player_event_manager, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	//void (*raise)(libvlc_exception_t*);
   declare_func( void, libvlc_media_add_option, (
                                   libvlc_media_t * p_md,
                                   const char * ppsz_options,
                                   libvlc_exception_t * p_e ) );

	void (*raiseEx)(libvlc_exception_t* DBG_PASS );

} vlc;

struct my_vlc_interface
{
   CTEXTSTR name;
	libvlc_exception_t ex;
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
   libvlc_media_list_player_t *mlp;
	libvlc_media_t *m;
	//libvlc_media_descriptor_t *md;
	libvlc_media_list_t *ml;
   libvlc_event_manager_t *mpev; // player event
   libvlc_event_manager_t *mev; // media event

	mediacontrol_Instance *mc;
   mediacontrol_Exception *mcex;
	mediacontrol_StreamInformation *si;
	//int host_image_show;
	//int host_image_grab;

   //Image host_images[6];
   Image host_image;
	PSI_CONTROL host_control;
	PRENDERER host_surface;
	int nSurface;
	Image surface;

	int list_count;
	int list_index;

	PTHREAD waiting;
	struct {
		BIT_FIELD bDone : 1;
		BIT_FIELD bPlaying : 1;
	} flags;

   PLIST controls;
   PLIST panels;

   int images; // should be a count of frames in available and updated.
	PLINKQUEUE available_frames;
	PLINKQUEUE updated_frames;
   PTHREAD update_thread;
};

PLIST interfaces;

void MyRaise( libvlc_exception_t *ex DBG_PASS )
#define raise(ex) raiseEx( ex DBG_SRC )
{
	if( ex->b_raised )
	{
		lprintf( "Exception: %s", ex->psz_message );
		//DebugBreak();
		lprintf( "Some exception...." );
	}
	//if( ex->i_code )
   //   lprintf( "code...%d", ex->i_code );
   // did ex happen?
}

static void CPROC _libvlc_callback_t( const libvlc_event_t *event, void *user )
{

}
static void CPROC PlayerEvent( const libvlc_event_t *event, void *user )
{
	struct my_vlc_interface *pmyi = (struct my_vlc_interface *)user;
	switch( event->type )
	{
	case libvlc_MediaPlayerStopped:
		lprintf( "Stopped." );
		pmyi->flags.bDone = 1;
      if( pmyi->waiting )
			WakeThread( pmyi->waiting );
      break;
	case libvlc_MediaPlayerEndReached:
		lprintf( "End reached." );
      if(0)
		{
			struct vlc_release *vlc_release = New( struct vlc_release );
			vlc_release->pmyi = pmyi;
         vlc_release->tick = timeGetTime();
			AddLink( &l.vlc_releases, vlc_release );
		}
		pmyi->flags.bDone = 1;
      if( pmyi->waiting )
			WakeThread( pmyi->waiting );
      break;
	}

}

static void *CPROC lock( PTRSZVAL psv )
{
	struct my_vlc_interface *pmyi = (struct my_vlc_interface*)psv;
   Image capture = (Image)DequeLink( &pmyi->available_frames );
	CDATA *data;
#ifdef DEBUG_LOCK_UNLOCK
	lprintf( "locking..." );
#endif
	if( !capture )
	{
      pmyi->images++;
      lprintf( "Ran out of images... adding a new one... %d", pmyi->images );
		capture = MakeImageFile( 352, 240 );
	}

	pmyi->surface = capture;
	pmyi->host_image = capture; // last captured image for unlock to enque.
	data = GetImageSurface( pmyi->host_image );

#if 0
	//EnterCriticalSec( &l.buffer );
	if( pmyi->host_image )
	{
		if( l.flags.bLogTiming )
			lprintf( "Result soft-image" );
		data = GetImageSurface ( pmyi->surface = pmyi->host_image );
	}
   else if( pmyi->host_control )
		data = GetImageSurface( pmyi->surface = GetControlSurface( pmyi->host_control ) );
	else if( pmyi->host_surface )
	{
      //lprintf( "Resulting render surface..." );
		data = GetImageSurface( pmyi->surface = GetDisplayImage( pmyi->host_surface ) );
	}
#endif

	if( !pmyi->flags.bPlaying )
	{
		pmyi->flags.bPlaying = 1;
	}
#ifdef INVERT_DATA
	data += pmyi->surface->pwidth * (pmyi->surface->height -1);
#endif
#ifdef DEBUG_LOCK_UNLOCK
	lprintf( "Resulting surface %p", data );
#endif
	return data;
}

static void AdjustAlpha( struct my_vlc_interface *pmyi )
{
	PCDATA surface = GetImageSurface( pmyi->surface );
	_32 oo = pmyi->surface->pwidth;
	_32 width = pmyi->surface->width;
   _32 height = pmyi->surface->height;
	_32 r;
	for( r = 0; r < height; r++ )
	{
      _32 c;
		for( c = 0; c < width; c++ )
		{
			CDATA p = surface[c];
			_32 cr = RedVal( p );
			_32 g = GreenVal( p );
			_32 b = BlueVal( p );
			_32 a;
			if( (cr + g + b) > ( 3*220 ) )
				a = 0;
         else
				a =  ((cr<32?255:(240-cr)) +(g<32?255:(240-g)) +(b<32?255:(240-b) )) / 9;
         surface[c] = SetAlpha( p, a );
		}
		surface += oo;
	}
}

// this takes images from the queue of updated images and puts them on the display.
static PTRSZVAL CPROC UpdateThread( PTHREAD thread )
{
	struct my_vlc_interface *pmyi = (struct my_vlc_interface*)GetThreadParam( thread );
	while( 1 )
	{
		Image last_output_image;
      Image output_image = (Image)DequeLink( &pmyi->updated_frames );
		if( output_image )
		{
#ifdef DEBUG_LOCK_UNLOCK
			lprintf( "Updating..." );
#endif
			last_output_image = output_image;
         l.flags.bUpdating = 1;
			while( output_image = (Image)DequeLink( &pmyi->updated_frames ) )
			{
				EnqueLink( &pmyi->available_frames, last_output_image );
            last_output_image = output_image;
			}
			if( l.flags.bHoldUpdates )
			{
            l.flags.bUpdating = 0;
				continue;
			}

         output_image = last_output_image;
			if( pmyi->host_image )
			{
				if( pmyi->controls )
				{
					INDEX idx;
					PSI_CONTROL pc;
					if( l.flags.bLogTiming )
						lprintf( "updating controls..." );
					LIST_FORALL( pmyi->controls, idx, PSI_CONTROL, pc )
					{
						if( !IsControlHidden( pc ) )
						{
							Image output = GetControlSurface( pc );
							if( output )
							{
								//AdjustAlpha( pmyi);
								if( l.flags.bLogTiming )
									lprintf( "Output to control." );
								BlotScaledImageAlpha( output, output_image, ALPHA_TRANSPARENT );
								if( l.flags.bLogTiming )
									lprintf( "output the control (screen)" );
								// may hide btween here and there...
								UpdateFrame( pc, 0, 0, 0, 0 );
								if( l.flags.bLogTiming )
									lprintf( "Finished output." );
							}
						}
						else
						{
							if( l.flags.bLogTiming )
								lprintf( "hidden control..." );
						}
					}
				}
				if( pmyi->panels )
				{
					INDEX idx;
					PRENDERER render;
					if( l.flags.bLogTiming )
						lprintf( "updating panels..." );
					LIST_FORALL( pmyi->panels, idx, PRENDERER, render )
					{
						if( IsDisplayHidden( render ) )
							continue;
						{
							Image output = GetDisplayImage( render );
							//lprintf( "img %p", output );
							if( output )
							{
								if( l.flags.bLogTiming )
									lprintf( "adjusting alpha...", output->width, output->height );
								AdjustAlpha( pmyi);
								if( l.flags.bLogTiming )
									lprintf( "Ouptut to render. %d %d", output->width, output->height );
								BlotScaledImage( output, output_image );
								//BlotImage( output, pmyi->host_image, 0, 0 );
								//RedrawDisplay( renderer );
								if( l.flags.bLogTiming )
									lprintf( "Force out." );
								UpdateDisplayPortion( render, 0, 0, 0, 0 );
								if( l.flags.bLogTiming )
									lprintf( "Force out done." );

							}
						}
					}
				}
				if( l.flags.bLogTiming )
					lprintf( "Update finished." );
			}
			else if( pmyi->host_control )
				UpdateFrame( pmyi->host_control, 0, 0, 0, 0 );
			else if( pmyi->host_surface )
			{
				UpdateDisplayPortion( pmyi->host_surface, 0, 0, 0, 0 );
			}
         EnqueLink( &pmyi->available_frames, output_image );
         l.flags.bUpdating = 0;
		}
		else
		{
#ifdef DEBUG_LOCK_UNLOCK
			lprintf( "Sleeping waiting to update..." );
#endif
			WakeableSleep( SLEEP_FOREVER );
		}
	}
}

static void CPROC unlock( PTRSZVAL psv )
{
	struct my_vlc_interface *pmyi = (struct my_vlc_interface*)psv;
	// in this case, we want to update the control - it's already had its content filled.
#ifdef DEBUG_LOCK_UNLOCK
	lprintf( "unlock" );
#endif
	EnqueLink( &pmyi->updated_frames, pmyi->host_image );
	WakeThread( pmyi->update_thread );
}

void ReleaseInstance( struct my_vlc_interface *pmyi )
{
	//DestroyFrame( &pmyi->host_control );
	if( !pmyi->mlp )
	{
		//vlc.libvlc_media_player_stop (pmyi->mp, &pmyi->ex);
		//vlc.raise (&pmyi->ex);

		/* Free the media_player */
		lprintf( "Releasing instance..." );
		vlc.libvlc_media_player_release (pmyi->mp);
		vlc.raise (&pmyi->ex);

		if( 0 && pmyi->mpev )
		{
			vlc.libvlc_event_detach( pmyi->mpev
										  , libvlc_MediaPlayerEndReached
										  , PlayerEvent
										  , pmyi, &pmyi->ex );
			vlc.raise (&pmyi->ex);
			vlc.libvlc_event_detach( pmyi->mpev
										  , libvlc_MediaPlayerStopped
										  , PlayerEvent
										  , pmyi, &pmyi->ex );
			vlc.raise (&pmyi->ex);
		}

		vlc.libvlc_release (pmyi->inst);
		vlc.raise (&pmyi->ex);
	}
	if( pmyi->host_surface )
	{
		lprintf( "------------- CLOSE DISPLAY ------------" );
      //HideDisplay( pmyi->host_surface );
		CloseDisplay( pmyi->host_surface );
	}
		lprintf( "Released instance..." );

	DeleteLink( &interfaces, pmyi );
	Release( pmyi );
}

void LoadVLC( void )
{
	PVARTEXT pvt;
	char *lib;
   int n;
   char pathbuf2[256];
	if( vlc.libvlc_exception_init )
		return;

	pvt = VarTextCreate();

	OSALOT_AppendEnvironmentVariable( "PATH", l.vlc_path );
   snprintf( pathbuf2, sizeof( pathbuf2 ), "%s\\plugins", l.vlc_path );
	for( n = 0; pathbuf2[n]; n++ )
		if( pathbuf2[n] == '/' )
         pathbuf2[n] = '\\';
	OSALOT_AppendEnvironmentVariable( "PATH", pathbuf2 );
   vtprintf( pvt, "%s\\libvlc.dll", l.vlc_path );
   lib = GetText( VarTextPeek( pvt ) );
   //vlc.raise = MyRaise;
   vlc.raiseEx = MyRaise;
	setup_func( void, libvlc_exception_init, ( libvlc_exception_t * ) );
	setup_func( libvlc_instance_t *,libvlc_new,( int , const char *const *, libvlc_exception_t *) );
	setup_func( libvlc_media_t *, libvlc_media_new, ( libvlc_instance_t *p_instance,
																	 const char * psz_mrl,
																	 libvlc_exception_t *p_e ) );
   setup_func( void, libvlc_media_add_option, (
                                   libvlc_media_t * p_md,
                                   const char * ppsz_options,
                                   libvlc_exception_t * p_e ) );
   setup_func( libvlc_media_player_t *, libvlc_media_player_new, ( libvlc_instance_t *, libvlc_exception_t * ) );
	setup_func( libvlc_media_player_t *, libvlc_media_player_new_from_media, ( libvlc_media_t *, libvlc_exception_t * ) );
	setup_func( void, libvlc_media_release,(
                                   libvlc_media_t * ) );
	setup_func( libvlc_media_player_t *, libvlc_media_player_new_from_media, ( libvlc_media_t *, libvlc_exception_t * ) );
	setup_func( void, libvlc_media_release,(
                                   libvlc_media_t * ) );
	setup_func( void, libvlc_media_player_play, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	setup_func( void, libvlc_media_player_pause, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	setup_func( void, libvlc_media_player_stop, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	setup_func( void, libvlc_release, ( libvlc_instance_t * ) );
	setup_func( void, libvlc_media_player_release, ( libvlc_media_player_t * ) );
	setup_func( _32, libvlc_media_player_get_position, ( libvlc_media_player_t *, libvlc_exception_t *) );
	setup_func( libvlc_time_t, libvlc_media_player_get_length, ( libvlc_media_player_t *, libvlc_exception_t *) );
   setup_func( libvlc_time_t, libvlc_media_player_get_time, ( libvlc_media_player_t *, libvlc_exception_t *) );
	setup_func( mediacontrol_Instance *,
					 mediacontrol_new_from_instance,( libvlc_instance_t*,
																mediacontrol_Exception * ) );
   setup_func( mediacontrol_Exception *,
					 mediacontrol_exception_create,( void ) );
   setup_func( mediacontrol_StreamInformation *,
					 mediacontrol_get_stream_information,( mediacontrol_Instance *self,
																	  mediacontrol_PositionKey a_key,
																	  mediacontrol_Exception *exception ) );
	setup_func( void,
				  libvlc_media_list_add_media, ( libvlc_media_list_t *,
														  libvlc_media_t *,
														  libvlc_exception_t * ) );
   setup_func( void,
    libvlc_media_list_player_set_media_player,(
                                     libvlc_media_list_player_t * ,
                                     libvlc_media_player_t * ,
															  libvlc_exception_t * ) );
   setup_func( void,
    libvlc_media_list_player_set_media_list,(
                                     libvlc_media_list_player_t * ,
                                     libvlc_media_list_t * ,
                                     libvlc_exception_t * ) );
   setup_func( void,
    libvlc_media_list_player_play, ( libvlc_media_list_player_t *,
                                   libvlc_exception_t * ) );
   setup_func( void,
    libvlc_media_list_player_stop, ( libvlc_media_list_player_t *,
												libvlc_exception_t * ) );
   setup_func( libvlc_media_list_player_t *,
    libvlc_media_list_player_new, ( libvlc_instance_t * ,
                                  libvlc_exception_t * ) );
   setup_func( void,
    libvlc_media_list_player_release, ( libvlc_media_list_player_t * ) );
   setup_func( libvlc_media_list_t *,
					 libvlc_media_list_new,( libvlc_instance_t *, libvlc_exception_t * ) );
   setup_func( void,
					 libvlc_media_list_release, ( libvlc_media_list_t * ) );
   setup_func( void,  libvlc_media_list_player_play_item_at_index, (libvlc_media_list_player_t *, int , libvlc_exception_t *) );
   setup_func( void,
    libvlc_media_list_player_next, ( libvlc_media_list_player_t *,
												libvlc_exception_t * ) );
	setup_func( libvlc_state_t,
    libvlc_media_list_player_get_state, ( libvlc_media_list_player_t * ,
													  libvlc_exception_t *  ) );
   setup_func( void, libvlc_event_attach, ( libvlc_event_manager_t *,
                                         libvlc_event_type_t ,
                                         mylibvlc_callback_t ,
                                         void *,
                                         libvlc_exception_t * ) );
	setup_func( void, libvlc_event_detach, ( libvlc_event_manager_t *,
														 libvlc_event_type_t ,
														 mylibvlc_callback_t ,
														 void *,
														 libvlc_exception_t * ) );
   setup_func( libvlc_event_manager_t *,
					 libvlc_media_event_manager,( libvlc_media_t * p_md,
														  libvlc_exception_t * p_e ) );
   setup_func( libvlc_event_manager_t *, libvlc_media_player_event_manager, ( libvlc_media_player_t *, libvlc_exception_t * ) );
	VarTextDestroy( &pvt );
}

void SetupInterface( struct my_vlc_interface *pmyi )
{
	LoadVLC();
   AddLink( &interfaces, pmyi );

}


struct my_vlc_interface *FindInstance( CTEXTSTR url )
{
	INDEX idx;
   struct my_vlc_interface *i;
	LIST_FORALL( interfaces, idx, struct my_vlc_interface *, i )
	{
		if( StrCaseCmp( i->name, url ) == 0 )
         return i;
	}
   return NULL;
}

struct my_vlc_interface *CreateInstance( CTEXTSTR url )
{

	struct my_vlc_interface *pmyi = FindInstance(url);
	if( !pmyi )
	{
		pmyi = New( struct my_vlc_interface );
		MemSet( pmyi, 0, sizeof( struct my_vlc_interface ) );
      pmyi->name = StrDup( url );
		{
			PVARTEXT pvt;
			char **argv;
			int argc;

			SetupInterface( pmyi );
			pvt = VarTextCreate();

			vtprintf( pvt,
						//"--verbose=2"
						" --file-logging"
						" -I dummy --config=%s --plugin-path=%s\\%s"
					  , l.vlc_config
					  , l.vlc_path
					  , "plugins"
					  );

			ParseIntoArgs( GetText( VarTextPeek( pvt ) ), &argc, &argv );
			vlc.libvlc_exception_init (&pmyi->ex);
			/* init vlc modules, should be done only once */
			pmyi->inst = vlc.libvlc_new ( argc, (char const*const*)argv, &pmyi->ex);
			vlc.raise (&pmyi->ex);
			VarTextDestroy( &pvt );
		}
	}
   return pmyi;
}

struct my_vlc_interface *CreateInstanceIn( PSI_CONTROL pc, CTEXTSTR url )
{
	struct my_vlc_interface *pmyi;
	pmyi = FindInstance( url );
   if( !pmyi )
	{
		PVARTEXT pvt;
		char **argv;
		int argc;
		//GetControlSurface( pc );
		//if( !( pmyi = controls_1 ) )
		{
			Image image = MakeImageFile( 352, 240 );
			pmyi = New( struct my_vlc_interface );
			MemSet( pmyi, 0, sizeof( struct my_vlc_interface ) );

         EnqueLink( &pmyi->available_frames, image );
			pmyi->update_thread = ThreadTo( UpdateThread, (PTRSZVAL)pmyi );

			pmyi->host_image = image;
			pmyi->host_control = NULL;//pc;
			pmyi->host_surface = NULL;
         pmyi->name = StrDup( url );
 			SetupInterface( pmyi );

#ifdef __64__
#error blah.
#endif

			pvt = VarTextCreate();
			vtprintf( pvt, "--verbose=2"
						//" --file-logging"
						" -I dummy --config=%s"
						" --no-osd"
						" --no-audio"
						//" --file-caching=0"
						" --plugin-path=%s/%s"
						" --vout=vmem"
						" --vmem-data=%ld"
						" --vmem-width=%ld"
						" --vmem-height=%ld"
						" --vmem-pitch=%ld"
						" --vmem-chroma=RV32"
						" --vmem-lock=%ld"
						" --vmem-unlock=%ld"
						//, OSALOT_GetEnvironmentVariable( "MY_LOAD_PATH" )
					  , l.vlc_config
					  , l.vlc_path
					  , "plugins"
					  , pmyi
					  , image->width
					  , image->height
#ifdef INVERT_DATA
					  , -image->pwidth*4
#else
					  , image->pwidth*4
#endif
					  , lock
					  , unlock
					  );
			lprintf( "Creating instance with %s", GetText( VarTextPeek( pvt ) ) );
			ParseIntoArgs( GetText( VarTextPeek( pvt ) ), &argc, &argv );

			vlc.libvlc_exception_init (&pmyi->ex);
			/* init vlc modules, should be done only once */
			pmyi->inst = vlc.libvlc_new ( argc, (char const*const*)argv, &pmyi->ex);
			vlc.raise (&pmyi->ex);
			VarTextDestroy( &pvt );

		}
	}

	AddLink( &pmyi->controls, pc );

	return pmyi;
}


static int EvalExcept( int n )
{
	switch( n )
	{
	case 		STATUS_ACCESS_VIOLATION:
		//if( l.flags.bLogKeyEvent )
			lprintf( "Access violation - OpenGL layer at this moment.." );
	return EXCEPTION_EXECUTE_HANDLER;
	default:
		//if( l.flags.bLogKeyEvent )
			lprintf( "Filter unknown : %08X", n );

		return EXCEPTION_CONTINUE_SEARCH;
	}
	return EXCEPTION_CONTINUE_EXECUTION;
}

struct my_vlc_interface *CreateInstanceOn( PRENDERER renderer, CTEXTSTR name )
{
	struct my_vlc_interface *pmyi;
	pmyi  = FindInstance( name );
   if( !pmyi )
	{
		PVARTEXT pvt;
		char **argv;
		int argc;
      Image image = MakeImageFile( 352, 240 );//GetDisplayImage( renderer );
		pmyi = New( struct my_vlc_interface );
      MemSet( pmyi, 0, sizeof( struct my_vlc_interface ) );
      pmyi->host_control = NULL;
		pmyi->host_surface = NULL;//renderer;
		pmyi->host_image = image;

		EnqueLink( &pmyi->available_frames, image );
		pmyi->update_thread = ThreadTo( UpdateThread, (PTRSZVAL)pmyi );

      pmyi->name = StrDup( name );
		{
			int x;
			for( x =0; x < 6; x++ )
			{
				//pmyi->host_images[x] = MakeImageFile( 1024, 768 );
            //EnqueLink( &pmyi->host_image, pmyi->host_images[x] );
			}
		}
      //pmyi->host_images = NewArray( Image, 6 );
      lprintf( "Adding to panels..." );
      AddLink( &pmyi->panels, renderer );
      SetupInterface( pmyi );

		pvt = VarTextCreate();

#ifdef __64__
#error blah.
#endif

		vtprintf( pvt, "--verbose=2"
					//" --file-logging"
					" -I dummy --config=c:\\ftn3000\\etc\\vlc.cfg"
					" --no-osd"
					//" --noaudio"
               //" --skip-frames"
					" --plugin-path=%s\\%s"
               //" --drop-late-frames"
               //" --file-caching=0"
					" --vout vmem"
					" --vmem-data %ld"
					" --vmem-width %ld"
					" --vmem-height %ld"
					" --vmem-pitch %ld"
					" --vmem-chroma RV32"
               " --vmem-lock %ld"
					" --vmem-unlock %ld"
				  , l.vlc_path
				  , "plugins"
				  , pmyi
				  , image->width
				  , image->height
#ifdef INVERT_DATA
				  , -image->pwidth*4
#else
				  , image->pwidth*4
#endif
				  , lock
				  , unlock
				  );

      lprintf( "Creating instance with %s", GetText( VarTextPeek( pvt ) ) );
		ParseIntoArgs( GetText( VarTextPeek( pvt ) ), &argc, &argv );

		vlc.libvlc_exception_init (&pmyi->ex);
		/* init vlc modules, should be done only once */
#ifdef _MSC_VER
		__try
		{
#endif
			pmyi->inst = vlc.libvlc_new ( argc, (char const*const*)argv, &pmyi->ex);
#ifdef _MSC_VER
		}
		__except( EvalExcept( GetExceptionCode() ) )
		{
									lprintf( "Caught exception in libvlc_new" );
		}
#endif
		vlc.raise (&pmyi->ex);
		VarTextDestroy( &pvt );
   
	}
   return pmyi;
}

ATEXIT( unload_vlc_interface )
{
	INDEX idx;
	struct my_vlc_interface *i;
	LIST_FORALL( interfaces, idx, struct my_vlc_interface*, i )
	{
		//DestroyFrame( &i->host_control );
#ifdef __WATCOMC__
#ifndef __cplusplus
		_try {
#endif
#endif
			lprintf( "Doing stop 2..." );
			vlc.libvlc_media_list_player_stop( i->mlp, &i->ex );
			vlc.raise (&i->ex);
#ifdef __WATCOMC__
#ifndef __cplusplus
		}
		_except( EXCEPTION_EXECUTE_HANDLER )
		{
			lprintf( "Caught exception in stop media list player" );
			return;
			;
		}
#endif
#endif
      lprintf( "Doing stop..." );
		vlc.libvlc_media_player_stop (i->mp, &i->ex);
		vlc.raise (&i->ex);

      lprintf( "doing release..." );
		/* Free the media_player */
		vlc.libvlc_media_player_release (i->mp);

		vlc.libvlc_media_list_release( i->ml );

		vlc.libvlc_media_list_player_release( i->mlp );

      lprintf( "releasing instance..." );
		vlc.libvlc_release (i->inst);
		vlc.raise (&i->ex);
	}
	/* Stop playing */

}

void PlayItem( CTEXTSTR url_name )
{
	struct my_vlc_interface *pmyi;
	pmyi = CreateInstance( url_name );
	pmyi->m = vlc.libvlc_media_new (pmyi->inst, url_name, &pmyi->ex);
	vlc.raise (&pmyi->ex);

	/* Create a media player playing environement */
	pmyi->mp = vlc.libvlc_media_player_new_from_media( pmyi->m, &pmyi->ex);
	vlc.raise (&pmyi->ex);

	vlc.libvlc_media_release (pmyi->m);

	/* play the media_player */
	vlc.libvlc_media_player_play (pmyi->mp, &pmyi->ex);
	vlc.raise (&pmyi->ex);

}


void StopItem( struct my_vlc_interface *pmyi )
{
   // yeah stop playing.
	vlc.libvlc_media_player_stop (pmyi->mp, &pmyi->ex);
}


struct my_vlc_interface * PlayItemInEx( PSI_CONTROL pc, CTEXTSTR url_name, CTEXTSTR extra_opts )
{
	struct my_vlc_interface *pmyi = FindInstance( url_name );
	if( !pmyi )
	{
		pmyi = CreateInstanceIn( pc, url_name );

		pmyi->ml = vlc.libvlc_media_list_new( pmyi->inst, &pmyi->ex );
		vlc.raise (&pmyi->ex);

		pmyi->m = vlc.libvlc_media_new (pmyi->inst, url_name, &pmyi->ex);
		vlc.raise (&pmyi->ex);

		if( extra_opts )
		{
			vlc.libvlc_media_add_option( pmyi->m, extra_opts, &pmyi->ex );
			vlc.raise( &pmyi->ex);
		}
		vlc.libvlc_media_list_add_media( pmyi->ml, pmyi->m, &pmyi->ex );
		vlc.raise (&pmyi->ex);

		vlc.libvlc_media_release (pmyi->m);
		vlc.raise (&pmyi->ex);
      pmyi->m = NULL;

		pmyi->mlp = vlc.libvlc_media_list_player_new( pmyi->inst, &pmyi->ex);
		vlc.raise (&pmyi->ex);

		/* Create a media player playing environement */
		pmyi->mp = vlc.libvlc_media_player_new( pmyi->inst, &pmyi->ex);
		vlc.raise (&pmyi->ex);

		vlc.libvlc_media_list_player_set_media_list( pmyi->mlp, pmyi->ml, &pmyi->ex );
		vlc.raise (&pmyi->ex);

		vlc.libvlc_media_list_player_set_media_player( pmyi->mlp, pmyi->mp, &pmyi->ex );
		vlc.raise (&pmyi->ex);

		{
			//libvlc_time_t t_length = vlc.libvlc_media_player_get_length( pmyi->mp, &pmyi->ex );
			//lprintf( "length %ld", t_length );
		}

		//vlc.libvlc_media_release (pmyi->m);
		{
			//libvlc_time_t t_length = vlc.libvlc_media_player_get_length( pmyi->mp, &pmyi->ex );
			//lprintf( "length %ld", t_length );
		}

		vlc.libvlc_media_list_player_play_item_at_index( pmyi->mlp, 0, &pmyi->ex );

		//vlc.libvlc_media_list_player_play( pmyi->mlp, &pmyi->ex );

		/* play the media_player */
		//vlc.libvlc_media_player_play (pmyi->mp, &pmyi->ex);
		vlc.raise (&pmyi->ex);
		{
		//libvlc_time_t t_length = vlc.libvlc_media_player_get_length( pmyi->mp, &pmyi->ex );
			//lprintf( "length %ld", t_length );
		}
	}
	else
      AddLink( &pmyi->controls, pc );
	return pmyi;
}

struct my_vlc_interface * PlayItemIn( PSI_CONTROL pc, CTEXTSTR url_name )
{
   return PlayItemInEx( pc, url_name, NULL );
}



struct on_thread_params
{
   int done;
	PRENDERER renderer;
   CTEXTSTR url_name;
};

PTRSZVAL CPROC PlayItemOnThread( PTHREAD thread )
{
	PTRSZVAL psv = GetThreadParam( thread );
	struct on_thread_params *parms = (struct on_thread_params*)psv;
	struct my_vlc_interface *pmyi;

	TEXTCHAR buffer[566];
	GetCurrentPath( buffer, sizeof(buffer) );
	pmyi = FindInstance( parms->url_name );
	if( !pmyi )
	{

		pmyi = CreateInstanceOn( parms->renderer, parms->url_name );
		//lprintf( "Instance completed." );
		pmyi->m = vlc.libvlc_media_new (pmyi->inst, parms->url_name, &pmyi->ex);
		vlc.raise (&pmyi->ex);
		parms->done = 1;

		/* Create a media player playing environement */
		pmyi->mp = vlc.libvlc_media_player_new_from_media( pmyi->m, &pmyi->ex);
		vlc.raise (&pmyi->ex);

		parms->done = 1;
		pmyi->mpev = vlc.libvlc_media_player_event_manager( pmyi->mp, &pmyi->ex );
		vlc.libvlc_event_attach( pmyi->mpev, libvlc_MediaPlayerEndReached, PlayerEvent, pmyi, &pmyi->ex );
		vlc.libvlc_media_release (pmyi->m);

      //WakeableSleep( 1000 );
		/* play the media_player */
      //lprintf( "Instance play." );
		vlc.libvlc_media_player_play( pmyi->mp, &pmyi->ex);

		vlc.raise (&pmyi->ex);
		pmyi->waiting = thread;
		parms->done = 1;
		while( !pmyi->flags.bDone )
		{
			//lprintf( "Waiting for done..." );
			WakeableSleep( 10000 );
		}
		lprintf( "Vdieo ended... cleanup." );
		ReleaseInstance( pmyi );
	}
	else
	{
      lprintf( "Device already open... adding renderer." );
      // probably another thread is already running this...
		AddLink( &pmyi->panels, parms->renderer );
      parms->done = 1;
	}
	return 0;
}

struct my_vlc_interface * PlayItemOn( PRENDERER renderer, CTEXTSTR url_name )
{
	struct on_thread_params parms;
	parms.renderer = renderer;
	parms.url_name = url_name;
	parms.done = 0;
   //EnterCriticalSec( &l.creation );
	ThreadTo( PlayItemOnThread, (PTRSZVAL)&parms );
	while( !parms.done )
      Relinquish();
   //LeaveCriticalSec( &l.creation );
	return NULL;
}

struct my_vlc_interface * PlayItemOnEx( PRENDERER renderer, CTEXTSTR url_name, CTEXTSTR extra_opts )
{
	struct on_thread_params parms;
	parms.renderer = renderer;
	parms.url_name = url_name;
	parms.done = 0;
   //EnterCriticalSec( &l.creation );
	ThreadTo( PlayItemOnThread, (PTRSZVAL)&parms );
	while( !parms.done )
      Relinquish();
   //LeaveCriticalSec( &l.creation );
	return NULL;
}

void PlayUsingMediaList( struct my_vlc_interface *pmyi, PLIST files )
{
	CTEXTSTR file_to_play;
   INDEX idx;
	pmyi->ml = vlc.libvlc_media_list_new( pmyi->inst, &pmyi->ex );
   vlc.raise (&pmyi->ex);

	LIST_FORALL( files, idx, CTEXTSTR, file_to_play )
	{
		pmyi->m = vlc.libvlc_media_new (pmyi->inst, file_to_play, &pmyi->ex);
		vlc.raise (&pmyi->ex);

		pmyi->mev = vlc.libvlc_media_event_manager( pmyi->m, &pmyi->ex );
		vlc.raise (&pmyi->ex);

		vlc.libvlc_media_list_add_media( pmyi->ml, pmyi->m, &pmyi->ex );
		vlc.raise (&pmyi->ex);

		vlc.libvlc_media_release (pmyi->m);
		vlc.raise (&pmyi->ex);
      pmyi->list_count++;
	}



	pmyi->mlp = vlc.libvlc_media_list_player_new( pmyi->inst, &pmyi->ex);
	vlc.raise (&pmyi->ex);

	/* Create a media player playing environement */
	pmyi->mp = vlc.libvlc_media_player_new( pmyi->inst, &pmyi->ex);
	vlc.raise (&pmyi->ex);

	vlc.libvlc_media_list_player_set_media_list( pmyi->mlp, pmyi->ml, &pmyi->ex );
   vlc.raise (&pmyi->ex);

   vlc.libvlc_media_list_player_set_media_player( pmyi->mlp, pmyi->mp, &pmyi->ex );
   vlc.raise (&pmyi->ex);


	vlc.libvlc_media_list_player_play_item_at_index( pmyi->mlp, pmyi->list_index, &pmyi->ex );
	vlc.raise (&pmyi->ex);
	pmyi->list_index++;
	if( pmyi->list_index == pmyi->list_count )
		pmyi->list_index = 0;


      //DebugBreak();
      //pmyi->mcex = vlc.mediacontrol_exception_create();
		//pmyi->mc = vlc.mediacontrol_new_from_instance( pmyi->inst, pmyi->mcex );
      //pmyi->si = vlc.mediacontrol_get_stream_information( pmyi->mc, 0, pmyi->mcex );

		//t_length = vlc.libvlc_media_player_get_length( pmyi->mp, &pmyi->ex );
      //lprintf( "length is %ld", t_length );
	//vlc.raise( &pmyi->ex );
   pmyi->mpev = vlc.libvlc_media_player_event_manager( pmyi->mp, &pmyi->ex );
   vlc.libvlc_event_attach( pmyi->mpev, libvlc_MediaPlayerEndReached, PlayerEvent, pmyi, &pmyi->ex );

	//vlc.libvlc_media_list_player_next( pmyi->mlp, &pmyi->ex );
   //vlc.raise (&pmyi->ex);

	{
      int played = 0;
		libvlc_time_t t_current;
		libvlc_time_t t_length;
		do
		{
#if 0
         libvlc_state_t state = vlc.libvlc_media_list_player_get_state( pmyi->mlp, &pmyi->ex );
			lprintf( "... %d", state );
#ifdef __WATCOMC__
			_try {
#endif
				t_current = vlc.libvlc_media_player_get_time( pmyi->mp, &pmyi->ex );
#ifdef __WATCOMC__
			}
			_except( EXCEPTION_EXECUTE_HANDLER )
			{
				lprintf( "Caught exception in get_time" );
            return;
				;
			}
#endif

         lprintf( "..." );
			vlc.raise( &pmyi->ex );
         lprintf( "..." );
			if( t_current )
			{
            played = 1;
				//if( t_length == 0 )
				{
         lprintf( "..." );
					t_length = vlc.libvlc_media_player_get_length( pmyi->mp, &pmyi->ex );
         lprintf( "..." );
					vlc.raise( &pmyi->ex );
         lprintf( "..." );
				}
				if( t_current > 15000  &&(  t_current < ( t_length - 15000 )) )
				{
#if defined( __WATCOMC__ )
#ifndef __cplusplus
						_try
						{
#endif
#endif
						vlc.libvlc_media_list_player_next( pmyi->mlp, &pmyi->ex );
#if defined( __WATCOMC__ )
#ifndef __cplusplus
					}
					_except( EXCEPTION_EXECUTE_HANDLER )
					{
						vlc.libvlc_media_list_player_play_item_at_index( pmyi->mlp, 0, &pmyi->ex );
						//lprintf( "Caught exception in video output window" );
						;
					}
#endif
#endif
					{

					}
					//vlc.libvlc_media_list_player_pause (pmyi->mlp, &pmyi->ex);
					vlc.raise (&pmyi->ex);
					//vlc.libvlc_media_player_set_time( pmyi->mp, t_length - 15000, &pmyi->ex );
					//vlc.libvlc_media_list_player_play (pmyi->mlp, &pmyi->ex);
					//vlc.raise (&pmyi->ex);
				}

				lprintf( "now is %lld %lld", t_length, t_current );
			}
			else
            lprintf( "current is 0..." );
			if( !t_current && played )
			{
            played = 0;
				lprintf( "..." );
				//vlc.libvlc_media_list_player_play_item_at_index( pmyi->mlp, index++, &pmyi->ex );
				if( index == count )
					index = 0;
			}
			lprintf( "..." );
#endif
			IdleFor( 250 );//+ ( t_length - t_current ) / 5 );
         lprintf( "Tick..." );
		}
		while( 1  );//|| ( t_length - t_current ) > 50 );
	}
}


void SetPriority( DWORD proirity_class )
		{
			HANDLE hToken, hProcess;
			TOKEN_PRIVILEGES tp;
			OSVERSIONINFO osvi;
			DWORD dwPriority;
			osvi.dwOSVersionInfoSize = sizeof( osvi );
			GetVersionEx( &osvi );
			if( osvi.dwPlatformId  == VER_PLATFORM_WIN32_NT )
			{
				//b95 = FALSE;
				// allow shutdown priviledges....
				// wonder if /shutdown will work wihtout _this?
				if( DuplicateHandle( GetCurrentProcess(), GetCurrentProcess()
										 , GetCurrentProcess(), &hProcess, 0
										 , FALSE, DUPLICATE_SAME_ACCESS  ) )
					if( OpenProcessToken( hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken ) )
					{
						tp.PrivilegeCount = 1;
						if( LookupPrivilegeValue( NULL
														, SE_SHUTDOWN_NAME
														, &tp.Privileges[0].Luid ) )
						{
							tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
							AdjustTokenPrivileges( hToken, FALSE, &tp, 0, NULL, NULL );
						}
						else
							GetLastError();
					}
					else
						GetLastError();
				else
					GetLastError();
				CloseHandle( hProcess );
				CloseHandle( hToken );

				dwPriority = proirity_class;
			}
			else
			{
				//b95 = TRUE;
				lprintf( "platform failed." );
				dwPriority = NORMAL_PRIORITY_CLASS;
			}
			if( !SetThreadPriority( GetCurrentThread(), dwPriority ) )
			{
            lprintf( "Priority filed!" );
			}
			//SetPriorityClass( GetCurrentProcess(), dwPriority );
		}


void PlayList( PLIST files, S_32 x, S_32 y, _32 w, _32 h )
{
	CTEXTSTR file_to_play;
   INDEX idx;
	PRENDERER transparent[2];
   int npmyi_to_play = 0; // which is the one that is waiting to play.
   int npmyi_playing = -1; // which is the one that is waiting to play.
	struct my_vlc_interface *pmyi[2];

	//pmyi->list_count = 0;
	//pmyi->list_index = 0;
start:
	LIST_FORALL( files, idx, CTEXTSTR, file_to_play )
	{
		lprintf( "waiting for available queue..." );
		while( npmyi_to_play == npmyi_playing )
			WakeableSleep( 1000 );
		lprintf( "New display..." );
		SetPriority( THREAD_PRIORITY_IDLE );

		transparent[npmyi_to_play] = OpenDisplaySizedAt( DISPLAY_ATTRIBUTE_LAYERED, w, h, x, y );
		DisableMouseOnIdle( transparent[npmyi_to_play], TRUE );

		UpdateDisplay( transparent[npmyi_to_play] );
		lprintf( "New isntance..." );
		pmyi[npmyi_to_play] = CreateInstanceOn( transparent[npmyi_to_play], file_to_play );
		lprintf( "New media..." );

		pmyi[npmyi_to_play]->m = vlc.libvlc_media_new (pmyi[npmyi_to_play]->inst
																	 , file_to_play
																	 , &pmyi[npmyi_to_play]->ex);
		vlc.raise (&pmyi[npmyi_to_play]->ex);

		lprintf( "New media player from media..." );
		/* Create a media player playing environement */
		pmyi[npmyi_to_play]->mp = vlc.libvlc_media_player_new_from_media( pmyi[npmyi_to_play]->m, &pmyi[npmyi_to_play]->ex);
		vlc.raise (&pmyi[npmyi_to_play]->ex);

		lprintf( "Release media..." );
		vlc.libvlc_media_release (pmyi[npmyi_to_play]->m);

		lprintf( "New event manager..." );
		pmyi[npmyi_to_play]->mpev = vlc.libvlc_media_player_event_manager( pmyi[npmyi_to_play]->mp, &pmyi[npmyi_to_play]->ex );
		vlc.libvlc_event_attach( pmyi[npmyi_to_play]->mpev, libvlc_MediaPlayerEndReached, PlayerEvent, pmyi[npmyi_to_play], &pmyi[npmyi_to_play]->ex );
		vlc.raise (&pmyi[npmyi_to_play]->ex);


		SetPriority( THREAD_PRIORITY_NORMAL );
		if( npmyi_playing >= 0 )
		{
			pmyi[npmyi_playing]->waiting = MakeThread();
			while( !pmyi[npmyi_playing]->flags.bDone )
			{
				WakeableSleep( 1000 );
			}
		}
		pmyi[npmyi_to_play]->flags.bDone = FALSE;
		pmyi[npmyi_to_play]->flags.bPlaying = FALSE; // set on first lock of buffer.
		lprintf( "------- BEGIN PLAY ----------- " );
		vlc.libvlc_media_player_play (pmyi[npmyi_to_play]->mp, &pmyi[npmyi_to_play]->ex);
		if( npmyi_playing >= 0 )
		{
			ReleaseInstance( pmyi[npmyi_playing] );
		}
		/* play the media_player */
		vlc.raise (&pmyi[npmyi_to_play]->ex);
		npmyi_playing = npmyi_to_play;
		npmyi_to_play++;
		if( npmyi_to_play == 2 )
         npmyi_to_play = 0;
	}
   goto start;

	// PlayUsingMediaList( pmyi, files );

}


struct on_sound_thread_params
{
   int done;
	PRENDERER renderer;
   CTEXTSTR url_name;
};

PTRSZVAL CPROC PlaySoundItemOnThread( PTHREAD thread )
{
	PTRSZVAL psv = GetThreadParam( thread );
	struct on_sound_thread_params *parms = (struct on_sound_thread_params*)psv;
	struct my_vlc_interface *pmyi;
	pmyi = CreateInstance( parms->url_name );

	pmyi->m = vlc.libvlc_media_new (pmyi->inst, parms->url_name, &pmyi->ex);
	vlc.raise (&pmyi->ex);

	parms->done = 1;
   Relinquish();

	/* Create a media player playing environement */
	pmyi->mp = vlc.libvlc_media_player_new_from_media( pmyi->m, &pmyi->ex);
	vlc.raise (&pmyi->ex);

	pmyi->mpev = vlc.libvlc_media_player_event_manager( pmyi->mp, &pmyi->ex );
   // hrm... sound files don't know mediaendreaached
   //vlc.libvlc_event_attach( pmyi->mpev, libvlc_MediaPlayerEndReached, PlayerEvent, pmyi, &pmyi->ex );
   vlc.libvlc_event_attach( pmyi->mpev, libvlc_MediaPlayerStopped, PlayerEvent, pmyi, &pmyi->ex );
	vlc.libvlc_media_release (pmyi->m);

    /* play the media_player */
    vlc.libvlc_media_player_play (pmyi->mp, &pmyi->ex);
    vlc.raise (&pmyi->ex);

	 pmyi->waiting = thread;
	 while( !pmyi->flags.bDone )
		 WakeableSleep( 10000 );
    lprintf( "Vdieo ended... cleanup." );
	 ReleaseInstance( pmyi );
    return 0;
}



void PlaySoundFile( CTEXTSTR file )
{
	struct on_thread_params parms;
	parms.renderer = NULL;
	parms.url_name = file;
   parms.done = 0;
	ThreadTo( PlaySoundItemOnThread, (PTRSZVAL)&parms );
   // wait for the parameters to be read...
	while( !parms.done )
      Relinquish();

}

void PlaySoundData( POINTER data, size_t length )
{

}

void HoldUpdates( void )
{
   l.flags.bHoldUpdates = 1;
	while( l.flags.bUpdating )
		Relinquish();
   return;
}

void ReleaseUpdates( void )
{
   // next frame update will wake the updating thread...and it will be able to process agian.
   l.flags.bHoldUpdates = 0;
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
///  0.8.6 compatibility...
//------------------------------------------------------------------------

#if 0
static void quit_on_exception (libvlc_exception_t *excp) {
   if (libvlc_exception_raised (excp)) {
      fprintf(stderr, "error: %s\n", libvlc_exception_get_message(excp));
      exit(-1);
   }
}

int main(int argc, char **argv) {
   libvlc_exception_t excp;
   libvlc_instance_t *inst;
   int item;
   char *myarg0 = "-I";  char *myarg1 = "dummy";
   char *myarg2 = "--plugin-path=c:\\program files\\videolan\\plugins";
   char *myargs[4] = {myarg0, myarg1, myarg2, NULL};
   char *filename = "c:\\video\\main\\Everybody_Hates_Chris_Feb_26.mpg";

   libvlc_exception_init (&excp);
   inst = libvlc_new (3, myargs, &excp);
   quit_on_exception (&excp);
   item = libvlc_playlist_add (inst, filename, NULL, &excp); 
   quit_on_exception (&excp);
   libvlc_playlist_play (inst, item, 0, NULL, &excp); 
   quit_on_exception (&excp);
   Sleep (10000);
   libvlc_destroy (inst);
   return 0;
}

#endif
