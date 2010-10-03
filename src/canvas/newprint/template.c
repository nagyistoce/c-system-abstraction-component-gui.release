#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <stdio.h> // sprintf

#include <sharemem.h>
#include <image.h>
#include <logging.h>

#include "template.h"

static PTEMPLATE templates;


int DoLoadTemplate( FILE *file, PTEMPLATE template );

//--------------------------------------------------------------------------

PTEMPLATE FindTemplate( char *name )
{
	PTEMPLATE current = templates;
	while( current )
	{
		if( !strncmp( name, current->name, current->namelen ) )
			return current;
		current = current->next;
	}
	return NULL;
}

//--------------------------------------------------------------------------

PPRINTMACRO FindMacro( char *name, PTEMPLATE template )
{
	PPRINTMACRO current = template->macros;
	while( current )
	{
		if( !strncmp( name, current->name, current->namelen ) )
		{
			return current;
		}
		current = current->next;
	}
	return NULL;
}

//--------------------------------------------------------------------------

PPRINTFONT FindFont( char *name, PPRINTPAGE page )
{
	PPRINTFONT current = page->fonts;
	while( current )
	{
		if( !strcmp( current->name, name ) )
		{
			return current;
		}
		current = current->next;
	}	
	return NULL;
}

//---------------------------------------------------------------------------

// defines common to line readers/handlers
#define CHAR ((*string)[0])
#define NEXTCHAR ((*string)++)
#define SKIPSPACES()		while( CHAR && ( CHAR == ' ' || CHAR == '\t' ) ) NEXTCHAR;
#define SKIPTOSPACE()	while( CHAR && ( CHAR != ' ' && CHAR != '\t' ) ) NEXTCHAR;

//---------------------------------------------------------------------------

void GetFractions( char **string, PCOORDPAIR pair )
{
	int step;
	int accum1, accum2, accum3;
	int neg;
	if( !string )
		return;
	pair->x.denominator = 0;
	pair->y.denominator = 0;
	SKIPSPACES();
	if( CHAR != '(' )
	{
		Log( WIDE("Ill formed fraction\n"));
		return;
	}
	NEXTCHAR;

	for( step = 0; step < 2; step++ )
	{
		SKIPSPACES();
		neg = 0;
		if( step )
		{
			while( CHAR && CHAR != ',' )
			{
				// Error here also...
				Log( WIDE("Garbage after first fraction...") );
				NEXTCHAR;
			}
			if( !CHAR )
			{
				Log( WIDE("Must seperate fractions with a ','") );
				return;
			}
			NEXTCHAR;
			SKIPSPACES();
		}
		accum1 = 0;
		accum2 = 0;
		accum3 = 0;

		if( CHAR == '-' )
		{
			NEXTCHAR;
			SKIPSPACES();
			neg = !neg;
		}
		while( CHAR <= '9' && CHAR >= '0' )
		{
			accum1 *= 10; 
			accum1 += CHAR - '0';
			NEXTCHAR;
		}

		if( accum1 == 0 )
		{
			if( !step )
			{
				pair->x.numerator = 0;
				pair->x.denominator = 1;
	  			if( neg )
	  				pair->x.denominator *= -1;
			}
			else
			{
				pair->y.numerator = 0;
				pair->y.denominator = 1;
   			if( neg )
   				pair->y.denominator *= -1;
			}
			continue;
		}
		SKIPSPACES();
		if( CHAR != '/' )
		{
			if( CHAR == ',' )
			{
				if( !step )
				{
				 	pair->x.numerator = accum1;
			 		if( neg )
	   				pair->x.denominator = -1;
					else
                  pair->x.denominator = 1;
				}
				else
				{
					// error here - triple comma sepeartor....
				 	pair->y.numerator = accum1;
				 	pair->y.denominator = 1;
	   			if( neg )
	   				pair->y.denominator *= -1;
				}	
				// remain on this character for above validation...
			 	continue;
			}
			else if( CHAR == ')' )
			{
				if( step )
				{
				 	pair->y.numerator = accum1;
				 	pair->y.denominator = 1;
	   			if( neg )
	   				pair->y.denominator *= -1;
				}
				else
				{
					Log( WIDE("Premature ending of fractions - only one gathered") );
				}
				continue;
			}
			else
			{
				while( CHAR && CHAR <= '9' && CHAR >= '0' )
				{
					accum2 *= 10;
					accum2 += CHAR - '0';
					NEXTCHAR;
				}
				if( !accum2 )
				{
					// error here!
				}
			}
		}

		SKIPSPACES();
		if( CHAR != '/' )
		{
			// error here - double value no / or , to sperate..
		}
		else
		{	
			NEXTCHAR;
			while( CHAR <= '9' && CHAR >= '0' )
			{
				accum3 *= 10;
				accum3 += CHAR - '0';
				NEXTCHAR;
			}
			if( !accum3 )
			{
				// zero demonitor... this is sooo bad...
				Log( WIDE("Zero denominator is soooo bad.") );
			}
			else
			{
				if( !step )
				{
					if( accum2 )
					{
						pair->x.numerator = accum1 * accum3 + accum2;
						pair->x.denominator = (neg)?-accum3:accum3;
					}
					else
					{
						pair->x.numerator = accum1;
						pair->x.denominator = (neg)?-accum3:accum3;
					}
				}
				else
				{
					if( accum2 )
					{
						pair->y.numerator = accum1 * accum3 + accum2;
						pair->y.denominator = (neg)?-accum3:accum3;
					}
					else
					{
						pair->y.numerator = accum1;
						pair->y.denominator = (neg)?-accum3:accum3;
					}         
				}
			}
   	}
	}
	SKIPSPACES();
	if( CHAR == ')' )
		NEXTCHAR;
	else
		Log1( WIDE("End of fraction is not ) it is %c"), CHAR );

	// should now either have an error - or we should have a valid pair...
}

