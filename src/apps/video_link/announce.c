/*
 *  announce.c
 *  Copyright: FortuNet, Inc.  2006
 *  Product:  BingLink      Project: alpha2server   Version: Alpha2
 *  Contributors:    Chris Bjerke, Christopher Green
 *  Depricated method of launching sound events.
 *
 */


#define USE_IMAGE_INTERFACE g.pImageInterface

#include "fbi_rep_gui.h"
#include <image.h>
#include <signal.h>
#include <system.h>
#include <sqlstub.h>

#ifdef MILK_PLUGIN
#include <../src/apps/milk/resource.h>
#include <../src/apps/milk/milk_registry.h>
#include <../src/apps/milk/milk_button.h>
#endif

#define START_ANNOUNCE 1
#define STOP_ANNOUNCE  2

#ifdef __LINUX
char cSoundFileDir[128] = "/usr/src/fortunet/alpha2/sounds";
#else
	char cSoundFileDir[128] = "c:\ftn300\etc";
#endif


int InitOpen( int );
extern int CPROC DrawAllBackgrounds( PCOMMON );
extern int ScriptCommunicator ( char*, char *, char * );

void CPROC CancleKeyStub( PTRSZVAL x, PKEY_BUTTON b)
{
      if( g.st == 1 )
      {
         HideCommon( GetKeyCommon( g.common.pkbSoundClose ) );
         RevealCommon( GetKeyCommon( g.common.pkbSoundOpen ) );
      }
      else
      {
         HideCommon( GetKeyCommon( g.common.pkbSoundOpen ) );
         RevealCommon( GetKeyCommon( g.common.pkbSoundClose ) );
      }
      DestroyFrame(&g.openpage );
   return;
}

int fnCheckData( void )
{
    char szCmd[512];
    CTEXTSTR error = NULL, result = NULL;
    char y[] = {"NONE"};
    int x;

    lprintf( "Test Sting = %s", y);
    snprintf( szCmd, sizeof(szCmd), "SELECT announcement FROM link_hall_state" );
    lprintf( " szCmd = %s", szCmd );
    if( !SQLQuery( g.odbc, szCmd, &result ) )
    {
	GetSQLError( &error );
	lprintf( "SQL Error: %s", error );
	return -1;
    }
    lprintf("Result = %s, Test = %s", result, y );
    x = strncmp( y, result, 4 );
    lprintf( "X = %d", x );
    return x;
}

int fnWriteSoundFileName( char * buf )
{
    char szSqlCmd[512];
    
    snprintf(szSqlCmd, sizeof(szSqlCmd), "UPDATE link_hall_state SET announcement =\"%s\"", buf );
#if 0    
    if( !fnCheckData() )
    {
    	SQLCommand( g.odbc, szSqlCmd );
        return 1;
    }
    return 0;
#endif
   SQLCommand( g.odbc, szSqlCmd );
   return 1;
}

void CPROC SoundKeyStub( PTRSZVAL x, PKEY_BUTTON b )
{
   int iPb = (int)x;
   int s = iPb;
   char szSoundBuf[64];
   
   char szSound[ MAXOPENSOUNDS ][20] = { 
			"announcer"
			, "carson"
			, "nicholson"
			, "stallone"
			, "stewart"
			, "swartzenegger" };
      
   switch (g.st)
   {
      case 1:
      {
         snprintf(szSoundBuf, (sizeof(szSoundBuf)), "%s_open.ogg", szSound[s]);
         xlprintf(LOG_ALWAYS)("%s is MyOpenSound", szSoundBuf);
         ScriptCommunicator( "nothing", "sound",  szSoundBuf);
            DestroyFrame(&g.openpage );
         break;
      }
      case 2:
      {
         snprintf(szSoundBuf, (sizeof(szSoundBuf)), "%s_close.ogg", szSound[s]);
         xlprintf(LOG_ALWAYS)("%s is MyCloseSound", szSoundBuf);
         ScriptCommunicator( "nothing", "sound", szSoundBuf );
            DestroyFrame(&g.openpage );
         break;
      }
      default:
      {
            DestroyFrame(&g.openpage );
         break;
      }
   }
	fnWriteSoundFileName( szSoundBuf );

}

int OpenAnnounce( int st )
{
   int iMaxFrameWidth, iMaxFrameHeight;
   
   iMaxFrameWidth = 1024;
   iMaxFrameHeight = 768;
   g.openpage = CreateFrame (""
                          , 0
			  , 0
			  , iMaxFrameWidth
			  , iMaxFrameHeight
			  , BORDER_NOMOVE | BORDER_FIXED | BORDER_NONE
			  , 0 );
   AddCommonDraw( g.openpage, DrawAllBackgrounds );
   InitOpen( st );
      DisplayFrame(g.openpage);
   return 1;
}


   char szOpenSound[ MAXOPENSOUNDS ][20] = { 
			"announcer"
			, "carson"
			, "nicholson"
			, "stallone"
			, "stewart"
			, "swartzenegger" };
   int iDefaultPos[MAXOPENSOUNDS][2] = {
			{ 88, 185 }
			, { 394, 185 }
			, { 700, 185 }
			, { 88, 400 }
			, { 394, 400 }
			, { 700, 400 } };


#ifdef MILK_PLUGIN

OnCreateMenuButton( "State Change Button" )( PMENU_BUTTON button )
{
   static int x;
   return (++x);
}

OnKeyPressEvent( "State Change Button" )( PTRSZVAL ppsv, PKEY_BUTTON key )
{
   PTRSZVAL psv = *(PTRSZVAL*)ppsv;
   lprintf( "incoming psv %p and that is %ld", ppsv, psv );
   SoundKeyStub( psv, key );
}

#endif

