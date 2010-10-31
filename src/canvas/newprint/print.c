//-----------------------------------------------------------------------
// Created 2001-12-4 : Jim Buckeyne
//      Purpose - generic print module for printing packs 
//      Scales fonts, etc, to the metrics fo the device context
//      should be rather easy scale to other orientations....
//      code marked out by #LOCALPRINTWINDOW was used to 
//      demonstrate the appropriate scaling within a flexible 
//      window.  There are problems yet with the font scaler, there
//      are times that it can't ever pick a font that works well....
//      and these appear as a hang....
//
//-----------------------------------------------------------------------

#if defined(PRINT_NETWORK) // print to printer directly executing own system(copy)
#define DEDICATEDPRINT
#define NETWORK_ENABLED
//#define PRINT_DISABLED
#define LOG_ENABLE
#endif

#if defined(PRINT_DIRECT) // print to printer directly executing own system(copy)
#define DEDICATEDPRINT
//#define NETWORK_ENABLED
//#define PRINT_DISABLED
#define LOG_ENABLE
#endif

#if defined(PRINT_WINDOW) // print to a window directly 
//#define DEDICATEDPRINT
//#define NETWORK_ENABLED
#define PRINT_DISABLED
#define LOG_ENABLE
#endif

#if defined(PRINT_FILE)
#define DEDICATEDPRINT
#define DISABLEOUTPUT
//#define NETWORK_ENABLED
//#define PRINT_DISABLED
#define LOG_ENABLE
#endif

//#define RECIEPT_DATA

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <winspool.h>
//#include <string.h>
//#include <stdlib.h>
//#include <commdlg.h>

#include <controls.h>

#include <stdio.h>

char ticketcode[36];

#ifdef NETWORK_ENABLED
#include <network.h>
PCLIENT pcAgent;
#define NL_SELF      0
#define NL_COMPLETE  1
#endif
#include <sharemem.h>
#include <image.h>
#include <logging.h>

#define STRXX(n) #n
#define STRNUM(n) STRXX(n)

#define PSIPRINTWINDOW
//#define LOCALPRINTWINDOW
#define LOG_ENABLE


//LPDEVMODE PrinterMode;

//--------------------------------------------------------------------------
//#include "bingpack.h"

char rcptname[280];

int bonusnums[3];

//PPRINT_PACKINFO  gpPackInfo;
//LPBINGPACKDATA   gpPackData;
void LoadSellData( void );
void FillCards( int start );
//--------------------------------------------------------------------------

#include "template.h"

//--------------------------------------------------------------------------

// define this symbol to create a window which demonstrates
// approximate printer output...


#ifdef LOG_ENABLE
FILE *log;
#endif

//--------------------------------------------------------------------------

#if defined( LOCALPRINTWINDOW ) || defined( PSIPRINTWINDOW )
// junk data - neede domsehting to print....
/*
BINGPACKDATA gpack[2] = { { "name"
 								 , 6
 								 , 6
 								 , 0
 								 }
 							  , { "name"
 								 , 6
 								 , 6
 								 , 0
 								 } };
								 PRINT_PACKINFO dummyinfo;
                         */
PTEMPLATE DisplayTemplate;
//HBRUSH BackBrush;

#endif

//--------------------------------------------------------------------------

#include "templatedraw.h"

//--------------------------------------------------------------------------