//---------------------------------------------------------------------------


char HEX[17] = "0123456789ABCDEF";
char hex[17] = "0123456789abcdef";

_32 ReadColorHash( char **string )
{
	_32 accum = 0;
	int i;
	char *pos;
	SKIPSPACES();
	if( CHAR == '$' )
	{
		for( i = 0; i < 6; i++ )
		{
			NEXTCHAR;
			pos = strchr( HEX, CHAR );
			if( !pos )
			{
				pos = strchr( hex, CHAR );
				if( !pos )
					break;
				else
				{
					accum *= 16;
					accum += pos - hex;
				}
			}
			else
			{
				accum *= 16;
				accum += pos - HEX;
			}	
		}
		if( i == 6 ) // should be on the end of the last character...
			NEXTCHAR;
		else
		{
			// if I wasn't 6 - then it failed and was a short value...
			Log( WIDE("Look you didn't format the color right.") );
		}
	}
	return accum;
}

//---------------------------------------------------------------------------

void ReadImage( char **string, PTEMPLATE template )
{
	int l;
   int lighten = 0;
   PPRINTIMAGE NewImage;
   if( !template->currentpage )
   {
      Log( WIDE("Need a page to include the image on.") );
      return;
   }
	NewImage = Allocate( sizeof( PRINTIMAGE ) );
	MemSet( NewImage, 0, sizeof( PRINTIMAGE ) );
	SKIPSPACES();
	while( CHAR )
	{
		char buffer[256];
		if( !strncmp( *string, WIDE("at"), 2 ) )
		{
			*string += 2;
			GetFractions( string, &NewImage->location );
			sLogCoords( buffer, &NewImage->location );
			AddCoords( &NewImage->location, &template->currentpage->Origin );
			Log1( WIDE("Image at: %s"), buffer );
		}
		else if( !strncmp( *string, WIDE("size"), 4 ) )
		{
			*string += 4;
			GetFractions( string, &NewImage->size );
			sLogCoords( buffer, &NewImage->size );
			Log1( WIDE("Image sized: %s"), buffer );
		}
		else if( !strncmp( *string, WIDE("from"), 4 ) )
		{
			char *name;
			char save;
			*string += 4;
			SKIPSPACES();
			name = *string;
			SKIPTOSPACE();
			save = CHAR;
			CHAR = 0;
			NewImage->image = LoadImageFile( name );
			if( !NewImage->image )
			{
				char msg[256];
				sprintf( msg, WIDE("Failed to load image %s"), name );
				MessageBox( NULL, msg, WIDE("Template Error!"), MB_OK );
			}
			else
				Log1( WIDE("Loaded image %s"), name );
			CHAR = save;
			Log1( WIDE("remaining line is: %s"), *string );
		}
		else if( !strncmp( *string, WIDE("lighten"), 7 ) )
		{
		   int accum = 0;
         *string += 7;
			SKIPSPACES();
			while( CHAR >= '0' && CHAR <= '9' )
			{
				accum *= 10;
				accum += CHAR - '0';
				NEXTCHAR;
			}
			lighten = accum;
		}
		else
			NEXTCHAR;
	}

	if( (l=1),NewImage->image && 
	    (l=2),NewImage->location.width.denominator &&
	    (l=3),NewImage->location.height.denominator &&
	    (l=4),NewImage->size.width.denominator &&
	    (l=5),NewImage->size.height.denominator )
	{
		if( lighten )
		{
			BlatColorAlpha( NewImage->image, 0, 0
								, NewImage->image->width, NewImage->image->height
								, AColor( 255, 255, 255, lighten * 255 / 100 ) );
		}
		if( !template->MacroRecording )
		{
         NewImage->next = template->currentpage->images;
         template->currentpage->images = NewImage;
		}
		else
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_IMAGE;
			po->data.image = NewImage;
			AddLink( &template->MacroRecording->Operations, po );
		}
	}
	else
	{
		Log1( WIDE("Failed to load image completely...%d"), l );
	 	if( NewImage->image )
	 		UnmakeImageFile( NewImage->image );
		Release( NewImage );
	}
}