int InitOpen( int st )
{
   Image iNormal, iMask[MAXOPENSOUNDS];
   char szBuffer1[128], lpszEntry[128], szReturnBuffer[128];
   int retval, count, n, h, w, y, x, iXPos[MAXOPENSOUNDS], iYPos[MAXOPENSOUNDS];
   int iSXSize, iSYSize;

//   snprintf(lpszEntry, (sizeof(lpszEntry)), "SXSize", n);
   strcpy(lpszEntry, "SXSize" );
   iSXSize = FutGetPrivateProfileInt( "Sound/Positions"
					, lpszEntry
					, 180
					, lpszFilename
					);
 
//   snprintf(lpszEntry, (sizeof(lpszEntry)), "SYSize", n);
   strcpy(lpszEntry, "SYSize" );
   iSYSize = FutGetPrivateProfileInt( "Sound/Positions"
					, lpszEntry
					, 165
					, lpszFilename
					);

   count = sizeof(szReturnBuffer);

   strcpy( lpszEntry, "CancelHeight" );
   h = FutGetPrivateProfileInt( "General"
			        , lpszEntry      
				, 60
			        , lpszFilename
			        );

   strcpy( lpszEntry, "Cancelwidth" );
   w = FutGetPrivateProfileInt( "General"
			        , lpszEntry
			        , 80
			        , lpszFilename
			        );
   strcpy( lpszEntry, "CancelX" );
   x = FutGetPrivateProfileInt( "General"
 			       , lpszEntry
			       , 30
			       , lpszFilename
			       );

   strcpy( lpszEntry, "CancelY" );
   y = FutGetPrivateProfileInt( "General"
			       , lpszEntry
			       , 20
			       , lpszFilename
			       );
   g.common.pkbCancel= MakeKeyEx( g.openpage
                                , x, y
                                , w, h
                                , ENU_CancelAnnounceButton
                                , 0 //g.iGlare
                                , NULL //iNormal //g.iNormal
                                , NULL //iPressed //g.iPressed
                                , NULL //iMask //               , g.sheet[0].iMask
                                , 0 // set to image and pass image pointer for texture
                                , Color( 0x72, 0x22, 0x22 )
                                , "ABACK\0"//, szTitle
                                , /*NULL */ g.keyfont
                                , CancleKeyStub
                                , (PTRSZVAL) ENU_CancelAnnounceButton //the ptrszval
                                , (char *)0//the char*
                                );


   for (n =1; n <= MAXOPENSOUNDS; n++ )
   {
      x = n-1;
      snprintf(lpszEntry, (sizeof(lpszEntry)), "XPosition%d", n);
      iXPos[x] = FutGetPrivateProfileInt( "Sound/Positions"
					  , lpszEntry
					  , iDefaultPos[x][0]
					  , lpszFilename
					  ); 	
      snprintf(lpszEntry, (sizeof(lpszEntry)), "YPosition%d", n);
      iYPos[x] = FutGetPrivateProfileInt( "Sound/Positions"
					  , lpszEntry
					  , iDefaultPos[x][1]
					  , lpszFilename
					  ); 	
   }

   for (n =1; n <= MAXOPENSOUNDS; n++ )
   {
      x = n - 1;
      if ( st == 1 )
      { 
         g.st = st;
         snprintf(szBuffer1, (sizeof(szBuffer1)), "%s_open.png", szOpenSound[x] ); 
         xlprintf(LOG_ALWAYS)("%s is sound file name %d", szBuffer1, n);
     
	snprintf(lpszEntry, (sizeof(lpszEntry)), "OpenSound%d", n);
	retval = FutGetPrivateProfileString( "Sound/FileNames"
					  , lpszEntry
					  , szBuffer1
					  , szReturnBuffer
					  , count
					  , lpszFilename
					  );
	xlprintf(LOG_ALWAYS)("%s is close sound %d", szReturnBuffer, n);
	iMask[x] = iNormal = LoadImageFile( szReturnBuffer );


	g.pKeySoundButton[x] = MakeKeyEx( g.openpage
					  , iXPos[x], iYPos[x]
					  , iSXSize, iSYSize
					  , (PKEYBUTTONOPENSOUNDBASEID + x)
					  , 0
					  , iNormal
					  , NULL
					  , iMask[x]
					  , 0
					  , Color( 0x92, 0x8A, 0x72 )
					  , "ATitle\0"
					  , g.keyfont
					  , SoundKeyStub
					  , x
					  , 0 );
      }
      else
      {
          g.st = st;
	  snprintf(szBuffer1, (sizeof(szBuffer1)), "%s_close.png", szOpenSound[x] );
	  xlprintf(LOG_ALWAYS)("%s is sound file name %d", szBuffer1, n);
     
	  snprintf(lpszEntry, (sizeof(lpszEntry)), "CloseSound%d", n);
	  retval = FutGetPrivateProfileString( "Sound/FileNames"
					  , lpszEntry
					  , szBuffer1
					  , szReturnBuffer
					  , count
					  , lpszFilename
					  );
	  xlprintf(LOG_ALWAYS)("%s is close sound %d", szReturnBuffer, n);
	  iMask[x] = iNormal = LoadImageFile( szReturnBuffer );

          g.pKeySoundButton[x] = MakeKeyEx( g.openpage
					  , iXPos[x], iYPos[x]
					  , iSXSize, iSYSize
					  , (PKEYBUTTONCLOSESOUNDBASEID + x)
					  , 0
					  , iNormal
					  , NULL
					  , iMask[x]
					  , 0
					  , Color( 0x92, 0x8A, 0x72 )
					  , "ATitle\0"
					  , g.keyfont
					  , SoundKeyStub
					  , x
					  , 0 );
      }
   }
   return 1;
}
