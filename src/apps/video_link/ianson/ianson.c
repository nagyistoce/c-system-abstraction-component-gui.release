


#include <stdhdrs.h>
#include <network.h>
#include <sqlgetoption.h>
#include <system.h>

#include "video_ctrl.h"
//#include "../bdtat_box_control/bdata.h"

int done;
PTHREAD thread;
TEXTCHAR dsn[256];
PODBC odbc;

void CPROC capture_output( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 size )
{
	lprintf( "%s", buffer );

}

void CPROC catch_end( PTRSZVAL psv, PTASK_INFO task )
{
	// task ended... mark status?
	lprintf( "vlc(?) ended..." );
   done = 1;
   WakeThread( thread );
}

void MarkLaunched( void )
{
	SQLCommandf( NULL, "update link_hall_state join location on link_hall_state.hall_id=location.id set task_launched=0 where packed_name='%s'", GetSystemName() );
}

void MarkMasterReady( void )
{
	SQLCommandf( NULL, "update link_hall_state join location on link_hall_state.hall_id=location.id set master_ready=1 where packed_name='%s'", GetSystemName() );
}

void UnmarkEnabled( void )
{
	SQLCommandf( NULL, "update link_hall_state join location on link_hall_state.hall_id=location.id set enabled=0 where packed_name='%s'", GetSystemName() );
}

PTRSZVAL CPROC WaitForSocket( PTHREAD thread )
{
	PCLIENT pc = NULL;
	while( !pc )
	{
		pc = OpenTCPClient( "localhost:1234", 1234, NULL );
		if( !pc )
		{
         lprintf( "Not yet.." );
			WakeableSleep( 100 );
		}
	}
   MarkMasterReady();
	MarkLaunched();
   lprintf( "Okay." );
   RemoveClient( pc );
   return 0;
}

int main( int argc, char **argv )
{
   int emulate_bdata_box = SACK_GetProfileIntEx( "BingoLink/BData", "Emulate Bdata Box?", 1, TRUE );
	int use_bdata_box = emulate_bdata_box?0:SACK_GetProfileIntEx( "BingoLink/BData", "Use Bdata Box(v1)?", 0, TRUE );
	if( argc < 4 )
		return 0;


   strcpy( dsn, "MySQL" );
   odbc = ConnectToDatabase( dsn );

	if( StrCaseCmp( argv[1], "bdata_host" ) == 0 )
	{
      MarkLaunched();
	}

	else if( StrCaseCmp( argv[1], "broadcast" ) == 0 )
	{
//		DISPLAY=:0.1 exec $VLC_PATH/vlc --fullscreen -vvv v4l:/dev/video0 --sout '#transcode{vcodec=mp4v,acodec=mpga,vb=3072,height=480,width=720,ab=192,channels=2}:duplicate{dst=display,dst=standard{access=http,mux=ts,dst=0.0.0.0:1234,height=480,width=720}}' >/tmp/vlc.log 2>&1
  // 	;;
      TEXTCHAR prog_ini[256];
      TEXTCHAR path_ini[256];
      TEXTCHAR options[1024];
		CTEXTSTR program=prog_ini;
		CTEXTSTR path = path_ini;
		CTEXTSTR args[] = { "--fullscreen","-vvv","dshow://"
								, NULL//":sout=#transcode{vcodec=mp4v,vb=800,scale=1}:duplicate{dst=display,dst=std{access=http,mux=ts,dst=0.0.0.0:8080}} :sout-keep"
								, NULL };
      args[3] = options;
      SACK_GetProfileStringEx( "BingoLink/VLC", "vlc options", ":sout=#transcode{vcodec=mp4v,vb=800,scale=1}:duplicate{dst=display,dst=std{access=http,mux=ts,dst=0.0.0.0:8080}} :sout-keep", options, sizeof( options ), TRUE );
		SACK_GetProfileStringEx( "BingoLink/VLC", "program path", "c:\\tools\\vlc\\vlc.exe", prog_ini, sizeof( prog_ini ), TRUE );
		SACK_GetProfileStringEx( "BingoLink/VLC", "start-in path", ".", path_ini, sizeof( path_ini ), TRUE );
		if( LaunchPeerProgramExx( program, path, args
								  , 0
								  , capture_output
								  , catch_end
								  , 0
									DBG_SRC
										)
		  )
		{
         ThreadTo( WaitForSocket, 0 );
         thread = MakeThread();
			while( !done )
			{
				WakeableSleep( 10000 );
				MarkLaunched();
			}
		}
		else
		{
			MarkLaunched();
         SQLCommandf( NULL, "update link_state set master_hall_id=0 where bingoday=now()" );
         SQLCommandf( NULL, "update link_state set master_hall_id=0 where bingoday=now()" );
		}


	}
	else if( StrCaseCmp( argv[1], "RESETLOCAL" ) == 0 )
	{
#if 0
      // init is redundant it's a condition of the port
		$SCRIPT_PATH/videoctrl init
         // set default, display 1
        $SCRIPT_PATH/videoctrl display 1



           echo "RESETLOCALing bdata box at $4..." >> /tmp/ianson.log
			if [ -z $NO_BDATA_CTRL ]; then SCRIPT_PATH=$SCRIPT_PATH $SCRIPT_PATH/bdata_control reset $4; fi


           if [[ -e /tmp/vp_pid ]] ; then
              kill -9 $(cat /tmp/vp_pid)
              rm -f /tmp/vp_pid
           fi         
           if [[ -e /tmp/promoalreadyplayinggraphic_pid ]] ; then
              kill -9 $(cat /tmp/promoalreadyplayinggraphic_pid)
              rm -f /tmp/promoalreadyplayinggraphic_pid
              killall -9 mtpaint
				  fi


           killall -9 vlc
           SCRIPT_PATH=$SCRIPT_PATH $SCRIPT_PATH/mark_ready2 $1
#endif
			  printf( "Reset Locaalling the bdata at %s\n", argv[4] );
			  video_ctrl_display( '1' );
			  // shutdown promo player
			  // shutdown vlc
			  system( "pskill vlc" );
           SQLCommandf( NULL, "replace into link_hall_state (id,link_hall_state.hall_id,enabled,prohibited) select link_hall_state.id,location.id,enabled,prohibited from location join link_hall_state on link_hall_state.hall_id=location.id where packed_name='%s'", GetSystemName() );
           SQLCommandf( NULL, "replace into link_hall_state (id,link_hall_state.hall_id,enabled,prohibited) select link_hall_state.id,location.id,link_hall_state.enabled,link_hall_state.prohibited from location join link_hall_state on link_hall_state.hall_id=location.id where packed_name='%s'", GetSystemName() );
	}
	else if( StrCaseCmp( argv[1], "LOCAL" ) == 0 )
	{
		video_ctrl_display( '1' );
		SQLCommandf( NULL, "replace into link_hall_state (id,link_hall_state.hall_id,enabled,controller,prohibited) select link_hall_state.id,location.id,link_hall_state.enabled,link_hall_state.controller,link_hall_state.prohibited from location join link_hall_state on link_hall_state.hall_id=location.id where packed_name='%s'", GetSystemName() );
	}

   return 0;
}