//---------------------------------------------------------------------------

void ReadRect( char **string, PTEMPLATE template )
{
	int lighten = 0;
	PPRINTRECT NewRect;
   if( !template->currentpage )
   {
      Log( WIDE("Need a page to include the rectangle on.") );
      return;
   }
   NewRect = Allocate( sizeof( PRINTRECT ) );
	MemSet( NewRect, 0, sizeof( PRINTRECT ) );
	SKIPSPACES();
	while( CHAR )
	{
		if( !strncmp( *string, WIDE("at"), 2 ) )
		{
			*string += 2;
         GetFractions( string, &NewRect->location );
			AddCoords( &NewRect->location, &template->currentpage->Origin );
		}
		else if( !strncmp( *string, WIDE("size"), 4 ) )
		{
			*string += 4;
			GetFractions( string, &NewRect->size );
		}
		else if( !strncmp( *string, WIDE("color"), 5 ) )
		{
			*string += 5;
			NewRect->color = ReadColorHash( string );
		}
		else
			NEXTCHAR;
	}
	if( NewRect->location.width.denominator &&
	    NewRect->location.height.denominator &&
	    NewRect->size.width.denominator &&
	    NewRect->size.height.denominator )
	{
		if( !template->MacroRecording )
		{
         NewRect->next = template->currentpage->rects;
         template->currentpage->rects = NewRect;
		}
		else
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_RECT;
			po->data.rect = NewRect;
			AddLink( &template->MacroRecording->Operations, po );
		}
	}
	else
	{
		Log( WIDE("Failed to load rect completely...") );
		Release( NewRect );
	}
}

//---------------------------------------------------------------------------

void ReadFont( char **string, PTEMPLATE template )
{
	char *name, save;
	PPRINTFONT NewFont = Allocate( sizeof( PRINTFONT ) );
	MemSet( NewFont, 0, sizeof( PRINTFONT ) );

	SKIPSPACES();
	name = *string;
	SKIPTOSPACE();
	save = CHAR;
	CHAR = 0;
	strcpy( NewFont->name, name );
   Log1( WIDE("Font name is: %s"), name );

	CHAR = save;

	SKIPSPACES();	
	while( CHAR )
	{
		if( !strncmp( *string, WIDE("fixed"), 5 ) )
		{
			NewFont->flags.fixed = TRUE;
			*string += 5;
		}
		else if( !strncmp( *string, WIDE("bold"), 4 ) )
		{
			NewFont->flags.bold = TRUE;
			*string += 4;
		}
		else if( !strncmp( *string, WIDE("invert"), 6 ) )
		{
			NewFont->flags.inverted = TRUE;
			*string += 6;
		}
		else if( !strncmp( *string, WIDE("vertical"), 8 ) )
		{
			Log( WIDE("Setting font vertical...") );
			NewFont->flags.vertical = TRUE;
			*string += 8;
		}
		else if( !strncmp( *string, WIDE("script"), 4 ) )
		{
			NewFont->flags.script = TRUE;
			*string += 4;
		}
		else if( !strncmp( *string, WIDE("center"), 6 ) )
		{
			NewFont->flags.center = TRUE;
			*string += 6;
		}
		else if( !strncmp( *string, WIDE("fits"), 4 ) )
		{
			char *text;
			*string += 4;
			SKIPSPACES();
			if( CHAR != '\"' )
			{
			 	Log( WIDE("bad fits string") );
			}
			NEXTCHAR;
			text = *string;
			while( CHAR && ( CHAR != '\"' ) )
				NEXTCHAR;
			if( !CHAR )
			{
				Log( WIDE("This thing isn't ended!\n") );
				return;
			}
			CHAR = 0;
			strcpy( NewFont->text, text );
			CHAR = '\"';
			NEXTCHAR;
			SKIPSPACES();
			if( CHAR=='i' )
			{
				NEXTCHAR;
				if( CHAR == 'n' )
					NEXTCHAR;
			}
			GetFractions( string, &NewFont->size );
		}
		else if( !strncmp( *string, WIDE("color"), 5 ) )
		{
			*string += 5;
			NewFont->color = ReadColorHash( string );
		}                                          
		else
			NEXTCHAR;
   }
   if( template->currentpage )
   {
      if( FindFont( name, template->currentpage ) )
      {
         Log1( WIDE("Font %s is being redefined.  Second instance will not be used"), name );
      }
      else
      {
         Log( WIDE("NEED TO VALIDATE FONT! (added to page)") );
         NewFont->next = template->currentpage->fonts;
         template->currentpage->fonts = NewFont;
      }
   }
   else if ( template->MacroRecording )
   {
      PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
      MemSet( po, 0, sizeof( PRINTOPERATION ) );
      po->nType = PO_FONT;
      po->data.font = NewFont;
      AddLink( &template->MacroRecording->Operations, po );
      Log( WIDE("NEED TO VALIDATE FONT! (recorded in macro)") );
   }
   else
   {
      Log( WIDE("Cannot define a font outside a macro, and/or outside a page") );
      Release( NewFont );
   }
}

