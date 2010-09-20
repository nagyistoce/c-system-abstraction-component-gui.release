
#include <stdhdrs.h>
#include <sqlgetoption.h>
#include <sackcomm.h>

static TEXTCHAR comport[32];


#define CMD_DISPLAY_RESET "BD"
#define CMD_DISPLAY_CAMERA "BC#"
#define CMD_DISPLAY_RESET_CAMERA "BD\rBC#"
#define CMD_DISPLAY_PIP "BP"
#define CMD_DISPLAY_PIP0 "BP1"

static int com;


PRELOAD( init_video_controller )
{
	SACK_GetProfileStringEx( GetProgramName(), "Video Multiplexor/Com Port", "\\\\.\\com11", comport, sizeof( comport ), TRUE );

   // this is actually called 'init', which sets the port parameters...
	com = SackOpenComm( comport, 0, 0 );
	if( com < 0 )
		return;



}


void video_ctrl_display( char port )
{
	char out[sizeof( CMD_DISPLAY_RESET_CAMERA )];
   strcpy( out, CMD_DISPLAY_RESET_CAMERA );
   out[sizeof(CMD_DISPLAY_RESET_CAMERA)-2] = port;
   out[sizeof(CMD_DISPLAY_RESET_CAMERA)-1] = '\r';
   SackCommWriteBuffer( com, out, sizeof( out ), 0 );
}

void video_ctrl_pip( char port1, char port2 )
{
   //            echo -e "\r${CMD_DISPLAY_RESET_CAMERA}${2}\r${CMD_DISPLAY_PIP}\r${CMD_DISPLAY_CAMERA}${3}\r" >> $SERIAL_DEV
   //         #echo -e "\r${CMD_DISPLAY_CAMERA}${2}\r" >> $SERIAL_DEV
   //         #echo -e "\r${CMD_DISPLAY_PIP}\r" >> $SERIAL_DEV
   //         #echo -e "\r${CMD_DISPLAY_CAMERA}${3}\r" >> $SERIAL_DEV
	char msg[sizeof( CMD_DISPLAY_RESET_CAMERA )
				+ sizeof( CMD_DISPLAY_PIP )
				+ sizeof( CMD_DISPLAY_CAMERA )
			  ];
   msg[ + 0] = CMD_DISPLAY_RESET_CAMERA[0];
   msg[ + 1] = CMD_DISPLAY_RESET_CAMERA[1];
	msg[ + 2] = port1;
	msg[sizeof( CMD_DISPLAY_CAMERA )-1] = '\r';

	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + 0] = CMD_DISPLAY_PIP[0];
	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + 1] = CMD_DISPLAY_PIP[1];
	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + sizeof( CMD_DISPLAY_PIP )-1] = '\r';

	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + sizeof( CMD_DISPLAY_PIP )
		 + 0 ] = CMD_DISPLAY_CAMERA[0];
	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + sizeof( CMD_DISPLAY_PIP )
		 + 1 ] = CMD_DISPLAY_CAMERA[1];
	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + sizeof( CMD_DISPLAY_PIP )
		 + 2 ] = port2;
	msg[sizeof( CMD_DISPLAY_RESET_CAMERA )+1
		 + sizeof( CMD_DISPLAY_PIP )
		 + sizeof( CMD_DISPLAY_CAMERA ) -1 ] = '\r';

   SackCommWriteBuffer( com, msg, sizeof( msg ), 0 );
}




