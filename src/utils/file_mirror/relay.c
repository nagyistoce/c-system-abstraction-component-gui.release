#define DO_LOGGING
#include <stdhdrs.h>
#include <time.h>
#include <netservice.h>
#include <ctype.h>
#include <stdio.h>
#include <network.h>
#include <sharemem.h>
#include <timers.h>
#include <idle.h>
#include <logging.h>
#include <filemon.h>
#include "account.h"
#include "relay.h"

#ifdef _WIN32
#include "resource.h"
#include "systray.h"
#endif

#define GetLastMsg GetNetworkLong( pc, NL_LASTMSG )
#define SetLastMsg(n) SetNetworkLong( pc, NL_LASTMSG, (n) )

PCLIENT UDPListen;
SOCKADDR *saBroadcast[2];
PCLIENT UDPBroadcast;
PCLIENT TCPClient, TCPControl;
int maxconnections;
typedef struct connection_tag {
    PCLIENT pc;
    _32     LastCommunication;
    _32     Version;
} CONNECTION, *PCONNECTION;
#define VER_CODE(major,minor) (((major)<<16)|(minor))
PCONNECTION Connection;


int bDone;
int bUseWatchdog;
int bEnableBroadcast;
int bGetWinners;
int bForceLowerCase;
int bReceiveLinkedPacket;

extern SOCKADDR *server;
extern char defaultlogin[];

_32 GetWinners;
_16 GetWinnerPort;
extern PNETBUFFER NetworkBuffers;

//---------------------------------------------------------------------------

void SendCallerStatus( PCLIENT pc )
{
    _32 msg[2];
    PNETBUFFER pCheck = NetworkBuffers;
    PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
    msg[0] = *(_32*)"CALR";
    while( pCheck )
    {
        if( account->netbuffer == pCheck )
            break;
        pCheck = pCheck->next;
    }
    if( pCheck )
    {
        if( pCheck->present )
            msg[1] = 0;
        else
            msg[1] = ( GetTickCount() - pCheck->LastMessage ) + 4000;
        SendTCP( pc, msg, 8 );
    }
    else
    {
        Log( "Could not find the network buffer for caller state for connection?!" );
    }
}

//---------------------------------------------------------------------------
int UpdateCallerStatus( PNETBUFFER pBuffer )
{
    int i;
    for( i = 0; i < maxconnections; i++ )
    {
        PACCOUNT account;
        if( !Connection[i].pc )
            continue;
        if( !NetworkLock( Connection[i].pc ) )
            continue;
        account = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
        if( account && account->netbuffer == pBuffer )
        {
            _32 msg[2];
            Log3( "Found connection listening to master caller: %s %d %ld"
                 , account->unique_name
                 , pBuffer->present
                 , ( GetTickCount() - pBuffer->LastMessage ) + 4000 );
            msg[0] = *(_32*)"CALR";
            if( pBuffer->present )
                msg[1] = 0;
            else
                msg[1] = ( GetTickCount() - pBuffer->LastMessage ) + 4000;
            SendTCP( Connection[i].pc, msg, 8 );
        }
        NetworkUnlock( Connection[i].pc );
    }
    return TRUE;
}

//---------------------------------------------------------------------------

_32 MakeVersionCode( char *version )
{
    _32 accum = 0;
    _32 code;
    while( version[0] && version[0] != '.' )
    {
        accum *= 10;
        accum += version[0] - '0';
        version++;
    }
    code = accum << 16;
    if( version[0] )
        version++;
    accum = 0;
    while( version[0] && version[0] != '.' )
    {
        accum *= 10;
        accum += version[0] - '0';
        version++;
    }
    code += accum;
    return code;
}

//---------------------------------------------------------------------------

void CPROC UDPRecieve( PCLIENT pc, POINTER buffer, int size, SOCKADDR *sa ) /*FOLD00*/
{
    int i;
    char msg[6];
    static char real_buffer[1024];
    //static int curupdate, lastsize;
    if( !buffer )
    {
        if( !size )
        {
        }
        else
        {
            //Log( "Announcing current state to client..." );
            *(_32*)msg = *(_32*)"DATA";
            {
                if( pc )
                {
                    PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                    if( account )
                    {
                        if( account->netbuffer->valid )
                        {
                            *(short*)&msg[4] = account->netbuffer->size;
                            // in theory this can get grouped together?
                            //Log1( "Writing Game packet to %s", account->unique_name );
                            if( SendTCPEx( pc, msg, 6, TRUE ) )
                            {
                                if( *(short*)&msg[4] )
                                    SendTCP( pc
                                             , account->netbuffer->buffer
                                             , account->netbuffer->size );
                            }
                        }
                    }
                    // send little endian to be nice to ourselves
                    return; // do NOT queue a UDP read here....
                }
            }
        }
    }
    else
    {
        //int bDidUpdate;
        PNETBUFFER pCheck;
        pCheck = NetworkBuffers;
        while( pCheck )
        {
            if( *(_64*)pCheck->sa == *(_64*)sa )
            {
                break;
            }
            pCheck = pCheck->next;
        }
        if( !pCheck )
        {
            Log( "Recieved a packet from a caller we weren't listening to." );
            ReadUDP( pc, real_buffer, sizeof(real_buffer) );
            return;
        }
        // if the current time elapses past this, the master
        // caller (for this buffer) has left;
        pCheck->LastMessage = GetTickCount() + 4000;
        if( !pCheck->present )
        {
            pCheck->present = TRUE;
            Log( "Master Caller has come back! There is joy..." );
            UpdateCallerStatus( pCheck );
        }
        if( size != pCheck->size
            || !pCheck->valid
            || memcmp( buffer, pCheck->buffer, size ) // not zero not match.
          )
        {
            pCheck->valid = 1;
            memcpy( pCheck->buffer, buffer, size );
            pCheck->size = size;
            for( i = 0; i < maxconnections; i++ )
            {
                if( Connection[i].pc )
                {
                    PACCOUNT account = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
                    if( account && ( account->netbuffer == pCheck ) )
                    {
                        *(_32*)msg = *(_32*)"DATA";
                        // send little endian to be nice to ourselves
                        *(short*)&msg[4] = size;
                        // in theory this can get grouped together?
                        //Log1( "Writing Game packet to %s", account->unique_name );
                        if( SendTCPEx( Connection[i].pc, msg, 6, TRUE ) )
                        {
                            SendTCP( Connection[i].pc
                                     , pCheck->buffer
                                     , pCheck->size);
                        }
                    }
                }
            }
        }
    }
    ReadUDP( pc, real_buffer, sizeof(real_buffer) );
}

//---------------------------------------------------------------------------

void CPROC UDPClose( PCLIENT pc ) /*FOLD00*/
{
    // this never happens?
    UDPListen = ServeUDP( "localhost", 3000, UDPRecieve, UDPClose );
    if( !UDPListen )
        Log( "Failed to reopen the closed UDP Listener (Relay Server)" );
}