//---------------------------------------------------------------------------

void ReadText( char **string, PTEMPLATE template )
{
   PPRINTTEXT NewText;
   if( !template->currentpage )
   {
      Log( WIDE("Need a page to include the text on.") );
      return;
   }
	NewText = Allocate( sizeof( PRINTTEXT ) );
	MemSet( NewText, 0, sizeof( PRINTTEXT ) );
	//Log1( WIDE("Reading text command: %s"), *string );
	while( CHAR )
	{
		if( !strncmp( *string, WIDE("at"), 2 ) )
		{
			// associate position with text thing...
			*string += 2;
         GetFractions( string, &NewText->position );
			AddCoords( &NewText->position, &template->currentpage->Origin );
		}
		else if( CHAR == '\"' )
		{
			// assocate text with text thing.
			char *text;
			NEXTCHAR;
			text = *string;
			while( CHAR && CHAR != '\"' )
				NEXTCHAR;
			if( !CHAR )
			{
				Log( WIDE("Text has no ending quote\n") );
				return;
			}
			else
			{
				char save;
				save = CHAR;
				CHAR = 0;
				NewText->text = Allocate( strlen( text ) + 1 );
				strcpy( NewText->text, text );
				CHAR = save;
				NEXTCHAR;
			}
		}
		else if( CHAR != ' ' && CHAR != '\t' ) 
		{
			// associate font with text thing.
			char *name;
			char save;
			PPRINTFONT font;
			name = *string;
			SKIPTOSPACE();
			save = CHAR;
         CHAR = 0;

			//NewText->font = FindFont( name, template );
			//if( !NewText->font )
			//{
			//	Log1( WIDE("Failed to find a font called \")%s\"\n", name );
         //}
         NewText->font = Allocate( strlen( name ) + 1 );
         strcpy( (char*)NewText->font, name );
			CHAR = save;
		}
		else 
			NEXTCHAR;
	}
	if( NewText->text && 
		 NewText->font &&
		 NewText->position.x.denominator &&
		 NewText->position.y.denominator )
	{
      if( template->currentpage )
      {
         PPRINTFONT font = FindFont( (char*)NewText->font
                                   , template->currentpage );
         if( !font )
         {
            Log1( WIDE("Font %s does not exist on current page..."), NewText->font );
         }
         else
         {
            Release( NewText->font );
            NewText->flags.resolvedfont = 1;
            NewText->font = font;
         }
      }

		if( !template->MacroRecording )
      {
         if( template->currentpage )
         {
            NewText->next = template->currentpage->texts;
            template->currentpage->texts = NewText;
         }
         else
         {
            Log( WIDE("Error storing text... no current page, not recording macro") );
            Release( NewText->text );
            Release( NewText->font );
            Release( NewText );
         }
		}
		else
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_TEXT;
			po->data.text = NewText;
         AddLink( &template->MacroRecording->Operations, po );
      }
      if( !NewText->flags.resolvedfont )
         Log( WIDE("Please resolve text font name before usage...") );
	}
	else
   {
      if( NewText->font )
         Release( NewText->font );
		if( NewText->text )
			Release( NewText->text );
		Release( NewText );
	}			
}

//---------------------------------------------------------------------------

