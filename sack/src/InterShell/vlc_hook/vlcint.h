
#ifdef VLC_INTERFACE_SOURCE
#define XTRN EXPORT_METHOD
#else
#define XTRN IMPORT_METHOD
#endif

#include <controls.h>

XTRN struct my_vlc_interface *PlayItemAgainst( Image image, CTEXTSTR opts );
XTRN void PlayItem( CTEXTSTR thing );
XTRN struct my_vlc_interface *PlayItemIn( PSI_CONTROL pc, CTEXTSTR url_name );
XTRN struct my_vlc_interface * PlayItemInEx( PSI_CONTROL pc, CTEXTSTR url_name, CTEXTSTR extra_opts );
XTRN void StopItem( struct my_vlc_interface *);
XTRN struct my_vlc_interface *PlayItemOn( PRENDERER renderer, CTEXTSTR url_name );
XTRN struct my_vlc_interface * PlayItemOnEx( PRENDERER renderer, CTEXTSTR url_name, CTEXTSTR extra_opts );
XTRN struct my_vlc_interface * PlayItemOnExx( PRENDERER renderer, CTEXTSTR url_name, CTEXTSTR extra_opts, int transparent );
XTRN struct my_vlc_interface *PlayItemAt( PRENDERER renderer, CTEXTSTR url_name );
XTRN struct my_vlc_interface * PlayItemAtEx( PRENDERER renderer, CTEXTSTR url_name, CTEXTSTR extra_opts );
XTRN struct my_vlc_interface * PlayItemAtExx( PRENDERER renderer, CTEXTSTR url_name, CTEXTSTR extra_opts, int transparent );
XTRN void HoldUpdates( void );
XTRN void ReleaseUpdates( void );

XTRN void PlayList( PLIST files, S_32 x, S_32 y, _32 w, _32 h );
XTRN void PlaySoundFile( CTEXTSTR file );

XTRN void StopItemIn( PSI_CONTROL pc );