//---------------------------------------------------------------------------

void CPROC TCPClose( PCLIENT pc ) /*FOLD00*/
{
    Log( "Client Disconnected..." );
    if( pc == TCPClient )
    {
        TCPClient = NULL;
        Logout( (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT ) );
        if( UDPBroadcast )
        {
            _32 msg[2];
            msg[0] = *(_32*)"CONN";
            msg[1] = 0;
            SendUDPEx( UDPBroadcast, msg, 8, saBroadcast[1] );
        }
        return;
    }

    {
        PCONNECTION mycon = (PCONNECTION)GetNetworkLong( pc, NL_CONNECTION );
        if( !mycon ) // wasn't open... don't bother we're done.
            return;
        if( mycon->pc == pc )
        {
            PACCOUNT myacct = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
            Logout( myacct );
            mycon->pc = NULL; // is now an available connection.
        }
        else
        {
            Log( "Fatal: Closing socket on a connection that wasn't this one..." );
        }
    }
    Release( (void*)GetNetworkLong( pc, NL_BUFFER ) );
    Log( "Client Disconnected..." );
}

//---------------------------------------------------------------------------
typedef struct mytime_tag {
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} MYTIME, *PMYTIME;

void SendTimeEx( PCLIENT pc, int bExtended ) /*FOLD00*/
{
    _32 timebuf[4];
    PMYTIME sendtime;
    if( bExtended )
    {
        timebuf[0] = *(_32*)"TIME";
        timebuf[1] = *(_32*)" NOW";
        sendtime = (PMYTIME)(timebuf+2);
    }
    else
    {
        timebuf[0] = *(_32*)"TIME";
        sendtime = (PMYTIME)(timebuf+1);
    }

#ifdef _WIN32
    {
        SYSTEMTIME st;
        GetSystemTime( &st );
        sendtime->year = st.wYear - 2000;
        sendtime->month = st.wMonth;
        sendtime->day = st.wDay;
        sendtime->hour = st.wHour;
        sendtime->minute = st.wMinute;
        sendtime->second = st.wSecond;
    }
#else
    {
        struct tm *timething;
        struct tm timebuf;
        time_t timevalnow;
        time(&timevalnow);
        timething = gmtime_r( &timevalnow, &timebuf );
        sendtime->year = timething->tm_year - 100;
        sendtime->month = timething->tm_mon+1;
        sendtime->day = timething->tm_mday;
        sendtime->hour = timething->tm_hour;
        sendtime->minute = timething->tm_min;
        sendtime->second = timething->tm_sec;
    }
#endif
    /*
     Log6( "Send time: %02d/%02d/%02d %d:%02d:%02d"
     , sendtime->month
     , sendtime->day
     , sendtime->year
     , sendtime->hour
     , sendtime->minute
     , sendtime->second );
     */
    if( bExtended )
        SendTCP( pc, timebuf, 8 + sizeof(MYTIME) );
    else
        SendTCP( pc, timebuf, 4 + sizeof(MYTIME) );
}

//---------------------------------------------------------------------------

void SetTime( char *buffer ) /*FOLD00*/
{
    PMYTIME time = (PMYTIME)buffer;
    /*
    Log6( "Got time Something like: %02d/%02d/%02d %d:%02d:%02d"
         , time->month
         , time->day
         , time->year
         , time->hour
         , time->minute
         , time->second );
    */
#ifdef _WIN32
    {
        SYSTEMTIME st, stNow;
        memset( &st, 0, sizeof( SYSTEMTIME ) );
        st.wYear = 2000 + time->year;
        st.wMonth = time->month;
        st.wDay = time->day;
        st.wHour = time->hour;
        st.wMinute = time->minute;
        st.wSecond = time->second;

        GetSystemTime( &stNow );
         if( ((stNow.wSecond - st.wSecond)*3600) + 
             ((stNow.wMinute - st.wMinute)*60) +
             ((stNow.wHour - st.wHour) < 10 ) 
           )
         {
              Log( " --- Time was quite near already" );
            return;
        }


        if( !SetSystemTime( &st ) )
        {
            MessageBox( NULL, "Failed to set the clock!", "Obnoxious box!", MB_OK );
        }
    }
#else
    {
        struct tm tmnow, tmgmt, tmlocal;
        time_t utc, local;
        struct timeval tvnow;
        tmnow.tm_year = time->year + 100;
        tmnow.tm_mon = time->month-1;
        tmnow.tm_mday = time->day;
        tmnow.tm_hour = time->hour;
        tmnow.tm_min = time->minute;
        tmnow.tm_sec = time->second;
        tvnow.tv_sec = mktime( &tmnow );
        tvnow.tv_usec = 0;
        gmtime_r( &tvnow.tv_sec, &tmgmt );
        utc = mktime( &tmgmt );
        localtime_r( &tvnow.tv_sec, &tmlocal );
        local = mktime( &tmlocal );
        tvnow.tv_sec += local - utc;
        settimeofday( &tvnow, NULL );
    }
#endif
}

//---------------------------------------------------------------------------

_32 dwLastReceive;
_32 pings_sent;

//---------------------------------------------------------------------------