void ReadLine( char **string, PTEMPLATE template )
{
   PPRINTLINE NewLine;
   if( !template->currentpage )
   {
      Log( WIDE("Need a page to include the line on.") );
      return;
   }
	NewLine = Allocate( sizeof( PRINTLINE ) );
	MemSet( NewLine, 0, sizeof( PRINTLINE ) );
	while( CHAR )
	{
		SKIPSPACES();
		if( CHAR == '(' )
		{
			GetFractions( string, &NewLine->from );
			AddCoords( &NewLine->from, &template->currentpage->Origin );
		}
		else if( CHAR == '-' )
		{
			NEXTCHAR;
			GetFractions( string, &NewLine->to );
			AddCoords( &NewLine->to, &template->currentpage->Origin );
		}
		else if( strncmp( *string, WIDE("color"), 5 ) )
		{
			*string += 5;
			NewLine->color = ReadColorHash( string );
		}
		else
			NEXTCHAR;
	}
	if(  NewLine->from.x.denominator == 0
	  || NewLine->from.y.denominator == 0
	  || NewLine->to.x.denominator == 0
	  || NewLine->to.y.denominator == 0 )
	{
		Release( NewLine );
	}
	else
	{
		if( !template->MacroRecording )
		{
			NewLine->next = template->currentpage->lines;
         template->currentpage->lines = NewLine;
		}				
		else
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_LINE;
			po->data.line = NewLine;
			AddLink( &template->MacroRecording->Operations, po );
		}
	}
}

//---------------------------------------------------------------------------

void ReadMacro( char **string, PTEMPLATE template )
{
	PPRINTMACRO NewMacro = Allocate( sizeof( PRINTMACRO ) );
   MemSet( NewMacro, 0, sizeof( PRINTMACRO ) );
   Log( WIDE("Begin macro...") );
	while( CHAR )
	{
		char *name, save;
		SKIPSPACES();
		name = *string;
		SKIPTOSPACE();

		save = CHAR;
		CHAR = 0;
		if( !NewMacro->name )
		{
			NewMacro->namelen = strlen( name );
         NewMacro->name = Allocate( NewMacro->namelen + 1 );
         if( FindMacro( NewMacro->name, template ) )
         {
            Log1( WIDE("Redefining macro: %s current will never execute"), NewMacro->name );
         }
			strcpy( NewMacro->name, name );
		}
		else
		{
			int len = strlen( name );
			char *var = Allocate( len + 1 );
			strcpy( var, name );
			//Log1( WIDE("Adding param name:%s"), var );
			AddLink( &NewMacro->Parameters, var );
		}
		CHAR = save;
	}
	if( NewMacro->name )
   {
      NewMacro->priorrecording = template->MacroRecording;
      // always promote all macros to top level in template
      // re-running macros which define macros will cause our
      // pages to grow each time.
      NewMacro->next = template->macros;
      template->macros = NewMacro;

      template->MacroRecording = NewMacro;
	}
	else
		Release( NewMacro );
}

//---------------------------------------------------------------------------
// applies to current macro recording...
PLIST ReadParameters( char **string )
{
	PLIST temp = NULL;
	char *var, save, *newvar, quote = 0;
	SKIPTOSPACE();
	SKIPSPACES();
	while( CHAR )
	{
		if( !quote )
		{
			if( CHAR == '\"' || CHAR == '\'' )
			{
				quote = CHAR;
				NEXTCHAR;
				var = *string;
				continue;
			}
			var = *string;
			SKIPTOSPACE();
			save = CHAR;
			CHAR = 0;
			newvar = Allocate( strlen( var ) + 1 );
			strcpy( newvar, var );
			AddLink( &temp, newvar );
			CHAR = save;
			SKIPSPACES();
		}
		else
		{
			if( quote == CHAR )
			{
				quote = 0;
				save = CHAR;
				CHAR = 0;
				newvar = Allocate( strlen( var ) + 1 );
				strcpy( newvar, var );
				AddLink( &temp, newvar );
				CHAR = save;
				NEXTCHAR;
				SKIPSPACES();
			}
			else
				NEXTCHAR;
		}
	}
	return temp;
}

//---------------------------------------------------------------------------

void ReadSQL( char **string, PTEMPLATE template )
{
// format of a SQL line ...
   // SQL (macro to invoke) (SQL query string)
}

//---------------------------------------------------------------------------