void CalculateSingleScaledFont( PDEVICE device
										, PPRINTFONT thisfont
									   )
{
#define MAX_HISTORY_SIZE 8
	//LOGFONT lf;
//SIZE sz, _sz[MAX_HISTORY_SIZE];
	struct {
		_32 cx, cy;
	} sz, _sz[MAX_HISTORY_SIZE];
	int n, n_sz; // index of this _sz to store, max to compare...
	Font basefont = 0;
	int y, x;
	int _pw[MAX_HISTORY_SIZE], pw = 0;
	int _ph[MAX_HISTORY_SIZE], ph = 0;
	int success;
	int TargetHeight, TargetWidth;

	TargetHeight = ( (device->scaley * thisfont->size.y.numerator ) /
							( thisfont->size.y.denominator ) );
	TargetWidth = ( (device->scalex * thisfont->size.x.numerator ) /
							( thisfont->size.x.denominator ) );

	fprintf( log, WIDE("Sizing font %s %d/%d(%d) %d/%d(%d)...\n")
					, thisfont->name
					, thisfont->size.x.numerator, thisfont->size.x.denominator
					, TargetWidth
					, thisfont->size.y.numerator, thisfont->size.y.denominator
					, TargetHeight
					);
   fprintf( log, WIDE("Target to fit: %d %d %d %d\n"), device->scalex, device->scaley, TargetHeight, TargetWidth );
		
	//memset( &lf, 0, sizeof( LOGFONT ) );


   //lf.lfHeight = -TargetHeight;
   //lf.lfWidth = TargetWidth / strlen(thisfont->text);
   //lf.lfWidth = lf.lfWidth; // some fixup - gets us closer

   if( thisfont->flags.vertical )
   {
     // lf.lfOrientation = 2700;
   }
   if( thisfont->flags.inverted )
   {
   	//lf.lfOrientation += 1800;
   }
   //if( lf.lfOrientation > 3600 )
   	//lf.lfOrientation -= 3600;
   //lf.lfEscapement = lf.lfOrientation;
   if( thisfont->flags.bold )
   {
	   //lf.lfWeight = FW_BOLD;
	}
	else
	{
		//lf.lfWeight = FW_THIN;
	}

   //lf.lfPitchAndFamily = 0;

	//if( thisfont->flags.script )
		//lf.lfPitchAndFamily |= FF_SCRIPT;
//	else
 //     lf.lfPitchAndFamily |= FF_MODERN;

	//if( thisfont->flags.fixed )
	   //lf.lfPitchAndFamily |= FIXED_PITCH;

   success = FALSE;
   for( n = 0; n < MAX_HISTORY_SIZE; n++ )
   {
   	_sz[n].cx = 0;
   	_sz[n].cy = 0;
   	_pw[n] = 0;
   	_ph[n] = 0;
   }
   n_sz = 0;
   while( !success )
   {	
   	Font hNewFont;
#ifdef LOG_ENABLE
		//fprintf( log, WIDE("%d (%s) check at: fh:%d fw:%d\n"), __LINE__, thisfont->text, lf.lfWidth, lf.lfHeight );
#endif
		hNewFont = RenderFontFile( WIDE("arialbd.ttf"), TargetWidth, TargetHeight, 0 );  //CreateFontIndirect( &lf );
		//if( !basefont )
		//	basefont = SelectObject( device->hdc, hNewFont );
		//else
		//	SelectObject( device->hdc, hNewFont );
		if( thisfont->hFont )
			DestroyFont( &thisfont->hFont );
		thisfont->hFont = hNewFont;		
   
		//GetTextExtentPoint( device->hdc, thisfont->text, strlen(thisfont->text), &sz );
#ifdef LOG_ENABLE
		fprintf( log, WIDE("Size was: x:%d y:%d vs w:%d h:%d\n"), sz.cx, sz.cy, TargetWidth, TargetHeight );
#endif
#if 0
		for( n = n_sz; n < MAX_HISTORY_SIZE; n++ )
		{
         /*
			if( _pw[n] == lf.lfWidth && _ph[n] == lf.lfHeight )
			{
				success = TRUE;
				break;
			}	
			if( sz.cx == _sz[n].cx && sz.cy == _sz[n].cy )
			{
				success = TRUE;
				break;
				}
				*/
         success = TRUE;
         break;
		}
		if( success )
			continue;
		for( n = 0; n < n_sz; n++ )
		{
         /*
			if( _pw[n] == lf.lfWidth && _ph[n] == lf.lfHeight )
			{
				success = TRUE;
				break;
			}	
			if( sz.cx == _sz[n].cx && sz.cy == _sz[n].cy )
			{
				success = TRUE;
				break;
				}
            */
         success = TRUE;
         break;
		}
		if( success )
			continue;
		_sz[n_sz] = sz;
		_ph[n_sz] = ph;
		_pw[n_sz] = pw;
		n_sz++;
		if( n_sz == MAX_HISTORY_SIZE )
			n_sz = 0;
		ph = lf.lfHeight;
		pw = lf.lfWidth;
		success = TRUE;
		if( sz.cx > TargetWidth )
		{
			lf.lfWidth -= 1+(( lf.lfWidth * (sz.cx - TargetWidth) / (sz.cx+TargetWidth) ));
			//lf.lfWidth -= ( lf.lfWidth * (sz.cx - TargetWidth) / sz.cx );
			success = FALSE;
			//continue;
		}
		if( sz.cy > TargetHeight )
		{
			lf.lfHeight -= (( lf.lfHeight * (sz.cy-TargetHeight)/( sz.cy +TargetHeight)))-1;
			//lf.lfHeight -= ( lf.lfHeight * (sz.cy-TargetHeight)/sz.cy );
			//lf.lfHeight ++;
			success = FALSE;
			//continue;
		}
#define MIN_UNDER 95
		if( sz.cy < ( TargetHeight * MIN_UNDER / 100 ) )
		{
			lf.lfHeight -= ( lf.lfHeight * (sz.cy-TargetHeight)/TargetHeight );
			//lf.lfHeight--;
			success = FALSE;
		}
		if( sz.cx < (TargetWidth * MIN_UNDER / 100 ) )
		{
			lf.lfWidth -= ( lf.lfWidth * (sz.cx - TargetWidth) / TargetWidth );
			//lf.lfWidth++;
			success = FALSE;
		}
#endif
	}
	//SelectObject( device->hdc, basefont );
}

//--------------------------------------------------------------------------

void ReleaseFont( PPRINTFONT font )
{
	if( font->hFont )
	{
      DestroyFont( &font->hFont );
		//DeleteObject( font->hFont );
		//font->hFont = 0;
	}
}

//--------------------------------------------------------------------------

void SizeAllFonts( PPRINTPAGE page )
{
	PPRINTFONT current = page->fonts;
	fprintf( log, WIDE("Sizing all fonts...\n") );
	while( current )
	{
		CalculateSingleScaledFont( &page->output, current );
		current = current->next;	
	}
}

//---------------------------------------------------------------------------

#ifdef PSIPRINTWINDOW


int CPROC DrawFrame( PCOMMON pc )
{
	PPRINTPAGE page = (PPRINTPAGE)GetCommonUserData( pc );
	Image surface = GetControlSurface( pc );
   /*
	 device->width = r.right - r.left;
	 device->height = r.bottom - r.top;
	 device->scalex = device->width * page->pagesize.x.denominator / page->pagesize.x.numerator;
	 device->scaley = device->height * page->pagesize.y.denominator / page->pagesize.y.numerator;
	 SizeAllFonts( page );
	*/
	RenderPage( surface, page );
}

static void CreateJunkWindow( PTEMPLATE template )
{
	PPRINTPAGE page;
	for( page = template->pages; page; page = page->next )
	{
		PCOMMON pc = CreateFrame( WIDE("Junk Window Thing"), 0, 0, 640, 480, BORDER_NORMAL, NULL );
		SetCommonDraw( pc, DrawFrame );
      SetCommonUserData( pc, (PTRSZVAL)page );
		DisplayFrame( pc );
	}
}
#endif

//---------------------------------------------------------------------------

int bPrinting;
int startingcard;