void CPROC TCPRead( PCLIENT pc, POINTER buffer, int size ) /*FOLD00*/
{
    int toread = 4;
    SetNetworkLong( pc
                      , NL_LASTMSGTIME
                      , dwLastReceive = GetTickCount() );
    if( !buffer )
    {
        buffer = Allocate( 4096 );
        SetNetworkLong( pc, NL_BUFFER, (_32)buffer );
        SetLastMsg( 0 );
        SetTCPNoDelay( pc, TRUE );
    }
    else
    {
        int LogKnown = TRUE;
        // handle receive DATA, PING/PONG
        if( !GetLastMsg && size == 4 )
        {
            // should really option verbose logging or not...
            Log2( "Got Msg: %d %4.4s", size, buffer );
            ((_8*)buffer)[4] = 0;

            // caller status - when it goes away, when it comes back...
            if( *(_32*)buffer == *(_32*)"CALR" )
            {
                SetLastMsg( *(_32*)buffer );
                toread = 4;
                LogKnown = FALSE; //
            }
            else if( *(_32*)buffer == *(_32*)"OPTS" )
            {
                SetLastMsg( *(_32*)buffer );
                toread = 4;
            }
            else if( *(_32*)buffer == *(_32*)"WIN?" )
            {
                // 2.5 second delay fairly arbitrary...
                // but this at least allows multiple destinations to
                // simultaneously ask for winners and group them together.
                // since often they WILL be asking based off the same event
                // which this thing sourced.
                // delay shortened to 500 mils, this lessens the chance
                // of caller servers winning exactly the same time
                // and losing 1/2 the results.
                _32 IP = GetNetworkLong( pc, GNL_IP );
                PACCOUNT pAccount;
                Log1( "%s is asking for a winner", inet_ntoa( *(struct in_addr*)&IP ) );
                if( pAccount = (PACCOUNT)GetNetworkLong(pc, NL_ACCOUNT) )
                {
                    GetWinnerPort = ((PACCOUNT)GetNetworkLong(pc, NL_ACCOUNT))->WinnerPort;
                    GetWinners = GetTickCount() + 500;
                }
                else
                {
                    Log( "Account was not found to get the winner port for." );
                }
            }
            else if( *(_32*)buffer == *(_32*)"DATA" )
            {
                SetLastMsg( *(_32*)buffer );
                toread = 2;
                LogKnown = FALSE;
            }
            else if( *(_32*)buffer == *(_32*)"SCAN" )
            {
                PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                INDEX idx;
                PMONDIR pDir;
                LIST_FORALL( account->Monitors, idx, PMONDIR, pDir )
                {
                    MonitorForgetAll( pDir->monitor );
                }
                account->DoScanTime = GetTickCount() - 1;
            }
            else if( *(_32*)buffer == *(_32*)"PONG" )
            {
                SetNetworkLong( pc, NL_PINGS_SENT, 0 );
                pings_sent = 0;
                LogKnown = FALSE;
            }
            else if( *(_32*)buffer == *(_32*)"PING" )
            {
                SendTCP( pc, "PONG", 4 );
                LogKnown = FALSE;
            }
            else if( *(_32*)buffer == *(_32*)"FILE" ||
                      *(_32*)buffer == *(_32*)"STAT" )
            {
                // message sent when there may be a file change...
                // responce to this is SEND(from, length), NEXT, WHAT
                // Log( "Remote has a file to send..." );
                SetLastMsg( *(_32*)buffer );
                toread = 13; // length, time, pathid dword and name length byte
                LogKnown = FALSE;
            }
            else if( *(_32*)buffer == *(_32*)"FDAT" ||
                      *(_32*)buffer == *(_32*)"CDAT" )
            {
                //Log( "Receiving file data..." );
                LogKnown = FALSE;
                SetLastMsg( *(_32*)buffer );
                toread = 8;
            }
            else if( *(_32*)buffer == *(_32*)"SEND" ||
                      *(_32*)buffer == *(_32*)"CSND"  )
            {
                // message sent indicating that the file is not current
                // and bytes (from) for (length) should be sent...
                // if from == EOF and length == 0 the file is up to date
                toread = 8; // get amount of data to send...
                LogKnown = FALSE;
                SetLastMsg( *(_32*)buffer );
            }
            else if( ( *(_32*)buffer == *(_32*)"NEXT" ) ||
                      ( *(_32*)buffer == *(_32*)"WHAT") )
            {
                PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                INDEX idx;
                PMONDIR pDir;
                int changes = 0;
                {
                    if( *(_32*)buffer == *(_32*)"WHAT")
                        Log( "The server has no idea what to do with specified file." );
                }
                LIST_FORALL( account->Monitors, idx, PMONDIR, pDir )
                {
                    if( changes = DispatchChanges( pDir->monitor ) )
                        break;
                }
                if( !changes )
                {
                   Log( "Accouncing DONE..." );
                    SendTCP( pc, "DONE", 4 ); // tell him we're done so he can close files
                }
                LogKnown = FALSE;
            }
            else if( *(_32*)buffer == *(_32*)"DONE" )
            {
                //Log( "Client has no further changes to report." );
                CloseCurrentFile( (PACCOUNT)GetNetworkLong( pc, NL_DATAMIRROR ) );
                LogKnown = FALSE;
            }
            else if( *(_32*)buffer == *(_32*)"KILL" )
            {
                LogKnown = FALSE;
                toread = 5; // 4 bytes of pathID 1 byte name length
                SetLastMsg( *(_32*)buffer );
            }
            else if( *(_32*)buffer == *(_32*)"USER"
                      || *(_32*)buffer == *(_32*)"VERS"
                      || *(_32*)buffer == *(_32*)"MARQ"
                      || *(_32*)buffer == *(_32*)"WNRS" )
            {
                // message indicates the user identification of the
                // agent connecting.... this updates his profile to
                // select where his data gets mirrored...
                LogKnown = FALSE;
                toread = 1;
                SetLastMsg( *(_32*)buffer );
            }
            else if( *(_32*)buffer == (*(_32*)"TIME") )
            {
                toread = sizeof(MYTIME);
                SetLastMsg( *(_32*)buffer );
            }
            else if( *(_32*)buffer == *(_32*)"OKAY" )
            {
                PACCOUNT login;
                // login takes care of launching file monitors now.
                login = Login( pc, "datamirror", GetNetworkLong( pc, GNL_IP ) ); // 0 is dwIP .. get that..
                if( login )
                {
                    SetNetworkLong( pc, NL_DATAMIRROR, (_32)login );
                }
                else
                {
                    Log( "datamirror account is not configured!" );
                    SendTCP( pc, "QUIT", 4 );
                    RemoveClient( pc );
                    return;
                }
            }
            else if( *(_32*)buffer == *(_32*)"EXIT" )
            {
                // Bad logins can cause this message...
                Log( "Login failure... instructed to exit..." );
                exit(0);
            }
            else
            {
                // complain unknown message or something....
                Log3( "%s Unknown message received size 4 %08lx %s"
                     , ((PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT ))->unique_name
                     , *(_32*)buffer
                     , (TEXTSTR)buffer);
                TCPDrainEx( pc, 0, FALSE ); // drain any present data...
            }
            if( LogKnown )
            {
                Log2(  "Known message received size 4 %08lx %s"
                     , *(_32*)buffer
                     , (TEXTSTR)buffer);

            }
        }
        else
        {
            _32 *buf = (_32*)buffer;
            static _32 filesize, filestart, filetime, filepath;
            if( !GetLastMsg )
            {
                Log2( "Invalid Message recieved: %d no last message buffer:%s", size, (char*)buffer );
            }
            else if( ( GetLastMsg == (*(_32*)"OPTS" ) ) )
            {
                // toread will ahve already been set to 4 - unless some option
                // indicates more data.....
                if( *(_32*)buffer == *(_32*)":win" )
                {
                    Log( "Windows remote - setting force lower case..." );
                    // and well - there's not much else to do about a windows thing on the other side.
                    bForceLowerCase = TRUE;
                }
                else if( *(_32*)buffer == *(_32*)":end" )
                {
                    Log( "Received last option - clear last msg" );
                    // last option - clear last message...
                    SetLastMsg( 0 );
                }
                else
                {
                    Log1( "Unknown option: %4.4s - continuing to read options", buffer );
                }
                goto next_read;
            }
            else if( (GetLastMsg == *(_32*)"DATA") )
            {
                toread = *(_16*)buffer;
                SetLastMsg( GetLastMsg + 1 );
                if( toread )
                {
                    ReadTCPMsg( pc, buffer, toread );
                    return;
                }
            }
            else if( UDPBroadcast && GetLastMsg == (*(_32*)"DATA")+1 )
            {
                Log( "Received completed data packet to send." );
                if( !SendUDPEx( UDPBroadcast, buffer, size, saBroadcast[1] ) )
                {
                    Log( "Failed to broadcast to SplashMan...." );
                }
                if( !SendUDPEx( UDPBroadcast, buffer, size, saBroadcast[0] ) )
                {
                    Log( "Failed to broadcast to caller...." );
                }
            }
            else if( GetLastMsg == *(_32*)"SEND" )
            {
                _32 *buf = (_32*)buffer;
                // 8 bytes in buffer are position and length of data to send...
                {
                    PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                    if( account )
                    {
                        Log3( "%s: Send Change( %ld, %ld )", account->unique_name, buf[0], buf[1] );
                    }
                    else
                    {
                        Log2( "<no account>: Send Change( %ld, %ld )", buf[0], buf[1] );
                    }
                    if( !SendFileChange( pc, account->LastFile, buf[0], buf[1] ) )
                    {
                        Log1( "Fatality - current monitor was lost on %s", account->unique_name );
                        RemoveClient( pc );
                        return;
                    }
                }
            }
            else if( GetLastMsg == *(_32*)"FDAT" )
            {
                filestart = buf[0];
                filesize = buf[1];
                SetLastMsg( GetLastMsg + 1 );
                if( filesize )
                    ReadTCPMsg( pc
                                 , GetAccountBuffer( (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT ) )
                                 , filesize );
                else
                    TCPRead( pc, buffer, 0 );
                return;
            }
            else if( GetLastMsg == (*(_32*)"FDAT")+1 )
            {
                Log2( "Writing data received into file... FPI:%ld filesize:%ld", filestart, filesize );
                UpdateAccountFile( (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT )
                                      , filestart
                                      , filesize );
            }
            else if( GetLastMsg == *(_32*)"FILE" ||
                      GetLastMsg == *(_32*)"STAT" )
            {
                filesize = buf[0];
                filetime = buf[1];
                filepath = buf[2];
                SetLastMsg( GetLastMsg + 1 );
                ReadTCPMsg( pc, buffer, *(char*)(buf+3) );
                return;
            }
            else if( GetLastMsg == (*(_32*)"FILE")+1 )
            {
                P_32 crc = Allocate( toread = ( sizeof( _32 ) *
                                                         (( filesize + 4095 ) / 4096 )) );
                ((char*)buffer)[size] = 0; // nul terminate string...
                SetLastMsg( GetLastMsg + 1 );
                if( toread )
                {
                    //Log1( "Read CRCs (%d)", toread );
                    ReadTCPMsg( pc, crc, toread );
                    return;
                }
                else
                {
                    //Log( "Read CRCs(0)!!!!!!" );
                }
                buffer = crc;
                size = toread;
                goto do_account;
            }
            else if( GetLastMsg == (*(_32*)"FILE")+2 )
            {
                // actually coincidentally the file buffer 'buffer' is still
                // correct for this...
            do_account:
                {
                    PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                    P_8 realbuffer = (P_8)GetNetworkLong( pc, NL_BUFFER );
                    OpenFileOnAccount( account
                                          , filepath
                                          , realbuffer // network buffer
                                          , filesize
                                          , filetime
                                          , buffer    // CRC buffer
                                          , size / 4  // count of CRC things
                                          );
                    Release( buffer ); // is actually CRC buffer, real network will be used next.
                    // then by default to read is correct.
                    //
                }
            }
            else if( GetLastMsg == (*(_32*)"STAT")+1 )
            {
                // now - should have total info to perform a stat type operation...
                // PathID comes from outgoing, filename result either triggers
                // a FILE or a KILL result.  This will have to be hung on the list
                // of pending changes....
                PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                ((char*)buffer)[size] = 0; // nul terminate string...
                Log5( "%s Stat file: %s/%s size: %d time: %d"
                     , account->unique_name
                     , ((PDIRECTORY)GetLink( &account->Directories, filepath ))->path
                     , buffer, filesize, filetime );
                // this is an outgoing monitor therefore 0 based.
                {
                    PMONDIR pDir = GetLink( &account->Monitors, filepath );
                    AddMonitoredFile( pDir->pHandler, buffer );
                    // set next scantime to 500 milliseconds - will allow
                    // these guys to get all their changes...
                    //Log( "Okay and - added that file to the monitor..." );
                    // ask for the next file.
                    SendTCP( pc, "NEXT", 4 );
                }
            }
            else if( GetLastMsg == *(_32*)"KILL" )
            {
                filepath = buf[0];
                toread = *(_8*)(buf+1);
                SetLastMsg( GetLastMsg + 1 );
                if( toread )
                {
                    ReadTCPMsg( pc, buffer, toread );
                    return;
                }
                else
                {
                    Log( "No filename associated with KILL" );
                    SendTCP( pc, "WHAT", 4 );
                }
            }
            else if( GetLastMsg == *(_32*)"USER"
                      || GetLastMsg == *(_32*)"VERS"
                      || GetLastMsg == *(_32*)"MARQ"
                      || GetLastMsg == *(_32*)"WNRS" )
            {
                toread = *(_8*)buffer;
                SetLastMsg( GetLastMsg + 1 );
                if( toread )
                {
                    ReadTCPMsg( pc, buffer, toread );
                    return;
                }
            }
            else if( GetLastMsg == (*(_32*)"WNRS")+1 )
            {
                //char msg[256];
                TEXTSTR buf = (TEXTSTR)buffer;
                buf[size] = 0;
#ifdef _WIN32
                //sprintf( msg, "Relay received winner report:\n%s", buf );
                //MessageBox( NULL, msg, "debug", MB_OK );
#endif
            }
            else if( GetLastMsg == (*(_32*)"VERS")+1 )
            {
                char *version = buffer;
                _32 version_code;
                version[size] = 0;
                version_code = MakeVersionCode( version );
                if( version_code < VER_CODE(2,1) )
                {
                    Log( "Version is below acceptable number. Telling client to exit..." );
                    SendTCP( pc, "EXIT", 4 );
                    return;
                }
                SetNetworkLong( pc, NL_VERSION, version_code );
            }
            else if( GetLastMsg == (*(_32*)"USER")+1 )
            {
                PACCOUNT login;
                ((char*)buffer)[size] = 0; // null terminate text...
                login = Login( pc, buffer, GetNetworkLong( pc, GNL_IP ) ); // 0 is dwIP .. get that..
                if( !login )
                {
                    Log( "Login failed? Telling client to exit..." );
                    SendTCP( pc, "EXIT", 4 );
                }
                else
                {
                    Log( "We accepted that client..." );
                    SendTCP( pc, "OKAY", 4 );
                    SetNetworkLong( pc, NL_ACCOUNT, (_32)login );
                    SendTimeEx( pc, FALSE );
                    // special case to send current state to the currently
                    // connecting network client...
                    UDPRecieve( pc, NULL, 1, NULL );
                    SendCallerStatus( pc );
                    // for all incoming directories - scan them, and report
                    // for 'stat' of files.
                }
            }
            else if( GetLastMsg == (*(_32*)"KILL") + 1 )
            {
                // at this point we should have a valid filename on
                // NL_ACCOUNT relative to remove...
                char filename[256];
                PACCOUNT account = (PACCOUNT)GetNetworkLong( pc, NL_ACCOUNT );
                ((P_8)buffer)[size] = 0;
                {
                    extern int bForceLowerCase;
                    if( bForceLowerCase )
                    {
                        char *fname;
                        for( fname = buffer; fname[0]; fname++ )
                            fname[0] = tolower( fname[0] );
                    }
                }
                sprintf( filename, "%s/%s"
                         , ((PDIRECTORY)GetLink( &account->Directories, filepath ))->path
                         , buffer );
                Log2( "%s is deleting %s", account->unique_name, filename );
                if( remove( filename ) < 0 )
                    Log1( "Failed while deleting file %s", filename );
                SendTCP( pc, "NEXT", 4 );
            }
            else if( GetLastMsg == (*(_32*)"TIME") )
            {
                SetTime( buffer );
            }
            else if( GetLastMsg == (*(_32*)"QUIT") )
            {
                Log( "Connection gave up and quit on me!" );
            }
            else
            {
                int tmp = GetLastMsg;
                Log4( "Unknown message %4.4s: %d bytes %08lx %s"
                     , &tmp
                     , size
                     , *(_32*)buffer
                     , (TEXTSTR)buffer );
                Log( "Unknown message closing..." );
                RemoveClient( pc ); // bad state
            }
            toread = 4;
            SetLastMsg( 0 );
        }
    }
next_read:
    // normally this would be a VERY bad thing to do...
    if( !toread )
        TCPRead( pc, buffer, 0 );
    ReadTCPMsg( pc, (POINTER)GetNetworkLong( pc, NL_BUFFER ), toread );
}