void ReadPage( char **string, PTEMPLATE template )
{
	// define page size here...
	PPRINTPAGE newpage = Allocate( sizeof( PRINTPAGE ) );
	MemSet( newpage, 0, sizeof( PRINTPAGE ) );
   newpage->Origin.x.denominator = 1;
   newpage->Origin.y.denominator = 1;
   newpage->DefinedIn = template->MacroRecording;
	while( CHAR && CHAR != '=' )
		NEXTCHAR;
	
	if( CHAR )
		NEXTCHAR;
	GetFractions( string, &newpage->pagesize );
	if( !newpage->pagesize.width.denominator ||
	    !newpage->pagesize.height.denominator )
	{
		Log( WIDE("not a valid pagesize\n") );
	}
	Log4( WIDE("Page is now %d/%d by %d/%d\n")
				, newpage->pagesize.width.numerator, newpage->pagesize.width.denominator
            , newpage->pagesize.height.numerator, newpage->pagesize.height.denominator );

	while( CHAR )
	{
		if( strncmp( *string, WIDE("portrait"), 8 ) == 0 )
		{
			*string += 7;
			newpage->flags.bLandscape = FALSE;
		}
		else if( strncmp( *string, WIDE("landscape"), 9 ) == 0 )
		{
			*string += 8;
			newpage->flags.bLandscape = TRUE;
		}
		NEXTCHAR;
	}
	
	if( newpage->flags.bLandscape )
	{
		FRACTION temp;
		temp = newpage->pagesize.width;
		newpage->pagesize.width = newpage->pagesize.height;
      newpage->pagesize.height = temp;
	}

   if( !template->MacroRecording )
   {
   	PPRINTPAGE lastpage;
   	lastpage = template->pages;
   	while( lastpage && lastpage->next )
   		lastpage = lastpage->next;
   	if( lastpage )
   	{
   		lastpage->next = newpage;
   	}
   	else
   		template->pages = newpage;
	   template->currentpage = newpage;
	}
	else
	{
		PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
		MemSet( po, 0, sizeof( PRINTOPERATION ) );
		po->nType = PO_PAGE;
		po->data.page = newpage;
      AddLink( &template->MacroRecording->Operations, po );
	}
}

//---------------------------------------------------------------------------

void ReadInclude( char **string, PTEMPLATE template )
{
	char save, quote = 0, *var;
	SKIPSPACES();
	while( CHAR )
	{
		if( !quote )
		{
			if( CHAR == '\"' || CHAR == '\'' )
			{
				quote = CHAR;
				NEXTCHAR;
				var = *string;
				continue;
			}
			NEXTCHAR;
		}
		else
		{
			if( quote == CHAR )
			{
				FILE *fin;
				quote = 0;
				save = CHAR;
				CHAR = 0;
				fin = fopen( var, WIDE("rt") );
				if( fin )
					DoLoadTemplate( fin, template );
				CHAR = save;
				NEXTCHAR;
				SKIPSPACES();
			}
			else
				NEXTCHAR;
		}
	}
	
}

//---------------------------------------------------------------------------

void ReadBarcode( char **string, PTEMPLATE template )
{
   PPRINTBARCODE newcode;
   if( !template->currentpage )
   {
      Log( WIDE("Need a current page to put a barcode on.") );
      return;
   }
	newcode = Allocate( sizeof( PRINTBARCODE ) );
	MemSet( newcode, 0, sizeof( PRINTBARCODE ) );
	while( CHAR )
	{
		SKIPSPACES();
		if( strncmp( *string, WIDE("vertical"), 8 ) == 0 )
		{
			newcode->flags.vertical = TRUE;
			SKIPTOSPACE();
		} 
		else if( strncmp( *string, WIDE("at"), 2 ) == 0 )
		{
			*string+=2;
			GetFractions( string, &newcode->origin );
		}
		else if( strncmp( *string, WIDE("fits"), 4 ) == 0 )
		{
			*string+=4;
			GetFractions( string, &newcode->size );
		}
		else if( CHAR == '\"' )
		{
			// assocate text with text thing.
			char *text;
			NEXTCHAR;
			text = *string;
			while( CHAR && CHAR != '\"' )
				NEXTCHAR;
			if( !CHAR )
			{
				Log( WIDE("Barcode text has no ending quote\n") );
				return;
			}
			else
			{
				char save;
				save = CHAR;
				CHAR = 0;
				newcode->value = Allocate( strlen( text ) + 1 );
				strcpy( newcode->value, text );
				CHAR = save;
				NEXTCHAR;
			}
		}
		else
			NEXTCHAR;
	}
	if( !newcode->value ||
	    !newcode->origin.x.denominator ||
	    !newcode->origin.y.denominator || 
	    !newcode->size.x.denominator ||
	    !newcode->size.y.denominator )
	{
		Log( WIDE("Error reading barcode line.") );
		if( newcode->value )
			Release( newcode->value );
		Release( newcode );
	}
	else
	{
		if( !template->MacroRecording )
		{
			newcode->next = template->currentpage->barcodes;
			template->currentpage->barcodes = newcode;
		}
		else
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_BARCODE;
			po->data.barcode = newcode;
			AddLink( &template->MacroRecording->Operations, po );
		}
	}
}     