static int BINGCARDPrintCardDC( //LPBINGPACKDATA lpBINGPackData
                                PPRINTPAGE page
										//, PPRINT_PACKINFO pPackInfo
										)
{
   int r;
	char buffer[256];
	if( !page )
	{
		Log( WIDE("We can't print without having a page...") );
		return PRINT_ERROR_NO_DATA;
	}	
	//if( !lpBINGPackData )
	//	return PRINT_ERROR_NO_DATA;

   {
      //gpPackData = lpBINGPackData; // saves some layered passing...
      //gpPackInfo = pPackInfo;

      {
         DOCINFO di;
         MemSet( &di, 0, sizeof( DOCINFO ) );
         di.cbSize = sizeof( DOCINFO );
         di.lpszDocName = "Bingo Pack Print";
         di.lpszOutput = NULL; // output to default destination
         if( StartDoc( page->output.hdc, &di ) <= 0 )
         {
            bPrinting = 0;
            fprintf( log, WIDE("Failed to start the page?! %d\n"), GetLastError() );
            return PRINT_ERROR_StartDoc;
         }
      }
      {
         FillCards( startingcard );
         fprintf( log, WIDE("Sizing fonts to printer - this is constant and should NOT be done every print!\n") );
         fflush( log );

         // fix internal positions according to printer specifics.
         if( StartPage( page->output.hdc ) <= 0 )
         {
         	Log( WIDE("Aborting Print?!") );
            bPrinting = 0;
            AbortDoc(page->output.hdc);
            return PRINT_ERROR_StartPage;
         }
         Log( WIDE("Page..") );
         RenderPage( page );
         Log( WIDE("Rendered...") );
         if( ( r = EndPage( page->output.hdc ) ) <= 0 )
         {
            Log1( WIDE("EndPage failed: %d"), GetLastError() );
            bPrinting = 0;
            AbortDoc(page->output.hdc);
            switch( r )
            {
            default:
            case SP_ERROR:
               return PRINT_ERROR_EndPage;
            case SP_APPABORT:
               return PRINT_ERROR_EndPageAbort;
            case SP_USERABORT:
               return PRINT_ERROR_EndPageUserAbort;
            case SP_OUTOFDISK:
               return PRINT_ERROR_EndPageOutOfDisk;
            case SP_OUTOFMEMORY:
               return PRINT_ERROR_EndPageOutOfMemory;
            }
         }
      }
   }
   if( EndDoc( page->output.hdc ) <= 0 )
   {
      Log1( WIDE("EndDoc Failed: %d"), GetLastError() );
         AbortDoc( page->output.hdc );
		bPrinting = 0;
		return PRINT_ERROR_EndDoc;
	}
	bPrinting = 0;	
	return PRINT_ERROR_SUCCESS;
}

//---------------------------------------------------------------------------

void DumpDevMode( DEVMODE *pdm )
{
	Log1( WIDE("dmDeviceName     :%s"), pdm->dmDeviceName );
	Log1( WIDE("dmSpecVersion;   :%d"), pdm->dmSpecVersion)  ;
	Log1( WIDE("dmDriverVersion; :%d"), pdm->dmDriverVersion);
	Log1( WIDE("dmSize;          :%d"), pdm->dmSize)         ;
	Log1( WIDE("dmDriverExtra;   :%d"), pdm->dmDriverExtra)  ;
	Log1( WIDE("dmFields;        :%08x"), pdm->dmFields)       ;
	Log1( WIDE("dmOrientation;   :%d"), pdm->dmOrientation)  ;
	Log1( WIDE("dmPaperSize;     :%d"), pdm->dmPaperSize)    ;
	Log1( WIDE("dmPaperLength;   :%d"), pdm->dmPaperLength)  ;
	Log1( WIDE("dmPaperWidth;    :%d"), pdm->dmPaperWidth)   ;
	Log1( WIDE("dmScale;         :%d"), pdm->dmScale)        ;
	Log1( WIDE("dmCopies;        :%d"), pdm->dmCopies)       ;
	Log1( WIDE("dmDefaultSource; :%d"), pdm->dmDefaultSource);
	Log1( WIDE("dmPrintQuality;  :%d"), pdm->dmPrintQuality) ;
	Log1( WIDE("dmColor;         :%d"), pdm->dmColor)        ;
	Log1( WIDE("dmDuplex;        :%d"), pdm->dmDuplex)       ;
	Log1( WIDE("dmYResolution;   :%d"), pdm->dmYResolution);
	Log1( WIDE("dmTTOption       :%d"), pdm->dmTTOption);

}

//---------------------------------------------------------------------------