//---------------------------------------------------------------------------

void CPROC TCPControlClose( PCLIENT pc ) /*FOLD00*/
{
    Log( "Remote Control closed connection..." );
}

//---------------------------------------------------------------------------

void *DoUpdate( void *data ) /*FOLD00*/
{
    // PCONNECTION Connection = (PCONNECTION)data;
    // now we get to scan all files and do updates for this connection...
    return NULL;
}

//---------------------------------------------------------------------------

void CPROC ReadWinnersController( PCLIENT pc, POINTER buffer, int size )
{
    if( !buffer )
    {
        buffer = Allocate( 1024 );
    }
    else
    {
        PCLIENT pccontrol = (PCLIENT)GetNetworkLong( pc, 0 );
        char *buf = buffer;
        int c, len, end;
        buf[size] = 0;
        Log2( "Got a message sized for controller %d: %s", size, buf );

        for( c = 0; c < size; c++ )
        {
            if( buf[c] == '*' )
            {
                //len = ( size - c ) - 1;
                end = c;
                while( buf[end] >= 32 )
                    end++;
                len = end-c;
                if( pccontrol )
                {
                    SendTCP( pccontrol, "WINNERS:", 8 );
                    SendTCP( pccontrol, &len, 1 );
                    SendTCP( pccontrol, buf+c+1, len & 0xFF );
                    SendTCP( pccontrol, "ALL DONE", 8 );
                }
                RemoveClient( pc );
                return;
            }
        }
    }
    ReadTCP( pc, buffer, 1024 );
}