//---------------------------------------------------------------------------

#undef SKIPSPACES
#undef NEXTCHAR
#undef CHAR

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int RunSingleLine( char *string, PTEMPLATE template )
{
	if( strncmp( string, WIDE("page "), 5 ) == 0 )
	{
		// define page size here...
		string += 4;
      ReadPage( &string, template );
	}
	else if( strncmp( string, WIDE("include "), 8 ) == 0 )
	{
		string += 7;
		ReadInclude( &string, template );
	}
	else if( strncmp( string, WIDE("image "), 6 ) == 0 )
	{
		string += 5;
		ReadImage( &string, template );
	}                               
	else if( strncmp( string, WIDE("font "), 5 ) == 0 )
	{
		string += 4;
		ReadFont( &string, template );
	}
	else if( strncmp( string, WIDE("text "), 5 ) == 0 )
	{
		string += 4;
		ReadText( &string, template );
	}
	else if( strncmp( string, WIDE("rect "), 5 ) == 0 )
	{
		string += 4;
		ReadRect( &string, template );
	}
	else if( strncmp( string, WIDE("move_origin "), 12 ) == 0 )
	{
		COORDPAIR temporigin;
		string += 12;
		GetFractions( &string, &temporigin );
		if( template->MacroRecording )
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_MOVE_ORIGIN;
			po->data.origin = temporigin;
			AddLink( &template->MacroRecording->Operations, po );
		}
      else
		{
         lprintf( WIDE("Move_origin is intended for macro use...") );
         if( template->currentpage )
			{
            AddCoords( &template->currentpage->Origin, &temporigin );
            //template->currentpage->Origin = temporigin;
            {
               char coords[50];
               sLogCoords( coords, &temporigin );
               Log1( WIDE("Set Page origin to : %s"), coords );
            }
         }
         else
            Log( WIDE("Origins are associated with visuals, need a page for visuals, no page, no origin.") );
      }
	}
	else if( strncmp( string, WIDE("origin "), 7 ) == 0 )
	{
		COORDPAIR temporigin;
		string += 6;
		GetFractions( &string, &temporigin );
		if( template->MacroRecording )
		{
			PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
			MemSet( po, 0, sizeof( PRINTOPERATION ) );
			po->nType = PO_ORIGIN;
			po->data.origin = temporigin;
			AddLink( &template->MacroRecording->Operations, po );
		}
      else
      {
         if( template->currentpage )
         {
            template->currentpage->Origin = temporigin;
            {
               char coords[50];
               sLogCoords( coords, &temporigin );
               Log1( WIDE("Set Page origin to : %s"), coords );
            }
         }
         else
            Log( WIDE("Origins are associated with visuals, need a page for visuals, no page, no origin.") );
      }
	}
	else if( strncmp( string, WIDE("barcode"), 7 ) == 0 )
	{
		string += 7;
		ReadBarcode( &string, template );
	}
	else if( strncmp( string, WIDE("line"), 4 ) == 0 )
	{
		string += 4;
		ReadLine( &string, template );
	}
	else if( strncmp( string, WIDE("macro"), 5 ) == 0 )
	{
		string += 5;
		ReadMacro( &string, template );
	}
	else if( strncmp( string, WIDE("SQL"), 3 ) == 0 )
	{
		string += 3;
		ReadSQL( &string, template );
	}
	else if( strncmp( string, WIDE("validate"), 8 ) == 0 )
	{
		if( template->MacroRecording )
		{
			char text[256];
			PPRINTVALIDATE validate = Allocate( sizeof( PRINTVALIDATE ) );
			MemSet( validate, 0, sizeof( PRINTVALIDATE ) );
			string += 8;
			validate->text = Allocate( strlen( string )  + 1);
			strcpy( validate->text, string );
			{
				PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
				MemSet( po, 0, sizeof( PRINTOPERATION ) );
				po->nType = PO_VALIDATE;
				po->data.validate = validate;
				AddLink( &template->MacroRecording->Operations, po );
			}
		}
	}
	else
	{
		PPRINTMACRO macro;
		while( string[0] == ' ' || string[0] == '\t' )
			string++;
		macro = FindMacro( string, template );
		if( macro )
		{     
			// parse arguments, invoke macro (generate real code)
			PLIST tempvars;
			PPRINTMACRORUN pmr;
         tempvars = ReadParameters( &string );
	      pmr = Allocate( sizeof( PRINTMACRORUN ) );

			pmr->macro = macro;
         pmr->Values = tempvars;
         if( template->currentpage )
         {
            pmr->OriginalOrigin = template->currentpage->Origin;
            {
               char coords[50];
               sLogCoords( coords, &pmr->OriginalOrigin );
               Log1( WIDE("Set Macro origin to : %s"), coords );
            }
         }
         else
            Log( WIDE("Just a note: We're running a macro outside a page...") );
			pmr->priorrunning = NULL;
         if( !template->MacroRecording )
         {
            // list of macros to be run!....
            if( !template->currentpage )
            {
               // invoke the macro? to define the page?
               AddLink( &template->MacroList, pmr );
            }
            else
               AddLink( &template->currentpage->MacroList, pmr );
			}
			else
			{
				PPRINTOPERATION po = Allocate( sizeof( PRINTOPERATION ) );
				MemSet( po, 0, sizeof( PRINTOPERATION ) );
				po->nType = PO_RUNMACRO;
            po->data.runmacro = pmr;
				AddLink( &template->MacroRecording->Operations, po );
			}
		}
	}
	return 0;
}