void ConfigurePageDevice( PPRINTPAGE page )
{
#if 0
   PrinterMode->dmFields |= DM_ORIENTATION;
   if( page->flags.bLandscape )
   {
      PrinterMode->dmOrientation = DMORIENT_LANDSCAPE;
		/*
      PrinterMode->dmPaperSize = DMPAPER_LEGAL;
      PrinterMode->dmFields |= DM_PAPERSIZE;
      PrinterMode->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
      if( page->pagesize.y.numerator == 8 &&
          page->pagesize.y.denominator == 1 &&
          page->pagesize.x.numerator == 21 &&
          page->pagesize.x.denominator == 2 )
      {
         PrinterMode->dmPaperSize = DMPAPER_LEGAL;
         PrinterMode->dmFields |= DM_PAPERSIZE;
         PrinterMode->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
      }
      else if( page->pagesize.y.numerator == 8 &&
               page->pagesize.y.denominator == 1 &&
               page->pagesize.x.numerator == 27 &&
               page->pagesize.x.denominator == 2 )
      {
         PrinterMode->dmPaperSize = DMPAPER_LEGAL;
         PrinterMode->dmFields |= DM_PAPERSIZE;
         PrinterMode->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
      }
      else
      {
         COORDPAIR c, margin;
         c = page->pagesize;
         margin.x.numerator = 1;
         margin.x.denominator = 2;
         margin.y.numerator = 1;
         margin.y.denominator = 2;
         AddCoords( &c, &margin );
         PrinterMode->dmPaperLength = ( c.x.numerator * 254 ) /
            c.x.denominator;
         PrinterMode->dmPaperWidth = ( c.y.numerator * 254 ) /
            c.y.denominator;
         PrinterMode->dmFields |= DM_PAPERLENGTH|DM_PAPERWIDTH;
         PrinterMode->dmFields &= ~DM_PAPERSIZE;
      }
      */
   }
   else
   {
      PrinterMode->dmOrientation = DMORIENT_PORTRAIT;
      /*
      PrinterMode->dmPaperSize = DMPAPER_LEGAL;
         PrinterMode->dmFields |= DM_PAPERSIZE;
         PrinterMode->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
      if( page->pagesize.x.numerator == 8 &&
          page->pagesize.x.denominator == 1 &&
          page->pagesize.y.numerator == 21 &&
          page->pagesize.y.denominator == 2 )
      {
         PrinterMode->dmPaperSize = DMPAPER_LEGAL;
         PrinterMode->dmFields |= DM_PAPERSIZE;
         PrinterMode->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
      }
      else if( page->pagesize.x.numerator == 8 &&
               page->pagesize.x.denominator == 1 &&
               page->pagesize.y.numerator == 27 &&
               page->pagesize.y.denominator == 2 )
      {
         PrinterMode->dmPaperSize = DMPAPER_LEGAL;
         PrinterMode->dmFields |= DM_PAPERSIZE;
         PrinterMode->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
      }
      else
      */
      {
         COORDPAIR c, margin;
         c = page->pagesize;
         margin.x.numerator = 0;
         margin.x.denominator = 1;
         margin.y.numerator = 0;
         margin.y.denominator = 1;
         AddCoords( &c, &margin );
         PrinterMode->dmPaperLength = ( c.y.numerator * 254 ) /
            c.y.denominator;
         PrinterMode->dmPaperWidth = ( c.x.numerator * 254 ) /
            c.x.denominator;
         PrinterMode->dmFields |= DM_PAPERLENGTH|DM_PAPERWIDTH;
         PrinterMode->dmFields &= ~DM_PAPERSIZE;
         PrinterMode->dmPaperSize = 0;
      }
      
   }

	DumpDevMode( PrinterMode );
#endif
   {
      SYSTEMTIME st;
      GetLocalTime( &st );
      sprintf( page->output.filename, WIDE("Print%05d%02d%02d%02d%02d%02d%03d.binary"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
      fprintf( log, WIDE("attempting to print to : %s\n"), page->output.filename );
   }
}

//---------------------------------------------------------------------------
//
// Create an escape string containing the barcode for a Star 300 Series
// printer. Barcode font is INI defined. Resulting string of *lpiSize
// bytes (contains NULLs, so you must look at the length returned) is
// 8 pixels high.
//

//;THIS IS *,0,1,2,3,4,5,6,7,8,9 in standard 3of9 format
char *codestrings[] = {"*--*-**-**-*-" // *
					, WIDE("*-*--**-**-*-") // 0
					, WIDE("**-*--*-*-**-") // 1
					, WIDE("*-**--*-*-**-") // 2
					, WIDE("**-**--*-*-*-") // 3
					, WIDE("*-*--**-*-**-") // 4
					, WIDE("**-*--**-*-*-") // 5
					, WIDE("*-**--**-*-*-") // 6
					, WIDE("*-*--*-**-**-") // 7
					, WIDE("**-*--*-**-*-") // 8
					, WIDE("*-**--*-**-*-") }; // 9

static char *validchars = "*0123456789";

void DoBarCode(FILE *lpt, LPSTR szBarcodeString )
{
    char *szBcChar, szEntry[32];
    int iStrIndx, iBcIndx, iCharIndx, validlen;

    if (!szBarcodeString ||
    			//(strlen(szBarcodeString) < 0 ) ||
    			(strlen(szBarcodeString) > 30) )
        return;
    fputc( '\r', lpt );
    fputc( '\n', lpt );

    fputc( '\x1b', lpt  );
    fputc(  'K', lpt );

    validlen = 0;
    for( iStrIndx=0; szBarcodeString[iStrIndx]; iStrIndx++ )
    {
        //Only certain chars are part of barcode
			char *foundchar;
        if ( !strchr(validchars, szBarcodeString[iStrIndx]) )
            continue;
        validlen++;
	 }
	 fputc( ( validlen * 13 ) & 0xFF, lpt );
	 fputc( 0, lpt );

    for (iBcIndx=iStrIndx=0; szBarcodeString[iStrIndx]; iStrIndx++)
    {
        //Only certain chars are part of barcode
			char *foundchar;
        if ((foundchar = strchr(validchars, szBarcodeString[iStrIndx])) == NULL)
            continue;

        szBcChar =  codestrings[foundchar - validchars];

        for (iCharIndx=0; szBcChar[iCharIndx]; iCharIndx++)
        {
        		if( szBcChar[iCharIndx]=='*' )
        			fputc( '\xff', lpt );
        		else
        			fputc( 0, lpt );
        }
    }

}
//---------------------------------------------------------------------------

void DumpToPrinter( char *header, char *name, char *code )
{
	FILE *lpt;
	char *data;
	DWORD dwSize;

	lpt = fopen( WIDE("lpt1"), WIDE("wb") );
	if( !lpt )
	{
		MessageBox( NULL, WIDE("Failed to open lpt1"), WIDE("Print aborted"), MB_OK );
		return;
	}
	Log2( WIDE("Dumping to the printer: %s and %s"), header, name );

#if defined(RECIEPT_DATA)
	dwSize = 0;
	data = OpenSpace( NULL, header, &dwSize );
	{
		int i;
  		for( i = 0; i < dwSize; i++ )
  		{
  			if( data[i] < 32 &&
   			 data[i] != 13 &&
   			 data[i] != 10 )
   			{
   				Log2( WIDE("Squash %d (%d) to NUL!"), i, data[i] );
   			   data[i] = 0;
   			}
   	}
	}
	
	fwrite( data, 1, dwSize, lpt );
	CloseSpace( data );
#endif

	dwSize = 0;
	data = OpenSpace( NULL, name, &dwSize );
#if defined(RECIEPT_DATA)
   fwrite( data, 1, dwSize -13, lpt );
#else
   fwrite( data, 1, dwSize, lpt );
#endif
	CloseSpace( data );
#if defined(RECIEPT_DATA)
	DoBarCode( lpt, ticketcode );
	DoBarCode( lpt, ticketcode );
	DoBarCode( lpt, ticketcode );

	fwrite( WIDE("\x1bJ`\x1bm"), 1, 5, lpt );
#endif
	fflush( lpt );
	fclose( lpt );
	if( GetPrivateProfileInt( WIDE("Printer"), WIDE("delete when done"), 0, WIDE("./print.ini") ) )
	{
	   remove( header );
	   remove( name );
	}
}


//---------------------------------------------------------------------------

int FAR _export BINGCARDPrintCard( LPCSTR driver, LPCSTR device, LPCSTR output
											, //LPBINGPACKDATA lpBINGPackData
											, char *templatename
											, PPRINT_PACKINFO packinfo )
{
   PPRINTPAGE page;
 	PTEMPLATE template;
	if( !driver || !device || !output )
		return PRINT_ERROR_INVALID_PRINTER_NAME;
	//if( !lpBINGPackData )
	//	return PRINT_ERROR_NO_DATA;

  	template = FindTemplate( templatename );
  	if( !template )
  		return PRINT_ERROR_NO_TEMPLATE;

   page = template->pages;
   for( page = template->pages; page; page = page->next )
   {
   	Log( WIDE("To configure page device for page...") );
      ConfigurePageDevice( page );

		{
         PCANVAS canvas = GetCanvas();
         //HDC hDC;
         int Result;

         //hDC = CreateDC( driver, device
         //                , NULL/*page->output.filename*/, PrinterMode );

         if( !hDC )
         {
            // this better not be zero - we shall test however...
            Log( WIDE("Failed to get printer DC\n") );
            return PRINT_ERROR_CreateDC;
         }
         Log4( WIDE("Got DC for printer: %08x (%s,%s,%s)"), hDC, driver, device, page->output.filename );
         //ResetDC( hDC, PrinterMode );

         page->output.hdc = hDC;
         page->output.scalex = GetDeviceCaps( page->output.hdc, LOGPIXELSX	);
         page->output.scaley = GetDeviceCaps( page->output.hdc, LOGPIXELSY );
         fprintf( log, WIDE("Printer resolution: %d by %d\n"), page->output.scalex, page->output.scaley );
         page->output.width = GetDeviceCaps( page->output.hdc, HORZRES );
         page->output.height = GetDeviceCaps( page->output.hdc, VERTRES );
         fprintf( log, WIDE("Printer Size: %d by %d\n"), page->output.width, page->output.height );

#ifdef RECIEPT_DATA
	// resize to receipt data...
         page->output.height = (page->output.scaley * page->pagesize.y.numerator )
         					/ ( page->pagesize.y.denominator );

         fprintf( log, WIDE("Printer Size: %d by %d\n"), page->output.width, page->output.height );
#endif
         SizeAllFonts( page );

         Result = BINGCARDPrintCardDC( /*lpBINGPackData, */page/*, packinfo*/ );
			if( Result )
				return Result;
         DeleteDC( hDC );
#ifndef NETWORK_ENABLED
#ifndef DISABLEOUTPUT
			DumpToPrinter( rcptname, page->output.filename, WIDE("123456789") );
#endif
#endif
         page->output.hdc = 0;
      }
   }
   return 0;
}

//---------------------------------------------------------------------------

int BINGOCARDLoadTemplate( char *templatename, char *templatefilename )
{
	return LoadTemplate( templatename, templatefilename );
}

//---------------------------------------------------------------------------

int BINGOCARDInitPrinting( char *device )
{
	int dmsize;
#ifndef PRINT_DISABLED
  	{
  		HANDLE hPrinter;
		extern int ExtDeviceMode(HWND, HANDLE, LPDEVMODE, LPSTR, LPSTR, LPDEVMODE, LPSTR, WORD);
      if( !OpenPrinter( device, &hPrinter, NULL ) )
      {
      	return PRINT_ERROR_CANTOPEN;
      }
      dmsize = DocumentProperties( NULL, hPrinter, NULL, NULL, NULL, 0 );
      if( dmsize < 0 )
      {
      	Log( WIDE("DocumentProperties returned bad information\n") );
      	return PRINT_ERROR_SUCCESS;
      }
      //PrinterMode = Allocate( dmsize );

      //DocumentProperties( NULL, hPrinter, NULL, PrinterMode, NULL, DM_OUT_BUFFER );
		//ExtDeviceMode( NULL, NULL, dm, device, output, NULL, NULL, DM_OUT_BUFFER	 );

		//DumpDevMode( PrinterMode );
	}
#endif
	return 0;
}

//---------------------------------------------------------------------------

//#ifdef LOCALPRINTWINDOW

char *rcptinfo; // memory mapped thing - do NOT modify...

void FillCards( int starting )
{
	char cardfile[256];
	GetPrivateProfileString( WIDE("Printer"), WIDE("Cards"), WIDE(""), cardfile,256, WIDE("./print.ini") );
	Log1( WIDE("Starting at: %d"), starting );
	{
		FILE *cards = fopen( cardfile, WIDE("rb") );
		if( cards )
		{
			int c, d, n;
			long int card = starting - 1 /*((( long)rand() << 12 ) + rand()) % 87984 */;
			for( c = 0; c < 6; c++ )
			{
				char data[12];
				fseek( cards, (card + (c*50)) * 12, SEEK_SET );
				fread( data, 1, 12, cards );
				gpack[0].dwCardNumArray[c] = (card + (c*50)) + 1;

				n = 0;
				for( d = 0; d < 25; d++ )
				{
					if( d != 12 )
					{
						gpack[0].byCardData[c][d] = ((data[n/2] >> ( (!( n & 1 )) * 4) )&0xF) + ( (d / 5)*15)+1;
						//gpack[1].byCardData[c][d] = (rand() % 15) + ( (d / 5)*15)+1;
						n++;
					}
				}
			}
			fclose( cards );
		}
		else
		{
			char msg[256];
			sprintf( msg, WIDE("Could not open card file\n%s from print.ini"), cardfile );
			MessageBox( NULL, msg, WIDE("Print Exiting"), MB_OK );
			exit(0);
		}
	}

	GetPrivateProfileString( WIDE("Printer"), WIDE("Bonus"), WIDE(""), cardfile,256, WIDE("./print.ini") );
	{
      FILE *cards;
		cards = fopen( cardfile, WIDE("rb") );
		if( cards )
		{
			unsigned char vals[3];
			fseek( cards,(3 * ((((starting%250) -1) *16) + starting/1500)+3), SEEK_SET);
			//fseek( cards, starting * 3, SEEK_SET );
			fread( vals, 1, 3, cards );
			bonusnums[0] = vals[0];
			bonusnums[1] = vals[1];
			bonusnums[2] = vals[2];
			fclose( cards );
		}
		else
		{
			char msg[256];
			sprintf( msg, WIDE("Could not open bonus file\n%s from print.ini"), cardfile );
			MessageBox( NULL, msg, WIDE("Print Exiting"), MB_OK );
			exit(0);
		}
	}
	Log( WIDE("Loaded the cards...") );
}

#define SetInfo(name) 	{	fgets( buffer, 256, in );    \
		if( dummyinfo.name )                              \
			Release( dummyinfo.name );                     \
		if( strlen( buffer ) > 1 )                        \
		{                                                 \
			buffer[strlen(buffer)-1] = 0;                  \
      	dummyinfo.name = Allocate( strlen(buffer)+1);  \
      	strcpy( dummyinfo.name, buffer );              \
       }                                                \
       else                                             \
       { dummyinfo.name = NULL; } }

#define AllocInfo(name)   	dummyinfo.name = Allocate( 256 ); dummyinfo.name[0] = 0;  

void LoadSellData( void )
{
	FILE *in;
	int n, dest, range;
	Log( WIDE("Loading sell data  (default to receipt)") );
	in = fopen( WIDE("BuyerData.txt"), WIDE("rt") );
	AllocInfo( PlayerID );
	AllocInfo( POSID );
	AllocInfo( SessionID );
	AllocInfo( Series );
	AllocInfo( PackID );
	AllocInfo( GameNumber );
	AllocInfo( Price );
	AllocInfo( TransactionID );
	AllocInfo( Game );
	AllocInfo( GameTitle1 );
	AllocInfo( GameTitle2 );
	AllocInfo( Date );
	AllocInfo( Time );
	dummyinfo.Aux1 = rcptinfo;
	//AllocInfo( Aux1 );
	AllocInfo( Aux2 );

	Log( WIDE("Allocated sell data") );
   if( in )
	{
		char buffer[256];
		//fgets( buffer, 256, in );
		//sscanf( buffer, WIDE("%d"), &range );
		range = 5706;
		dest = (rand() % range); 
		// random picking probably here...
		Log2( WIDE("Using seller data %d of %d\n"), dest, range );
		for( n = 0; n < dest; n++ )
		{
			fgets( buffer, 256, in );
		}
		{
			SYSTEMTIME st;
			char *start, *end, *lastname, *firstname, *series, *packid;
			GetLocalTime( &st );
			start = buffer;
			Log1( WIDE("Process: %s"), start );

			start++;
			// store player ID
			while( start[0] && start[0] != '\"' ) start++;
			start += 3; // ","
			Log1( WIDE("Process: %s"), start );

			lastname = start;
			while( start[0] && start[0] != '\"' ) start++;
			start[0] = 0;
			start += 3;
			Log1( WIDE("Process: %s"), start );

			firstname = start;
			while( start[0] && start[0] != '\"' ) start++;
			start[0] = 0;
			start += 2;
			Log1( WIDE("Process: %s"), start );

			series = start;
			while( start[0] && start[0] != ',' ) start++;
			start[0] = 0;
	      start += 2;
			Log1( WIDE("Process: %s"), start );

			dummyinfo.POSID[0] = *start;
			dummyinfo.POSID[1] = 0;

			start += 4;
			packid = start;
			while( start[0] && start[0] != '\"' ) start++;
			start[0] = 0;

			sprintf( dummyinfo.PlayerID, WIDE("%s %s"), firstname, lastname );
			strcpy( dummyinfo.Series, series );
			strcpy( dummyinfo.PackID, packid );

			sprintf( dummyinfo.Date, WIDE("%02d/%02d/%4d"), st.wMonth, st.wDay, st.wYear );
			sprintf( dummyinfo.SessionID, WIDE("Session: %d%c"), ((st.wHour+1)%13)+1, ((st.wHour+1)>12)?'P':'A' );
		}	
		/*
			SetInfo( PlayerID );
			SetInfo( POSID );
			SetInfo( SessionID );
			SetInfo( Series );
			SetInfo( PackID );
			SetInfo( GameNumber );
			SetInfo( Price );
			SetInfo( TransactionID );
			SetInfo( Game );
			SetInfo( GameTitle1 );
			SetInfo( GameTitle2 );
			SetInfo( Date );
			SetInfo( Time );
			SetInfo( Aux1 );
			SetInfo( Aux2 );
			fgets( buffer, 256, in ); // get ---<break>--- line
		*/
	}
	Log( WIDE("Sell Data completed...") );
}


#ifdef NETWORK_ENABLED

PCLIENT pcAgent;


void CPROC TCPClose( PCLIENT pc, int connections )
{
	PCLIENT *me;
	me = (PCLIENT*)GetNetworkLong( pc, 0 );
	*me = NULL;	
	printf( WIDE("Closed!?") );
}

void CPROC TCPRead( PCLIENT pc, POINTER buffer, int size )
{
   if( !buffer )
   {
      buffer = Allocate( 256 );
   }
   else
   {
      if( *(_32*)buffer == *(_32*)"DONE" )
         SetNetworkLong( pc, NL_COMPLETE, 1 );
   }
   ReadTCP( pc, buffer, 256 );
}

void CPROC UDPMessage( PCLIENT pc, POINTER buffer, int size, SOCKADDR *sa )
{
   if( !buffer )
   {
      buffer = Allocate( 2048 );
      SendUDP( pc, WIDE("WHERE RU"), 8 );
   }
   else
   {
      if( *(_64*)buffer == *(_64*)"IM HERE!" )
      {
      	Log( WIDE("Connecting to agent!") );
         pcAgent = OpenTCPClientAddrEx( sa, TCPRead, TCPClose, NULL );
         Log1( WIDE("Result: %p"), pcAgent );
         if( !pcAgent )
         {
            MessageBox( NULL, WIDE("(1)Print agent was not running\nprint job lost"), WIDE("Print Error"), MB_OK );
            exit(0);
         }
         SetNetworkLong( pcAgent, NL_SELF, (_32)&pcAgent );
      }
   }
   ReadUDP( pc, buffer, 2048 );
}


void SendFile( PCLIENT pc, char *name )
{
   _32 size = 0;
   FILE *file;
   POINTER memory;
   Log1( WIDE("Have to send a page called: %s"), name );
   memory = OpenSpace( NULL, name, &size );
   if( memory )
   {
      SendTCP( pcAgent, WIDE("PAGE"), 4 );
      SendTCP( pcAgent, &size, 4 );
      Log( WIDE("Memory Mapped a HUGE space") );
      SendTCP( pcAgent, memory, size ); // if write notifications worekd....
      CloseSpace( memory );
   }
   else
   	Log( WIDE("Failed to open the file into memory :( ") );
}

#endif

//#define DEFAULTPRINTER "HP LaserJet"
//#define DEFAULTPRINTER "Brother HL-2460 series"
//#define DEFAULTPRINTER "Star TSP847 (Fcut)"
//#define DEFAULTPRINTER "Star SP312 (J/TearBar)"
//#define DEFAULTPRINTER "Axiohm A793"
//int main( void )
int main( int argc, char **argv )
//int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow )
{
	HANDLE handle;
	//HDC hDC;
	//PRINTDLG pd;
	//DEVNAMES *pData;
	char device[256], driver[256], output[256], msg[256];
   char filename[256];
   char cmdline[256];
   char printername[256];

#ifdef LOG_ENABLE
   log = fopen( WIDE("junk.log"), WIDE("wt") );
	SetSystemLog( SYSLOG_FILE, log );
   fprintf( log, WIDE("pack size: %d\n"), sizeof(BINGPACKDATA ) );
   srand( GetTickCount() );
#endif
	//GetPrivateProfileString( WIDE("Printer"), WIDE("printer name"), WIDE("xx"), printername, sizeof( printername ), WIDE("./print.ini") );
   strcpy( cmdline, argv[1] );
   if( !strlen(cmdline) )
   {
   	MessageBox( NULL, WIDE("No template supplied as argument to print"), WIDE("Print Error"), MB_OK );
   	return 1;
   }
#ifdef RECIEPT_DATA
   {
   	char *rcptdata = strchr( cmdline, ' ' );
   	if( !rcptdata )
   	{
   		MessageBox( NULL, WIDE("Receipt data was not present on command line to print!"), WIDE("Command Error"), MB_OK );
   		return 1;
   	}
   	else
   	{
   		DWORD dwSize = 0;
   		int i;
   		char *cardtxt;
   		char *tmp;
   		rcptdata[0] = 0;
   		rcptdata++;
   		strcpy( rcptname, rcptdata );
   		tmp = OpenSpace( NULL, rcptdata, &dwSize );
   		if( !tmp )
   		{
   			MessageBox( NULL, WIDE("Reciept Info could not be mapped..."), WIDE("Fatal Error"), MB_OK );
   			return 1;
   		}
   		rcptinfo = Allocate( dwSize + 1 );
   		MemCpy( rcptinfo, tmp, dwSize );
   		rcptinfo[dwSize] = 0;
   		CloseSpace( tmp );
			for( i = 0; i < dwSize; i++ )
			{
				if( ( rcptinfo[i] == ' ' ) &&
				    ( rcptinfo[i+1] == 'C' ) &&
				    ( rcptinfo[i+2] == '=' ) &&
				    ( rcptinfo[i+3] == '0' ) )
				{
					cardtxt = rcptinfo + i;
					break;
				}
			}
			if( i < dwSize )
			{
	   		//cardtxt = strstr( rcptinfo, WIDE(" C=0") );
   			if( cardtxt )
   			{
   				cardtxt += 3;
   				sscanf( cardtxt, WIDE("%d"), &startingcard );
	   		   fprintf( log, WIDE("Starting Card is: %d"), startingcard );
   			}	
   		}
			for( i = 0; i < dwSize; i++ )
			{
				if( ( rcptinfo[i] == 'T' ) &&
				    ( rcptinfo[i+1] == 'i' ) &&
				    ( rcptinfo[i+2] == 'c' ) )
				{
					SYSTEMTIME st;
					GetLocalTime( &st );
               while( rcptinfo[i] < '0'|| rcptinfo[i] >'9' )
                  i++;
					sprintf( ticketcode, WIDE("*%04d%02d%02d%11.11s*")
								, st.wYear
								, st.wMonth
								, st.wDay
								, rcptinfo + i );
					Log1( WIDE("Ticket code: %s"), ticketcode );
					break;
				}
			}
   		for( i = 0; i < dwSize; i++ )
   		{
   			if( rcptinfo[i] < 32 &&
   			    rcptinfo[i] != 13 &&
   			    rcptinfo[i] != 10 )
   			{
   				Log2( WIDE("Squash %d (%d) to NUL!"), i, rcptinfo[i] );
   			   rcptinfo[i] = 0;
   			}
   		}	
   	}
   }
#endif

	if( BINGOCARDInitPrinting( printername ) )
	{
		char msg[256];
		sprintf( msg, WIDE("Perhaps you don't have a printer called \")%s\" installed?\n"
						  "Check print.ini also", printername );
		MessageBox( NULL, msg, WIDE("Printer Exiting"), MB_OK );
		return 0;
	}	
	BINGOCARDLoadTemplate( WIDE("First"), cmdline);

	FillCards(startingcard);
	LoadSellData();
#if !defined( PRINT_DISABLED )
	//--------------------------- Talk to the printer...
	{
		SYSTEMTIME st;
		GetLocalTime( &st );
		sprintf( filename, WIDE("Print%05d%02d%02d%02d%02d%02d%03d.binary"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
		fprintf( log, WIDE("attempting to print to : %s\n"), filename );
	}
	memset( &pd, 0, sizeof( pd ) );
	pd.lStructSize = sizeof( pd );
	pd.hDevNames = 0; 
	strcpy( driver, WIDE("winspool") );
	strcpy( device, printername );
	strcpy( output, filename );

#ifndef DEDICATEDPRINT
	if( (MessageBox( NULL, WIDE("Do you wish to print on default printer?"), WIDE("Print?"), MB_YESNO ) == IDYES) ||
		  PrintDlg( &pd ) )
#else
	pd.hDevNames = 0;
#endif
	{
		if( pd.hDevNames )
		{
			pData = (DEVNAMES*)GlobalLock( pd.hDevNames );
			if( pData )
			{
		#define GetString(p,o) ( (((char*)(p))+(o)) )
				strcpy( driver, GetString( pData, pData->wDriverOffset ) );
				strcpy( device, GetString( pData, pData->wDeviceOffset ) );
				strcpy( output, GetString( pData, pData->wOutputOffset ) );
				fprintf( log, WIDE("(%s) (%s) (%s)\n"), driver, device, output );
			}
         GlobalUnlock( pd.hDevNames );
		}
      fprintf( log, WIDE("Result code: %d\n"),
               BINGCARDPrintCard( driver, device, output
                                  , gpack
                                  , WIDE("First")
                                  , &dummyinfo ) );
   }
#endif
#ifdef NETWORK_ENABLED
	{
		if( !NetworkWait( NULL, 16, 16 ) )
		{
			MessageBox( NULL, WIDE("Network unreachable - print job lost\n"), WIDE("Print Error"), MB_OK );
			return 0;
      }
      ConnectUDP( NULL, 3016, WIDE("255.255.255.255"), 3005, UDPMessage, NULL );
      {
         int endtime = GetTickCount() + 3000;
         while( !pcAgent && ( endtime > GetTickCount() ) )
            Sleep(10);
         if( !pcAgent )
         {
            MessageBox( NULL, WIDE("(2)Print agent is not running - print job lost\n"), WIDE("Print Error"), MB_OK );
            return 0;
         }
      }
      if( (int)pcAgent == -1 )
      {
         MessageBox( NULL, WIDE("(3)Print agent is not running - print job lost\n"), WIDE("Print Error"), MB_OK );
         return 0;
      }

		{
			char msg[256];
			char basepath[256];
			int pages;
		 	PTEMPLATE template;
		 	PPRINTPAGE page;
		  	template = FindTemplate( WIDE("First") );
		  	if( !template )
		  		return 0;
         GetCurrentDirectory( 256, basepath );
         SendTCP( pcAgent, WIDE("BEGN"), 4 ); // start a new job set...
         for( pages = 0, page = template->pages; page; page = page->next, pages++ )
         {
            sprintf( msg, WIDE("%s/%s"), basepath, page->output.filename );
            printf( WIDE("name: %s\n"), msg );
            SendFile( pcAgent, msg );
            //remove( page->output.filename );
         }
         SendTCP( pcAgent, WIDE("DONE"), 4 );
      }
      while( !GetNetworkLong( pcAgent, NL_COMPLETE ) )
         Sleep( 1 );
      RemoveClient( pcAgent );
	}
#endif

#ifndef DEDICATEDPRINT
	//----------------------------- do demo card window

	DisplayTemplate = FindTemplate( WIDE("First") );
	gpPackInfo = &dummyinfo;
	gpPackData = gpack; // stuff...

   CreateJunkWindow( DisplayTemplate );

	{
		MSG msg;
		while( GetMessage( &msg, NULL, 0, 0 ) )
		{
			DispatchMessage( &msg );
		}	
	}
	// unknown name, handle to get, and no defaults specified
#endif
	return 0;
}
//#endif