//---------------------------------------------------------------------------

void CPROC TCPControlRead( PCLIENT pc, POINTER buffer, int size ) /*FOLD00*/
{
    int toread = 8;
    if( !buffer )
    {
        buffer = Allocate( 128 );
    }
    else
    {
        Log2( "Control Message: %8.8s (%d)", buffer, size );
        ((char*)buffer)[size] = 0;
        if( *(_64*)buffer == *(_64*)"DIE NOW!" )
        {
            int i;
            Log( "Instructed to die, and I shall do so.\n" );
            if( UDPListen )
                RemoveClient( UDPListen );
            if( TCPClient )
                RemoveClient( TCPClient );
            for( i = 0; i < maxconnections; i++ )
            {
                if( Connection[i].pc )
                    RemoveClient( Connection[i].pc );
            }
            RemoveClient( pc );
				//while( ProcessNetworkMessages() );
            Idle(); // process net messages should never be called directly, anymore
            bDone = TRUE;
        }
        else if( *(_64*)buffer == *(_64*)"UPDATE  " )
        {
            char *user = (char*)buffer+8;
            //int len;
            //char msg[512];
            //int i;
            Log1( "Instructed to update for %s", user[0]?user:"everyone" );
            Log( "Common updates are not done at the moment..." );
            SendTCP( pc, "ALL DONE", 8 );
            toread = 1;
        }
        else if( *(_64*)buffer == *(_64*)"GET TIME" )
        {
            Log( "Someone's asking for time... " );
            SendTimeEx( pc, TRUE );
        }
        else if( *(_64*)buffer == *(_64*)"WINNERS!" )
        {
            Log( "Supposed to do an update of winners NOW" );
            Log( "Can no longer get winners - have to specify a 'region' to udpate from or something" );
            //GetWinners = GetTickCount();
            SendTCP( pc, "ALL DONE", 8 );
        }
        else if( *(_64*)buffer == *(_64*)"WINNERS?" )
        {
            //PACCOUNT account  = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
            {
                PCLIENT pcWinCheck = OpenTCPClientEx( "localhost", 3033, ReadWinnersController, NULL, NULL );
                if( !pcWinCheck )
                {
                    int len;
                    char buf[256];
                    Log( "Log server is down... report error.." );
                    len = sprintf( buf, "Winner,report,agent,down." );

                    SendTCP( pc, "WINNERS:", 8 );
                    SendTCP( pc, &len, 1 );
                    SendTCP( pc, buf, len );
                }
                else
                    SetNetworkLong( pcWinCheck, 0, (PTRSZVAL)pc );
            }
            {
                PCLIENT pcWinCheck = OpenTCPClientEx( "localhost", 3034, ReadWinnersController, NULL, NULL );
                if( !pcWinCheck )
                {
                    int len;
                    char buf[256];
                    Log( "Log server is down... report error.." );
                    len = sprintf( buf, "Winner,report,agent,down." );

                    SendTCP( pc, "WINNERS:", 8 );
                    SendTCP( pc, &len, 1 );
                    SendTCP( pc, buf, len );
                }
                else
                    SetNetworkLong( pcWinCheck, 0, (PTRSZVAL)pc );
            }
        }
        else if( *(_64*)buffer == *(_64*)"MASTER??" )
        {
            Log( "Responding with Game master status... " );
            SendTCP( pc, "MESSAGE!\x1aWhich Master status?? N/A ", 35 );
            SendTCP( pc, "ALL DONE", 8 );
        }
        else if( *(_64*)buffer == *(_64*)"LISTUSER" )
        {
            char result[1024]; // INSUFFICENT for large scale!
            PACCOUNT account;
            int i;
            int ofs = 0;
            //Log1( "Listing users... %d", maxconnections );
            SendTCP( pc, "USERLIST", 8 );
            ofs = 0;
            for( i = 0; i < maxconnections; i++ )
            {
                if( Connection[i].pc )
                {
                    char version[64];
                    _32 version_code = GetNetworkLong( Connection[i].pc, NL_VERSION);
                    if( !version_code )
                        strcpy( version, "unknown" );
                    else
                        sprintf( version, "%ld.%ld"
                                 , version_code >> 16
                                 , version_code & 0xFFF );
                    account = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
                    if( account )
                    {
                        _32 IP = GetNetworkLong( Connection[i].pc, GNL_IP );
                        //Log1( "User %s connected...", account->unique_name );
                        ofs += sprintf( result+ofs, "%s (%d) %s %s %s\n"
                                       , account->unique_name
                                       , account->logincount
                                       , version
                                       , inet_ntoa( *(struct in_addr*)&IP )
                                       , GetNetworkLong( Connection[i].pc, NL_SYSUPDATE )?"(U)":"" );
                    }
                    else
                    {
                        Log( "No account on connection!?!?!" );
                    }
                }
            }
            SendTCP( pc, &ofs, 2 );
            SendTCP( pc, result, ofs );
        }
        else if( *(_64*)buffer == *(_64*)"MEM DUMP" )
        {
            DebugDumpMemFile( "Memory.Dump" );
            SendTCP( pc, "ALL DONE", 8 );
        }
        else if( *(_64*)buffer == *(_64*)"KILLUSER" )
        {
            // need to adjust the receive here....
               char *user = (char*)buffer+8;
                int i;
            for( i = 0; i < maxconnections; i++ )
            {
               if( Connection[i].pc )
                    {
                        PACCOUNT account;
                        account = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
                        if( strcmp( account->unique_name, user ) == 0 )
                        {
                            RemoveClient( Connection[i].pc );
                            SendTCP( pc, "MESSAGE!\x1aUser connection terminated", 35 );
                            SendTCP( pc, "ALL DONE", 8 );
                            break;
                        }
                    }
                }
                if( i == maxconnections )
                    RemoveClient( pc );
          }
          else if( *(_64*)buffer == *(_64*)"DO SCAN!" )
             {
                 char *user;
                 int i;
                 if( size > 8 )
                     user = (char*)buffer + 8;
                 else
                     user = NULL;
                 if( user )
                     Log1( "Instructed to do scan for %s", user );
                 for( i = 0; i < maxconnections; i++ )
                 {
                     if( Connection[i].pc )
                     {
                         PACCOUNT account;
                         account = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
                         if( account )
                         {
                             if( !user || strcmp( account->unique_name, user ) == 0 )
                             {
                                 char msg[256];
                                 PMONDIR pDir;
                                 INDEX idx;
                                 int len;
                                 Log1( "Account for %s found...", account->unique_name );
                                 SendTCP( Connection[i].pc, "SCAN", 4 );
                                 len = sprintf( msg + 9, "Client %s found, issued scan.", account->unique_name );
                                 len += sprintf( msg, "MESSAGE!%c", len );
                                 msg[9] = 'C';
                                 SendTCP( pc, msg, len );
                                 LIST_FORALL( account->Monitors, idx, PMONDIR, pDir )
                                 {
                                     MonitorForgetAll( pDir->monitor );
                                 }
                                 account->DoScanTime = GetTickCount() - 1;
                                 if( user )
                                 {
                                     SendTCP( pc, "ALL DONE", 8 ); // having problems closing this - messages don't get out
                                     break;
                                 }
                             }
                         }
                     }
                 }
                 SendTCP( pc, "ALL DONE", 8 ); // having problems closing this - messages don't get out
             }
          else if( *(_64*)buffer == *(_64*)"REBOOT!!" )
            {
               char *user = (char*)buffer+8;
                int i;
                Log1( "Instructed to do reboot for %s", user );
            for( i = 0; i < maxconnections; i++ )
            {
               if( Connection[i].pc )
               {
                PACCOUNT account;
                        account = (PACCOUNT)GetNetworkLong( Connection[i].pc, NL_ACCOUNT );
                        if( account )
                        {
                            if( strcmp( account->unique_name, user ) == 0 )
                            {
                                char msg[256];
                                int len;
                                Log1( "Account for %s found...", account->unique_name );
                                SendTCP( Connection[i].pc, "REBT", 4 );
                                len = sprintf( msg+9, "Client found, issued reboot." );
                                len += sprintf( msg, "MESSAGE!%c", len );
                                msg[9] = 'C';
                                SendTCP( pc, msg, len );
                                SendTCP( pc, "ALL DONE", 8 );
                                break;
                            }
                        }
                    }
                }
                if( i == maxconnections )
                    RemoveClient( pc );
            }
        else
        {
               Log1( "Unknown message from controller: %8.8s", buffer );
               RemoveClient( pc );
            //exit(0);
        }
    }
    ReadTCP( pc, buffer, 128 ); // should be okay...
    //Log( "Read enabled on Control Connection!" );
}

