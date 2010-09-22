
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

#include <stdio.h>

#include <sharemem.h>
#include <logging.h>

#include "template.h"

#ifdef NEWCODE
// all these obsolete keywords....
#define FAR
#define _export
#define huge
#define _huge
#define far 
#define near
#define LOCALPRINTWINDOW
#define LOG_ENABLE
#else
//#define APIENTRY PASCAL      
#endif

int nToSort;
int pSortList[128];

#include "bingpack.h"

extern PPRINT_PACKINFO  gpPackInfo;
extern LPBINGPACKDATA   gpPackData;
extern int bonusnums[3];

//--------------------------------------------------------------------------

int CollapseQuotes( char *string )
{
	// this routine takes "thing" "another" and makes "thinganother"
	// however "this" and "that" is still "this" and "that"
	char quote = 0, lastquote = 0, *lastquotepos = NULL;
	char *in = string, *out = string;
	if( !string )
		return 0;
	while( *string )
	{
		if( !quote )
		{
			if( lastquote == *string )
			{
				out = lastquotepos;
				quote = *string;
			}
			else
			{
				if( *string != ' ' && *string != '\t' )
				{
					lastquote = 0;
               lastquotepos = NULL;
				}

				if( *string == '\"' || *string == '\'' )
					quote = *string;
				*out = *string;
				out++;	
			}
		}
		else
		{
			if( *string == quote )
			{
				lastquote = quote;
				lastquotepos = out;
				quote = 0; 
			}
			*out = *string;
			out++;	
		}
		string++;
	}
	*out = 0;
	return out - in; // bad form - but in never changes so this is length out.
}

//--------------------------------------------------------------------------

void SortList( void )
{
	int n, k;
	for( n = 0; n < nToSort; n++ )
	{
		for( k = n+1; k < nToSort; k++ )
		{
			if( pSortList[n] > pSortList[k] )
			{
				int tmp = pSortList[n];
				pSortList[n] = pSortList[k];
				pSortList[k] = tmp;
			}
		}
	}
}

//--------------------------------------------------------------------------