int DoLoadTemplate( FILE *templatefile, PTEMPLATE template )
{
	char line[256];
	char *string;
	while( fgets( line, 256, templatefile ) )
	{
		//Log1( WIDE("Line: %s"), line );
		string = strchr( line, '#' );
		if( string )
		{
			if( line != string )
			{
				// allow \# to be a # character...

				while( string && string[-1] == '\\' )
				{
					strcpy( string-1, string );
         	   string = strchr( string, '#' );
            }
			}
			if( string )
				*string = 0; // trim any comments.
		}
		// set our working index to start of the buffer...
		string = line;
		if( strlen( line ) > 255 )
			Log( WIDE("Line is too long - more than 255 characters\n") );
		else
		{
			if( line[strlen(line)-1] == '\n' )
				line[strlen(line)-1] = 0; // strip the \n from the line.
			// otherwise it might be the last line without a \n
		}
		while( string[0] && ( string[0] == ' '|| string[0] == '\t' ) )
			string++;
		if( !string[0] ) // empty line...
			continue;
		if( strncmp( string, WIDE("endmacro"), 8 ) == 0 )
		{
         string += 8;
         Log( WIDE("End macro...") );
			if( template->MacroRecording )
         {
            if( template->currentpage &&
                template->currentpage->DefinedIn == template->MacroRecording )
            {
               template->currentpage = NULL;
            }
            template->MacroRecording = template->MacroRecording->priorrecording;
			}
			else
			{
				// format error...
				Log( WIDE("Attempt to end a macro when not recording a macro...\n") );
			}
		}
		else
		{
			RunSingleLine( string, template );
		}
	}
	fclose( templatefile );
	return 1;
}

void DumpTemplate( PTEMPLATE template )
{
   PPRINTPAGE page;
   PPRINTMACRORUN macro;
   INDEX idx;
   //Log( WIDE("Running all macros...") );
   LIST_FORALL( template->MacroList, idx, PPRINTMACRORUN,  macro )
   {
      Log1( WIDE("Run macro: %s"), macro->macro->name );
   }

   for( page = template->pages; page; page = page->next )
   {
      Log( WIDE("Page...") );
      LIST_FORALL( page->MacroList, idx, PPRINTMACRORUN,  macro )
      {
         Log1( WIDE("Run macro: %s"), macro->macro->name );
      }

   }
}


int LoadTemplate( char *templatename, char *templatefilename )
{
	FILE *templatefile;
	PTEMPLATE template;
	if( !templatename || !templatefilename )
		return 0;
	template = Allocate( sizeof( TEMPLATE ) );
	MemSet( template, 0, sizeof( TEMPLATE ) );
	template->name = Allocate( (template->namelen = strlen( templatename )) + 1 );
	strcpy( template->name, templatename );
	template->next = templates;
	templates = template;
   templatefile = fopen( templatefilename, WIDE("rt") );

	if( templatefile )
	{
      DoLoadTemplate( templatefile, template );
      template->currentpage = NULL; // clear this cause running makes temp pages...
      DumpTemplate( template );
		return TRUE;
   }

	return FALSE;
}