//---------------------------------------------------------------------------

void CPROC TCPControlConnect( PCLIENT pServer, PCLIENT pNew ) /*FOLD00*/
{
   // should wait for a command here...
   // since this control port could be useful for things like 'update common'
   // update self...
   Log( "Control program attached!" );
   SetTCPNoDelay( pNew, TRUE );
   SetReadCallback( pNew, TCPControlRead );
}


//---------------------------------------------------------------------------

void CPROC TCPConnection( PCLIENT pServer, PCLIENT pNew ) /*FOLD00*/
{
    int i;
    for( i = 0; i < maxconnections; i++ )
    {
        if( pNew == Connection[i].pc )
            Log( "We got a second instance of a client we KNEW about!\n" );
    }
    for( i = 0; i < maxconnections; i++ )
    {

        if( !Connection[i].pc )
        {
            _32 dwIP = GetNetworkLong( pNew, GNL_IP );
            Log2( "New client %d %s", i, inet_ntoa( *(struct in_addr*)&dwIP ) );
            SetNetworkLong( pNew, NL_CONNECTION, (_32)(Connection + i) );
            Connection[i].LastCommunication = GetTickCount();
            Connection[i].pc = pNew;
            SetCloseCallback( pNew, TCPClose );
            SetReadCallback( pNew, TCPRead );
            Log( "Okay... set up everything and going to continue... " );
            break;
        }
    }
    if( i == maxconnections )
    {
       Log( "Too many connections!" );
       RemoveClient( pNew );
    }
}