int SubstPrint( char *dest, char *src, PPRINTMACRORUN Macro )
{
	char *in, *out = dest, didsubst = 0;
	if( !dest || !src )
		return 0;
	for( in = src; *in; in++ )
	{
		if( *in == '%' )
		{
			INDEX i;
			char *name;
         char *text = NULL;
			in++;
			if( Macro )
         {
            PPRINTMACRORUN cur = Macro;
            do
            {
               LIST_FORALL( cur->macro->Parameters, i, char *, name )
               {
                  if( !strncmp( name, in, strlen( name ) ) )
                  {
                     in += strlen( name ) - 1;
                     if( cur->Values )
                     {
                        text = GetLink( &cur->Values, i );
                     }
                     else
                        Log( WIDE("Dunno - macro called without params?") );
                     break;
                  }
                  name = NULL; // clear the thing if we didn't match...
               }
               if( !name )
               {
                  cur = cur->priorrunning;
               }
               else
                  cur = NULL;
            } while( cur );
			}
			if( !text )
			{
				Log( WIDE("Argument was not a macro parameter...") );
				if( in[0] == 'N' &&
				    in[1] >= '1' && in[1] <= '9' )
				{
					// get item number here....
					char *tmp = in+1;
					int cardidx;
					int row, col;
					cardidx = 0;
					while( *tmp != '.' )
					{
						cardidx *= 10;
						cardidx += *tmp - '0';
						tmp++;
					}
               if( cardidx > gpPackData->byNumCards )
               {
               	Log2( WIDE("Cardidx:%d numcards:%d"), cardidx, gpPackData->byNumCards );
               	*out = '%';
               	out++;
               	*out = *in;
               	out++;
               	continue;
               }
              	Log2( WIDE("Cardidx:%d numcards:%d"), cardidx, gpPackData->byNumCards );
               in = tmp;
					in++;
					row = 0;
					while( *in != '.' )
					{
						row *= 10;
						row += *in - '0';
						in++;
					}
					in++;
					col = 0;
					while( *in >= '0' && *in <= '9' )
					{
						col *= 10;
						col += *in - '0';
						in++;
					}
					in--; // back up one for auto advance?
					cardidx--;
					row--;
					col--;
              	Log3( WIDE("Cardidx:%d Row:%d Col:%d"), cardidx,row,col );
					out += sprintf( out, WIDE("%d"), gpPackData->byCardData[cardidx][row + col*5] );
					didsubst++;
					continue;
				}
				else if( strncmp( in, WIDE("bonus"), 5 ) == 0 )
				{
				   in += 5;
					out += sprintf( out, WIDE("%d"), bonusnums[in[0] - 0x31] );
					// ((rand()>> 8 ) % 75) + 1 );
							//;  gpPackInfo->Random[ in[1] - 0x31 ] );
					didsubst++;
					continue;
				}
				else if( strncmp( in, WIDE("player"), 6 ) == 0 )
				{
					if( gpPackInfo->PlayerID )
					{
						out += sprintf( out, gpPackInfo->PlayerID );
						didsubst++;
						in += 5;
						continue;
				 	}
				}
				else if( strncmp( in, WIDE("price"), 5 ) == 0 )
				{
					if( gpPackInfo->Price )
					{
						out += sprintf( out, gpPackInfo->Price );
						didsubst++;
						in += 4;
						continue;
					}
				}
				else if( strncmp( in, WIDE("transaction"), 11 ) == 0 )
				{
					if( gpPackInfo->TransactionID )
					{
						out += sprintf( out, gpPackInfo->TransactionID );
						didsubst++;
						in += 10;
						continue;
					}
				}
				else if( strncmp( in, WIDE("POSID"), 5 ) == 0 )
				{
					if( gpPackInfo->POSID )
					{
						in += 4;
						out += sprintf( out, gpPackInfo->POSID );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("date"), 4 ) == 0 )
				{
					if( gpPackInfo->Date )
					{
						in += 3;
						out += sprintf( out, gpPackInfo->Date );
						didsubst++;
						continue;
					}
				}	
				else if( strncmp( in, WIDE("printdate"), 9 ) == 0 )
				{
					in += 8;
					{
						SYSTEMTIME st;
						GetLocalTime( &st );
						out += sprintf( out, WIDE("%02d:%02d:%02d %02d/%02d/%04d"), st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("time"), 4 ) == 0 )
				{
					if( gpPackInfo->Time )
					{
						in += 3;
						out += sprintf( out, gpPackInfo->Time );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("series"), 6 ) == 0 )
				{
					if( gpPackInfo->Series )
					{
						in += 5;
						out += sprintf( out, gpPackInfo->Series );
						didsubst++;
						continue;
					}	
				}
				else if( strncmp( in, WIDE("PackID"), 6 ) == 0 )
				{
					if( gpPackInfo->PackID )
					{
						in += 5;
						out += sprintf( out, gpPackInfo->PackID );
						didsubst++;
						continue;
					}
				}	
				else if( strncmp( in, WIDE("CardID"), 6 ) == 0 )
				{
					int num = 0;
					in += 6;
					// reference card number from var ....
					if( *in != '.' )
					{
						*out = '%';
						out++;
						continue;
					}
					in++;
					if( *in < '0' || *in > '9' )
					{
						in--; 
						out += sprintf( out, WIDE("%%CardID.") );
						continue;
					}
					while( *in >= '0' && *in <= '9' )
					{
						num *= 10;
						num += *in - '0';
						in++;
					}	
					if( num > gpPackData->byNumCards )
					{
						// don't have to retain exact info just a '%' is good enough.
						*out = '%';
						out++;
						continue; 
					}
					Log2( WIDE("%d is the carid: %d"), num, gpPackData->dwCardNumArray[num-1] );
					out += sprintf( out, WIDE("%d"), gpPackData->dwCardNumArray[num-1] );
					in --;
					didsubst++;
					continue; 
				}
				else if( strncmp( in, WIDE("game"), 4 ) == 0 )
				{
					if( gpPackInfo->Game )
					{
						in += 3;
						out += sprintf( out, gpPackInfo->Game );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("GameNum"), 7 ) == 0 )
				{
					if( gpPackInfo->GameNumber )
					{
						in += 6;
						out += sprintf( out, gpPackInfo->GameNumber );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("Aux1"), 4 ) == 0 )
				{
					if( gpPackInfo->Aux1 )
					{
						in += 4;
						out += sprintf( out, gpPackInfo->Aux1 );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("Aux2"), 4 ) == 0 )
				{
					if( gpPackInfo->Aux2 )
					{
						in += 4;
						out += sprintf( out, gpPackInfo->Aux2 );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("GameInfo1"), 9 ) == 0 )
				{
					if( gpPackInfo->GameTitle1 )
					{
						in += 8;
						didsubst++;
						out += sprintf( out, gpPackInfo->GameTitle1 );
						continue;
					}
				}
				else if( strncmp( in, WIDE("GameInfo2"), 9 ) == 0 )
				{
					if( gpPackInfo->GameTitle2 )
					{
						in += 8;
						didsubst++;
						out += sprintf( out, gpPackInfo->GameTitle2 );
						continue;
					}
				}
				else if( strncmp( in, WIDE("session"), 7 ) == 0 )
				{
					if( gpPackInfo->SessionID )
					{
						in += 6;
						out += sprintf( out, gpPackInfo->SessionID );
						didsubst++;
						continue;
					}
				}
				else if( strncmp( in, WIDE("Sort"), 4 ) == 0 )
				{
					Log( WIDE("Argument was a sort parameter...") );
					in += 4;
					if( in[0] == '(' )
					{
						int item = 0;
						in++;
						while( in[0] >= '0' && in[0] <= '9' )
						{
							item *= 10;
							item += in[0] - '0';
							in++;
						}
						Log1( WIDE("Sort(Item:%d)"), item );
						nToSort = 0;
						while( in[0] == ',' )
						{
							int value = 0;
							in++;
							while( in[0] >= '0' && in[0] <= '9' )
							{
								value *= 10;
								value += in[0] - '0';
								in++;
							}
							Log1( WIDE("Sort New Item: %d"), value );
							pSortList[nToSort++] = value;
						}
						if( in[0] != ')' )
						{
							out += sprintf( out, WIDE("%%Sort(%d,"), item );
							Log( WIDE("Bad format, or other substitutions must be done first") );
							in--; // we're too far cause loop auto advances at continue...
							continue;
						}
						else
						{
							SortList();
							Log1( WIDE("Sort result: %d"), pSortList[item-1] );
							out += sprintf( out, WIDE("%d"), pSortList[item-1] );
							didsubst++;
							//in--; // we're too far cause loop auto advances at continue...
							continue;
						}
					}
				}
				Log1( WIDE("Unknown substitution (%s)- copy literal.\n"), in );
				*out = '%';
				out++;
			}
			else
			{
				didsubst++;
				while( text && *text )
				{
					*out = *text;
				 	out++;
				 	text++;
				}
				continue; // 
			}
		}
		*out = *in;
		out++;
	}
	*out = 0;
   CollapseQuotes( dest );
	return didsubst;
}

//-------------------------------------------------------------------------

void PutImageOnDevice( PDEVICE device
							, int x, int y
							, int w, int h
							, Image pImage )
{
   BlotScaledImageSizedTo( device->image, pImage, x, y, w, h );
}



//--------------------------------------------------------------------------

void DrawTextElement( PPRINTPAGE page, PPRINTTEXT current )
{
	int x, y;
	char *outtext;
	char buffer[2][256];
	int buf = 0;
	char *text;
   _32 w, h;
	Log1( WIDE("Text Element: %s"), current->text );
	strcpy( buffer[1-buf], current->text );
	while( SubstPrint( buffer[buf], buffer[1-buf], NULL ) )
			buf = 1-buf;
	outtext = buffer[buf];
	//SubstPrint( outtext, current->text, NULL );
	if( strchr( outtext, '%' ) ) // abort usage of this thing...
		return;
   text = outtext;
   //SetBkMode( page->output.hdc, TRANSPARENT );
	if( current->position.x.denominator < 0 )
	{
		x = page->output.width + ( ( page->output.scalex * current->position.x.numerator ) /
							( current->position.x.denominator ) );
	}
	else
	{
		x = ( ( page->output.scalex * current->position.x.numerator ) /
							( current->position.x.denominator ) );
	}
	if( current->position.y.denominator < 0 )
	{
		y = page->output.height + ( ( page->output.scaley * current->position.y.numerator ) /
							( current->position.y.denominator ) );
	}
	else
	{
		y = ( ( page->output.scaley * current->position.y.numerator ) /
							( current->position.y.denominator ) );
	}
	
GetStringSizeFont( text, &w, &h, current->font->hFont );
	//SelectObject( page->output.hdc, current->font->hFont );
	if( current->font->flags.center )
	{
		SetTextAlign( page->output.hdc, TA_CENTER|TA_TOP );
	}
	else
		SetTextAlign( page->output.hdc, TA_LEFT|TA_TOP );

	SetTextColor( page->output.hdc, current->font->color );
	while( *text )
	{
		char *end = text, save = 0;
		while( *end && end[0] != '\n' && end[0] != '\r' ) end++;
		if( save = *end )
			*end = 0;
		Log1( WIDE("Draw: %s"), text );
		ExtTextOut( page->output.hdc
						, x, y
						, 0, NULL
						, text, strlen( text ), NULL );
		if( save )
		{
			*end = save;
			while( *end && ( end[0] == '\n' || end[0] == '\r' ) ) end++;
			y += ( page->output.scaley * current->font->size.y.numerator  )/
						current->font->size.y.denominator;
		}
		text = end;
	}
}           	

void DrawTexts( PPRINTPAGE page )
{
	PPRINTTEXT current;
	int x, y;

   SetBkMode( page->output.hdc, TRANSPARENT );
	for( current = page->texts; current; current = current->next )
	{
		DrawTextElement( page, current );
	}
}

//--------------------------------------------------------------------------

void DrawRect( PPRINTPAGE page, PPRINTRECT current )
{

	HBRUSH hbrush;
	RECT rc;
	{
	   LOGBRUSH lbr;
  	lbr.lbStyle = BS_SOLID;
     lbr.lbColor = current->color;
	   lbr.lbHatch = 0; // ignored
		hbrush = (HBRUSH)CreateBrushIndirect( &lbr );
	}
	if( current->location.x.denominator < 0 )
	{
		rc.left = page->output.width + ( page->output.scalex* current->location.x.numerator ) /
							( current->location.x.denominator );
	}
	else
	{
		rc.left = ( page->output.scalex* current->location.x.numerator ) /
							( current->location.x.denominator );
	}
	rc.right = rc.left + ( page->output.scalex* current->size.width.numerator ) /
						( current->size.width.denominator );
	if( current->location.y.denominator < 0 )
	{
		rc.top =  page->output.height + ( page->output.scaley* current->location.y.numerator ) /
							( current->location.y.denominator );
	}
	else
	{
		rc.top =  ( page->output.scaley* current->location.y.numerator ) /
							( current->location.y.denominator );
	}

	rc.bottom = rc.top +  ( page->output.scaley* current->size.height.numerator ) /
						( current->size.height.denominator );
	FillRect( page->output.hdc, &rc, hbrush );
	DeleteObject( hbrush );

}

void DrawRects( PPRINTPAGE page )
{
	PPRINTRECT current;
	for( current = page->rects; current; current = current->next )
	{
		DrawRect( page, current );
	}	
}

//--------------------------------------------------------------------------

void DrawLine( PPRINTPAGE page, PPRINTLINE current )
{
	HPEN hPen, hOld;
	int x, y;
	/*
	Log8( WIDE("Drawing a line...(%d/%d %d/%d)-(%d/%d %d/%d)\n")
				,current->from.x.numerator, current->from.x.denominator
				,current->from.y.numerator, current->from.y.denominator
				,current->to.x.numerator, current->to.x.denominator
				,current->to.y.numerator, current->to.y.denominator
						);
	*/
   hPen = CreatePen( PS_SOLID, 1, current->color );
   hOld = SelectObject( page->output.hdc, hPen );
	if( current->from.x.denominator < 0 )
	{
		x = page->output.width + ( ( page->output.scalex * current->from.x.numerator ) /
							( current->from.x.denominator ) );
	}
	else
	{
		x = ( ( page->output.scalex * current->from.x.numerator ) /
							( current->from.x.denominator ) );
	}
	if( current->from.y.denominator < 0 )
	{
		y = page->output.height + ( ( page->output.scaley * current->from.y.numerator ) /
							( current->from.y.denominator ) );
	}
	else
	{
		y = ( ( page->output.scaley * current->from.y.numerator ) /
							( current->from.y.denominator ) );
	}

   MoveToEx( page->output.hdc, x, y, NULL );
	if( current->to.x.denominator < 0 )
	{
		x = page->output.width + ( ( page->output.scalex * current->to.x.numerator ) /
							( current->to.x.denominator ) );
	}
	else
	{
		x = ( ( page->output.scalex * current->to.x.numerator ) /
							( current->to.x.denominator ) );
	}
	if( current->to.y.denominator < 0 )
	{
		y = page->output.height + ( ( page->output.scaley * current->to.y.numerator ) /
							( current->to.y.denominator ) );
	}
	else
	{
		y = ( ( page->output.scaley * current->to.y.numerator ) /
							( current->to.y.denominator ) );
	}
   LineTo( page->output.hdc, x, y );

	SelectObject( page->output.hdc, hOld );
	DeleteObject( hPen );
}

void DrawLines( PPRINTPAGE page )
{
	PPRINTLINE current;
	for( current = page->lines; current; current = current->next )
	{
		DrawLine( page, current );
	}
}

//--------------------------------------------------------------------------

#define INCHES(over,under) ( (scalex * over) / ( under ) )
#define VINCHES(over,under) ( (scaley * over) / ( under ) )
        
//--------------------------------------------------------------------------

void DrawBarcode( PPRINTPAGE page, PPRINTBARCODE current  )
{
//int width, int height, int scalex, int scaley
	HPEN hWhite;
	HPEN hBlack;
	HPEN oldpen = 0, priorpen;
	int line, base, top;
	int xfrom, xto;
	int step = MulDiv( 2, GetDeviceCaps( page->output.hdc, LOGPIXELSY ), 96 );
	return;
	// vertical barcode only ?
	hWhite = CreatePen( PS_SOLID, MulDiv( 2, GetDeviceCaps( page->output.hdc, LOGPIXELSY ), 96 ), RGB( 255, 255, 255 ) );
	hBlack = CreatePen( PS_SOLID, MulDiv( 2, GetDeviceCaps( page->output.hdc, LOGPIXELSY ), 96 ), RGB( 0, 0, 0 ) );

	xfrom = ( ( page->output.scalex * current->origin.x.numerator ) /
						( current->origin.x.denominator ) );
	if( current->origin.x.denominator < 0 )
		xfrom += page->output.width;

	top = ( ( page->output.scaley * current->origin.y.numerator ) /
						( current->origin.y.denominator ) );
	if( top  < 0 )
		top += page->output.height;

	xto = xfrom + ( ( page->output.scalex * current->size.x.numerator ) /
						( current->size.x.denominator ) );

	base = base + ( ( page->output.scaley * current->size.y.numerator ) /
						( current->size.y.denominator ) );
	if( top < base )
	{
		int tmp;
		tmp = top;
		top = base;
		base = tmp;
	}
	if( xfrom > xto )
	{
		int tmp;
		tmp = xfrom;
		xfrom = xto;
		xto = tmp;
	}

	for( line = base; line < top; line += step )
	{
		if( rand() & 4 ) // test any single bit for 50/50
		{
			priorpen = SelectObject( page->output.hdc, hBlack );
		}
		else
			priorpen = SelectObject( page->output.hdc, hWhite );
		if( !oldpen )
			oldpen = priorpen;
		MoveToEx( page->output.hdc, xfrom, line, NULL );
		LineTo( page->output.hdc, xto, line );
	}
	SelectObject( page->output.hdc, oldpen );
	DeleteObject( hWhite );
	DeleteObject( hBlack );
}


void DrawBarcodes( PPRINTPAGE page )
{
	PPRINTBARCODE current;
	for( current = page->barcodes; current; current = current->next )
		DrawBarcode( page, current );
}

//--------------------------------------------------------------------------

void DrawImage( PPRINTPAGE page, PPRINTIMAGE current )
{
	int x, y;
	int w, h;
	if( current->location.x.denominator < 0 )
	{
		x = page->output.width + ( ( page->output.scalex * current->location.x.numerator ) /
							( current->location.x.denominator ) );
	}
	else
	{
		x = ( ( page->output.scalex * current->location.x.numerator ) /
							( current->location.x.denominator ) );
	}
	if( current->location.y.denominator < 0 ) 
	{
		y = page->output.height + ( ( page->output.scaley * current->location.y.numerator ) /
							( current->location.y.denominator ) );
	}
	else	
	{
		y = ( ( page->output.scaley * current->location.y.numerator ) /
							( current->location.y.denominator ) );
	}

	if( current->size.x.denominator < 0 )
	{
		w = page->output.width + ( ( page->output.scalex * current->size.x.numerator ) /
							( current->size.x.denominator ) );
	}
	else
	{
		w = ( ( page->output.scalex * current->size.x.numerator ) /
							( current->size.x.denominator ) );
	}
	if( current->size.y.denominator < 0 ) 
	{
		h = page->output.height + ( ( page->output.scaley * current->size.y.numerator ) /
							( current->size.y.denominator ) );
	}
	else	
	{
		h = ( ( page->output.scaley * current->size.y.numerator ) /
							( current->size.y.denominator ) );
	}

	PutImageOnDevice( &page->output
				, x, y
				, w, h
				, current->image );
}

void DrawImages( PPRINTPAGE page )
{
	PPRINTIMAGE current;
	for( current = page->images; current; current = current->next )
		DrawImage( page, current );
}

//---------------------------------------------------------------------------


void RunDrawMacro( PPRINTPAGE page, PPRINTMACRORUN macro )
{
	PPRINTMACRO code = macro->macro;
	PPRINTOPERATION op;
	INDEX idx;
	char buffer[2][256];
	int buf = 0;
	//Log( WIDE("Running a macro..."));
	LIST_FORALL( code->Operations, idx, PPRINTOPERATION, op )
	{
		//Log1( WIDE("Statement: %s"), opnames[op->nType] );
		switch( op->nType )
		{
		case PO_LINE:
			{
				PRINTLINE line = *op->data.line;
				/*
				Log4( WIDE("Macro's origin: %d/%d %d/%d")
								, macro->Origin.x.numerator, macro->Origin.x.denominator
								, macro->Origin.y.numerator, macro->Origin.y.denominator );
				*/
				AddCoords( &line.from, &macro->Origin );
				AddCoords( &line.to, &macro->Origin );
				DrawLine( page, &line );
			} 
			break;
		case PO_TEXT:
			{
				PRINTTEXT text = *op->data.text;
				/*
				Log4( WIDE("Macro's origin: %d/%d %d/%d")
								, macro->Origin.x.numerator, macro->Origin.x.denominator
								, macro->Origin.y.numerator, macro->Origin.y.denominator );
				*/
				AddCoords( &text.position, &macro->Origin );
				strcpy( buffer[1-buf], op->data.text->text );
				while( SubstPrint( buffer[buf], buffer[1-buf], macro ) )
					buf = 1-buf;
				text.text = buffer[buf];
				DrawTextElement( page, &text );
			}
			break;
		case PO_RECT:
			{
				PRINTRECT rect = *op->data.rect;
				AddCoords( &rect.location, &macro->Origin );
				DrawRect( page, &rect );
			}
			break;
		case PO_IMAGE:
			{
				PRINTIMAGE image = *op->data.image;
				AddCoords( &image.location, &macro->Origin );
				DrawImage( page, &image );
			}
			break;
		case PO_BARCODE:
			{
				PRINTBARCODE barcode = *op->data.barcode;
				AddCoords( &barcode.origin, &macro->Origin );
				DrawBarcode( page, &barcode );
			}
			break;
		case PO_VALIDATE:
			{
				strcpy( buffer[1-buf], op->data.validate->text );
				while( SubstPrint( buffer[buf], buffer[1-buf], macro ) )
					buf = 1-buf;
				if( strchr( buffer[buf], '%' ) ) // incomplete substituion
				{
					Log1( WIDE("Validation...FAILED :%s"), buffer[buf] );
					return;
				}	
				Log1( WIDE("Validation...SUCCESS :%s"), buffer[buf] );
			}
         break;
      //case PO_MACRO:
      //   break;
		case PO_RUNMACRO:
			{
				PRINTMACRORUN newmacro = *op->data.runmacro;
				PLIST newlist = NULL, oldlist;
				char *text;
            INDEX idx;
            newmacro.priorrunning = macro;
				AddCoords( &newmacro.Origin, &macro->Origin );
				LIST_FORALL( newmacro.Values, idx, char*, text )
				{
					strcpy( buffer[1-buf], text );
					while( SubstPrint( buffer[buf], buffer[1-buf], macro ) )
						buf = 1-buf;
					text = Allocate( strlen( buffer[buf] ) + 1 );
					strcpy( text, buffer[buf] );
					AddLink( &newlist, text );
				}
				newmacro.Values = newlist;
				AddCoords( &newmacro.OriginalOrigin, &macro->Origin );
				newmacro.Origin = newmacro.OriginalOrigin;
				RunDrawMacro( page, &newmacro );
				LIST_FORALL( newlist, idx, char*, text )
					Release( text );
				DeleteList( &newlist );
			}
			break;
		case PO_ORIGIN:
			macro->Origin = macro->OriginalOrigin;
			AddCoords( &macro->Origin, &op->data.origin );
			break;
		case PO_MOVE_ORIGIN:
		   // needs to result in a change in the parent's origin
         // the macro context this is in dissappears
			macro->Origin = macro->OriginalOrigin;
			AddCoords( &macro->Origin, &op->data.origin );
         break;
      case PO_PAGE:
         /*
         if( template->currentpage )
            Release( template->currentpage );
         {
            PPRINTPAGE newpage = Allocate( sizeof( PRINTPAGE ) );
            *newpage = *po->data.page;
            newpage->next = template->pages;
            template->pages = newpage;
            template->currentpage = newpage;
            }
            */
         break;
		}
	}
}

//---------------------------------------------------------------------------

void DrawMacros( PLIST macros, PPRINTPAGE page )
{
	PPRINTMACRORUN macro;
	INDEX idx;
	//Log( WIDE("Running all macros...") );
	LIST_FORALL( macros, idx, PPRINTMACRORUN,  macro )
   {
      char coords[50];
      macro->Origin = macro->OriginalOrigin;
      sLogCoords( coords, &macro->Origin );
      Log1( WIDE("Setting origin to : %s"), coords );
		RunDrawMacro( page, macro );
	}
}

//---------------------------------------------------------------------------

void RenderPage( PPRINTPAGE page )
{
	DrawImages( page );
	DrawRects( page );
	DrawTexts( page );
	DrawLines( page );
   DrawBarcodes( page );

   DrawMacros( page->MacroList, page );
   RunSQL( page->SQL, page );
}

//---------------------------------------------------------------------------

void RenderTemplate( PTEMPLATE template )
{
   PPRINTPAGE page;
	DrawMacros( template->MacroList, NULL );
   page = template->pages;
   while( page )
   {
      RenderPage( page );
      page = page->next;
   }
}

//---------------------------------------------------------------------------