//---------------------------------------------------------------------------

void CPROC ReadWinners( PCLIENT pc, POINTER buffer, int size )
{
    if( !buffer )
    {
      // /echo *%(winners)lastsession winners: %(winners)lastwinners
            Log( "Sending request for winners..." );
        buffer = Allocate( 1024 );
    //  SendTCP( pc, "/echo *%%(winners)lastsession winners: %%(winners)lastwinners\r"
    //                  , 60 );
    }
    else
   {
       char *buf = buffer;
       int c, i, len, end;
       buf[size] = 0;
       Log2( "Got a message sized %d: %s", size, buf );
       for( c = 0; c < size; c++ )
       {
           if( buf[c] == '*' )
           {
               //len = ( size - c ) - 1;
               end = c;
               while( buf[end] >= 32 )
                  end++;
               len = end-c;
               for( i = 0; i < maxconnections; i++ )
               {
                   PCLIENT pcsend = NetworkLock( Connection[i].pc );
                   PACCOUNT account;
                   if( pcsend )
                   {
                       account = (PACCOUNT)GetNetworkLong( pcsend, NL_ACCOUNT );
                       if( account && ( account->WinnerPort == GetWinnerPort ) )
                       {
                                 if( GetNetworkLong( Connection[i].pc, NL_VERSION ) >= VER_CODE(1,11) )
                                 {
                                    SendTCP( pcsend, "WNRS", 4 );
                                    SendTCP( pcsend, &len, 1 );
                                    SendTCP( pcsend, buf+c+1, len & 0xFF );
                                 }
                                 else
                                 {
                           SendTCP( pcsend, "MARQ", 4 );
                           SendTCP( pcsend, &len, 1 );
                           SendTCP( pcsend, buf+c+1, len & 0xFF );
                         }
                       }
                       NetworkUnlock( pcsend );
                   }
               }
               RemoveClient( pc );
               return;
           }
       }
    }
    ReadTCP( pc, buffer, 1024 );
}

//---------------------------------------------------------------------------

int bServerRunning;

PTRSZVAL CPROC ServerTimerProc( PTHREAD unused )
{
   bServerRunning = 1;
    while(1)
    {
        int i;
        PNETBUFFER pCheck = NetworkBuffers;

        if( bReceiveLinkedPacket )
            while( pCheck )
            {
                // if the caller is already absent (!present) don't check his time.
                if( pCheck->present && GetTickCount() > pCheck->LastMessage )
                {
                    Log( "A caller has dissappeared.  Setting status." );
                    pCheck->present = 0;
                    UpdateCallerStatus( pCheck );
                }
                pCheck = pCheck->next;
            }


        if( bGetWinners && GetWinners && ( GetTickCount() > GetWinners ) )
        {
            PCLIENT pc = OpenTCPClientEx( "localhost", GetWinnerPort, ReadWinners, NULL, NULL );
            if( !pc )
            {
                for( i = 0; i < maxconnections; i++ )
                {
                    PCLIENT pcping = NetworkLock( Connection[i].pc );
                    if( pcping )
                    {
                        PACCOUNT account = (PACCOUNT)GetNetworkLong( pcping, NL_ACCOUNT );
                        if( account )
                        {
                            if( account->WinnerPort == GetWinnerPort )
                                SendTCP( pcping, "MARQ\x10" "Winners: unknown", 21 );
                        }
                        NetworkUnlock( pcping );
                    }
                }
            }
            GetWinners = 0;
        }

        for( i = 0; i < maxconnections; i++ )
        {
            // if the lock fails it's no longer active.
            PCLIENT pcping = NetworkLock( Connection[i].pc );
            if( pcping )
            {
                // a slightly longer time here will prevent
                // both the client and the server reverse pinging...
                // worst case we should never expect more than
                // really 2 pings (3rd fail) but we pad an extra
                // just to be certain....
                if( ( GetNetworkLong( pcping, NL_LASTMSGTIME ) + 13000 )
                    < GetTickCount() )

                {
                    if( GetNetworkLong( pcping, NL_PINGS_SENT ) > 4 )
                    {
                        Log2( "Connection[%d] %s is not responding... closing", i,
                                ((PACCOUNT)GetNetworkLong( pcping, NL_ACCOUNT ))->unique_name );
                        SetNetworkLong( pcping, NL_PINGS_SENT, 0 );
                        RemoveClient( pcping );

                    }
                    else
                    {
                        SetNetworkLong( pcping, NL_LASTMSGTIME, GetTickCount() );
                        SetNetworkLong( pcping, NL_PINGS_SENT
                                          , GetNetworkLong( pcping, NL_PINGS_SENT ) + 1 );
                        SendTCP( pcping, "PING", 4 );
                    }
                }
                NetworkUnlock( pcping );
            }
        }
        Sleep( 2000 );
    }
   bServerRunning = 0;
   return 0;
}

//---------------------------------------------------------------------------
int bClientRunning;

PTRSZVAL CPROC ClientTimerProc( PTHREAD thread )
{
   bClientRunning = 1;
    while( 1 )
    {
#ifdef _WIN32
        if( bDone )
        {
            PostQuitMessage( 0 );
            break;
        }
#endif
        if( !TCPClient )
        {
            TCPClient = OpenTCPClientAddrEx( server
                                                     , TCPRead
                                                     , TCPClose
                                                     , NULL );
            if( TCPClient )
            {
                char msg[256];
                int len;
                if( NetworkLock( TCPClient ) )
                {
                    Log1( "Connected, and logging in as %s", defaultlogin );
                    len = sprintf( msg, "VERS%c%sUSER%c%s"
                                     , strlen( RELAY_VERSION ), RELAY_VERSION
                                     , strlen( defaultlogin ), defaultlogin );
                    SendTCP( TCPClient, msg, len );
#ifdef _WIN32
                    Log( "Sending options" );
                    // format of options ...
                    // 4 characters (a colon followed by 3 is a good format)
                    // the last option must be ':end'.
                    // SO - tell the other side we're a windows system and are going to
                    // give badly cased files - set forcelower, and this side will do only
                    // case-insensitive comparisons on directories and names given.
                    len = sprintf( msg, "OPTS:win:end" );
                    Log2( "Sending message: %s(%d)", msg, len );
                    SendTCP( TCPClient, msg, len );
#endif
                    SendTCP( TCPClient, "WIN?", 4 );
                    if( UDPBroadcast )
                    {
                        _32 msg[2];
                        msg[0] = *(_32*)"CONN";
                        msg[1] = 1;
                        SendUDPEx( UDPBroadcast, msg, 8, saBroadcast[1] );
                    }
                    NetworkUnlock( TCPClient );
                }
            }
            else
            {
                Log( "Failed to open the socket?" );
            }
        }
        else
        {
            if( !NetworkLock( TCPClient ) )
                return 0;
            if( ( GetNetworkLong( TCPClient, NL_LASTMSGTIME ) + 10000 )
                < GetTickCount() )
            {
                if( GetNetworkLong( TCPClient, NL_PINGS_SENT ) > 10 )
                {
                    Log( "Connection is not responding... closing" );
                    SetNetworkLong( TCPClient, NL_PINGS_SENT, 0 );
                    pings_sent = 0;
                    RemoveClient( TCPClient );
                    // this removes TCPClient which causes an error in the network unlock.
                }
                else
                {

                    SetNetworkLong( TCPClient, NL_LASTMSGTIME, GetTickCount() );
                    dwLastReceive = GetTickCount();

                    SetNetworkLong( TCPClient, NL_PINGS_SENT
                                      , GetNetworkLong( TCPClient, NL_PINGS_SENT ) + 1 );
                    pings_sent++;
                    SendTCP( TCPClient, "PING", 4 );
                    //Log( "Sending a ping...." );
                    NetworkUnlock( TCPClient );
                }
            }
            else
                NetworkUnlock( TCPClient );

        }
        WakeableSleep( 2000);
    }
    bClientRunning = 0;
    return 0;
}


#ifdef _WIN32
int WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nCmdShow )
{
   int argc;
   char *argv[10];
   //char msg[256];
    char *configname;
   PTHREAD pServer, pClient;
   argv[0] = "relay.exe";
   argv[1] = lpCmd;
   for( argc = 1; argv[argc]; )
   {
      //sprintf( msg, "Getting %d terminating %d : %s", argc, argc-1, argv[argc-1] );
      //MessageBox( NULL, msg, "Delete Me", MB_OK );
      ++argc;
      argv[argc] = strchr( argv[argc-1], ' ' );
      if( argv[argc] )
      {
         argv[argc][0] = 0;
         while( argv[argc][0] == ' ' ) argv[argc]++;
         if( !argv[argc][0] )
            argv[argc] = NULL;
      }
		else
		{
			argc--; // subtract one...
         argv[argc] = NULL;
			break;
		}
   }

   //sprintf( msg, "argc = %d %s %s %s %s", argc, argv[0], argv[1], argv[2], argv[3] );
   //MessageBox( NULL, msg, "Delete Me", MB_OK );
#else
int main( char argc, char **argv ) /*FOLD00*/
{
   char *configname;
	PTHREAD pServer, pClient;
#endif
	if( argc < 2 )
		configname = "Accounts.Data";
   else
		configname = argv[1];
	SetCriticalLogging( TRUE );
	//SetAllocateLogging( TRUE );
	SetAllocateDebug( FALSE ); // make sure this is enabled
	SystemLogTime( SYSLOG_TIME_HIGH|SYSLOG_TIME_DELTA );

#ifndef _WIN32
	// we're a way friendly data only transport...
	umask( 0000 );
#endif
	ReadAccounts( configname );

	// 10 minimum for a client...
	// one send broadcast
	// one control service
	// one open to master server
	// one watchdog discover
	// one local watchdog
	// one pos watchdog
	// servers should have lots...
	if( !maxconnections )
	{
		Log( "You may want to specify connections = in the Accounts.Data file\n" );
		maxconnections = 256;
	}

	Connection = Allocate( sizeof( Connection[0] ) * maxconnections );
	MemSet( Connection, 0, sizeof( Connection[0] ) * maxconnections );
	if( !NetworkWait( 0, maxconnections+3, NUM_NETLONG ) )
	{
		Log( "Network did not initialize. Bye." );
		return 0;
	}

	{
		PNETBUFFER pNetBuffer = NetworkBuffers;
		Log1( "First net buffer: %p", NetworkBuffers );
		while( pNetBuffer )
		{
			DumpAddr( "Opening server: ", pNetBuffer->sa );
			OpenTCPServerAddrEx( pNetBuffer->sa, TCPConnection );
			pNetBuffer = pNetBuffer->next;
		}
	}

	{
		int altport = 0;
		do
		{
			TCPControl = OpenTCPServerEx( 3001 + altport, TCPControlConnect );
			if( !TCPControl )
			{
				if( !altport )
					altport = 10000;
				else
					altport++;
				if( altport > 10030 )
				{
					Log( "Failed to open control socket server" );
					break;
				}
				//Sleep( 2000 );
			}
		} while( !TCPControl );
	}
	// if broadcast address genuine is used - MUST enable broadcasting.
	//saBroadcast = CreateRemote( "255.255.255.255", 3000 );
	if( bEnableBroadcast )
	{
		saBroadcast[0] = CreateRemote( "127.0.0.1", 3000 );
		saBroadcast[1] = CreateRemote( "127.0.0.1", 3003 );
		do
		{
			UDPBroadcast = ServeUDP( NULL, 3001, NULL, NULL );
			if( !UDPBroadcast )
			{
				Log( "Failed to open the socket we're going to broadcast from" );
				Sleep( 2000 ); // wait 2 seconds
			}
			else
			{
				// enable broadcasting here.
				// UDPEnableBroadcast( UDPBroadcast, TRUE );
			}
		} while( !UDPBroadcast );
	}

	if( bReceiveLinkedPacket )
	{
		do
		{
			UDPListen = ServeUDP( NULL, 3000, UDPRecieve, UDPClose );
			if( !UDPListen )
			{
				Log( "Failed to open UDP listener" );
				Sleep(15000);
			}
		} while( !UDPListen );
	}

	if( NetworkBuffers ) // had someone to listen to...
	{
		Log( "Starting server...." );
		pServer = ThreadTo( ServerTimerProc, 0 );
	}

	if( defaultlogin[0] && server ) // had someone to login as...
	{
		Log( "Starting client...." );
		pClient = ThreadTo( ClientTimerProc, 0 );
	}

#ifdef _WIN32
	RegisterIcon( (char*)ICO_RELAY );
	{
		MSG msg;
		while( GetMessage( &msg, NULL, 0, 0 ) && !bDone )
		{
			DispatchMessage( &msg );
		}
	}
	bDone = 1;
	if( pServer )
		WakeThread( pServer );
	if( pClient )
		WakeThread( pClient );
	{
		_32 start = GetTickCount() + 1000;
		while( bServerRunning && ( start < GetTickCount() ) )
			Relinquish();
		EndThread( pServer );
		while( bClientRunning && ( start < GetTickCount() ) )
			Relinquish();
		EndThread( pClient );
	}
	UnregisterIcon();
	CloseAllAccounts();
	NetworkQuit();
	Release( Connection );
	DebugDumpMemFile( "relay.dump" );
#else
	WakeableSleep( SLEEP_FOREVER );
#endif
	return 0;
}

