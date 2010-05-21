
#include <stdhdrs.h>

#define IMAGE_LIBRARY_SOURCE
#include <ft2build.h>
#ifdef FT_BEGIN_HEADER

#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <filesys.h>
#include <sharemem.h>
#include <timers.h>
//#include "global.h"
#include <imglib/fontstruct.h>

#include <image.h>
#include "fntglobal.h"
#include <controls.h>

IMAGE_NAMESPACE

#define SYMBIT(bit)  ( 1 << (bit&0x7) )

FILE *output;

//-------------------------------------------------------------------------

#define fg (*global_font_data)
PRIORITY_PRELOAD( CreateFontRenderGlobal, IMAGE_PRELOAD_PRIORITY )
{
	SimpleRegisterAndCreateGlobal( global_font_data );

	if( !fg.library )
	{
		int error = FT_Init_FreeType( &fg.library );
		if( error )
		{
			Log1( WIDE("Free type init failed: %d"), error );
			return ;
		}
	}
}

//-------------------------------------------------------------------------

void PrintLeadin( int bits )
{
	if( bits == 2)
		fprintf( output, WIDE("#define BITS_GREY2\n") );
   if( bits != 8 )
		fprintf( output, WIDE("#include \"symbits.h\"\n") );
	fprintf( output, WIDE("typedef char CHARACTER, *PCHARACTER;\n") );
}

int PrintChar( int bits, int charnum, PCHARACTER character, int height )
{
	int  outwidth;
	TEXTCHAR charid[64];
	char *data = (char*)character->data;
	snprintf( charid, sizeof( charid ), WIDE("_char_%d"), charnum );

	#define LINEPAD WIDE("                  ")
	if( bits == 8 )
		outwidth = character->size; // round up to next byte increments size.
	else if( bits == 2 )
		outwidth = ((character->size+3) & 0xFC ); // round up to next byte increments size.
	else
		outwidth = ((character->size+7) & 0xF8 ); // round up to next byte increments size.

	if( ((outwidth)/(8/bits))*( ( character->ascent - character->descent ) + 1 ))
		fprintf( output, WIDE("static struct{ char s, w, o, j, a, d; unsigned char data[%d]; } %s =\n"), 
						((outwidth)/(8/bits))*( ( character->ascent - character->descent ) + 1 )
						, charid );    
	else
		fprintf( output, WIDE("static struct{ char s, w, o, j, a, d; } %s =\n")
						, charid );

	fprintf( output, WIDE("{ %2d, %2d, %2d, 0, %2d, %2d")
							, character->size
							, character->width
							, (signed)character->offset
							, character->ascent
							, character->descent
							);

	if( character->size )
		fprintf( output, WIDE(", { \n") LINEPAD );

	{
	   	int line, bit;
	   	char *dataline;
		data = (char*)character->data;
		for(line = character->ascent;
			 line >= character->descent;
			 line-- )
		{
			if( line != character->ascent )
			{
				if( !line )
					fprintf( output, WIDE(", // <---- baseline\n") LINEPAD );
				else
					fprintf( output, WIDE(",\n") LINEPAD );
			}
			dataline = data;
			if( bits == 1 )
			{
				for( bit = 0; bit < character->size; bit++ )
				{
					if( bit && ((bit % 8) == 0 ) )
						fprintf( output, WIDE(",") );
					if( dataline[bit >> 3] & SYMBIT(bit) )
					{
						fprintf( output, WIDE("X") );
					}
					else
						fprintf( output, WIDE("_") );
				}
			}
			else if( bits == 2)
			{
            /*
				for( bit = 0; bit < (character->size+(3))/(8/bits); bit++ )
				{
               fprintf( output, WIDE("%02x "), dataline[bit] );
					}
               */
				for( bit = 0; bit < character->size; bit++ )
				{
					if( bit && ((bit % 4) == 0 ) )
						fprintf( output, WIDE(",") );
               //fprintf( output, WIDE("%02x "), (dataline[bit >> 2] >> (2*(bit&0x3))) & 3 );
					switch( (dataline[bit >> 2] >> (2*(bit&0x3))) & 3 )
					{
					case 3:
						fprintf( output, WIDE("X") );
						break;
					case 2:
						fprintf( output, WIDE("O") );
                  break;
					case 1:
						fprintf( output, WIDE("o") );
                  break;
					case 0:
						fprintf( output, WIDE("_") );
                  break;
					}
				}
			}
			else if( bits == 8 )
			{
				for( bit = 0; bit < character->size; bit++ )
				{
					if( bit )
						fprintf( output, WIDE(",") );
					fprintf( output, WIDE("%3d"), dataline[bit] );
				}

			}

			// fill with trailing 0's
			if( bits < 8 )
			{
				for( ; bit < outwidth; bit++ )
					fprintf( output, WIDE("_") );
			}
			data += (character->size+(bits==8?0:bits==2?3:7))/(8/bits);
      }
   }

	if( character->size )
		fprintf( output, WIDE("} ") );
	fprintf( output, WIDE("};\n\n\n") );
	return 0;
}


void PrintFontTable( CTEXTSTR name, PFONT font )
{
	int idx;
	int i, maxwidth;
	idx = 0;
	maxwidth = 0;
	for( i = 0; i < 256; i++ )
	{
		if( font->character[i] )
			if( font->character[i]->width > maxwidth )
				maxwidth = font->character[i]->width;
	}
	fprintf( output, WIDE("struct { unsigned short height, baseline, chars; unsigned char flags, junk;\n")
				WIDE("         char *fontname;\n")
				WIDE("         PCHARACTER character[256]; } %s = { \n%d, %d, 256, %d, 0, \"%s\", {")
				, name
				, font->height 
			   , font->baseline
            , font->flags
				, name
				);

	for( i = 0; i < 256; i++ )
	{
		if( font->character[i] )
			fprintf( output, WIDE(" %c(PCHARACTER)&_char_%d\n"), (i)?',':' ', i );

	}
	fprintf( output, WIDE("\n} };") );
}

//-------------------------------------------------------------------------

void DumpFontFile( CTEXTSTR name, Font font_to_dump )
{
	PFONT font = (PFONT)font_to_dump;
	if( font )
	{
		output = sack_fopen( 0, name, WIDE("wt") );
		if( output )
		{
			PrintLeadin(  (font->flags&3)==1?2
							: (font->flags&3)==2?8
							: 1 );
			{
				int charid;
				for	( charid = 0; charid < 256; charid++ )
				{
					PCHARACTER character = font->character[charid];
					if( character )
						PrintChar( (font->flags&3) == FONT_FLAG_8BIT?8
									 :(font->flags&3) == FONT_FLAG_2BIT?2
									 :1
									,charid, character, font->height );
				}
			}
			PrintFontTable( font->name, font );
			sack_fclose( output );
		}
	}
	else
      Log( WIDE("No font to dump...") );
}


#define TOSCALED(n) ((n)<<6)
#define CEIL(n)  ((S_16)((((n) + 63 )&(-64)) >> 6))
#define FLOOR(n) ((S_16)(((n) & (-64)) >> 6))

#define BITMAP_MASK(buf,n)  (buf[((n)/8)] & (0x80U >> ( (n) & 7 )))

void RenderMonoChar( PFONT font
					, _32 idx
					, FT_Bitmap *bitmap
					, FT_Glyph_Metrics *metrics
               , FT_Int left_ofs
               , FT_Int width
					)
{
   /*
	Log2( WIDE("Character: %d(%c)"), idx, idx>32&&idx<127?idx:0 );
	//Log1( WIDE("Font Height %d"), font->height );
	//Log5( WIDE("Bitmap information: (%d by %d) %d %d %d")
	//	 , bitmap->width
	//	 , bitmap->rows
	//	 , bitmap->pitch
	//	 , bitmap->pixel_mode
	//	 , bitmap->palette
	//	 );
	Log2( WIDE("Metrics: height %d bearing Y %d"), metrics->height, metrics->horiBearingY );
	Log2( WIDE("Metrics: width %d bearing X %d"), metrics->width, metrics->horiBearingX );
	Log2( WIDE("Metrics: advance %d %d"), metrics->horiAdvance, metrics->vertAdvance );
   */
	if( bitmap->width > 1000 ||
	    bitmap->width < 0 ||
	    bitmap->rows > 1000 ||
	    bitmap->rows < 0 )
	{
		Log3( WIDE("Failing character %") _32fs WIDE(" rows: %d  width: %d"), idx, bitmap->width, bitmap->rows );
		font->character[idx] = NULL;
		return;
	}

	{
		PCHARACTER character = (PCHARACTER)Allocate( sizeof( CHARACTER ) +
												  ( bitmap->rows
													* ((bitmap->width+7)/8) ) );
		INDEX bit, line, linetop, linebottom;
		INDEX charleft, charright;
      INDEX lines;

      char *data, *chartop;
		font->character[idx] = character;
      if( !character )
      {
      	Log( WIDE("Failed to allocate character") );
      	Log2( WIDE("rows: %d width: %d"), bitmap->rows, bitmap->width );
      	return;
      }
		character->width = width /*CEIL(metrics->horiAdvance)*/;
		if( !bitmap->rows && !bitmap->width )
		{
			//Log( WIDE("No physical data for this...") );
			character->ascent = 0;
			character->descent = 0;
			character->offset = 0;
			character->size = 0;
			return;
		}

		if( CEIL( metrics->width ) != bitmap->width )
		{
			//Log4( WIDE("%d(%c) metric and bitmap width mismatch : %d %d")
			//		, idx, (idx>=32 && idx<=127)?idx:'.'
			//	 , CEIL( metrics->width ), bitmap->width );
			if( !CEIL( metrics->width ) && bitmap->width )
            metrics->width = TOSCALED( bitmap->width );
		}
		if( CEIL( metrics->height ) != bitmap->rows )
		{
			//lprintf( WIDE("%d(%c) metric and bitmap height mismatch : %d %d using height %d")
			//		, idx, (idx>=32 && idx<=127)?idx:'.'
			//	 , CEIL( metrics->height )
			//	 , bitmap->rows
			//	 , TOSCALED( bitmap->rows) );
			metrics->height = TOSCALED( bitmap->rows );
		}

		character->ascent = CEIL(metrics->horiBearingY);
      if( metrics->height )
			character->descent = -CEIL(metrics->height - metrics->horiBearingY) + 1;
		else
         character->descent = CEIL( metrics->horiBearingY ) - font->height + 1;
		character->offset = CEIL(metrics->horiBearingX);
		character->size = bitmap->width;
      //charleft = charleft;
      //charheight = CEIL( metrics->height );

		//Log7( WIDE("(%d(%c)) Character parameters: %d %d %d %d %d")
		//	 , idx, idx< 32 ? ' ':idx
		//	 , character->width
      //    , character->offset
		//	 , character->size
		//	 , character->ascent
		//	 , character->descent );

		// for all horizontal lines which are blank - decrement ascent...
      chartop = NULL;
		data = (char*)bitmap->buffer;

		linetop = 0;
		linebottom = character->ascent - character->descent;
		//Log2( WIDE("Linetop: %d linebottom: %d"), linetop, linebottom );

		lines = character->ascent - character->descent;
		// reduce ascent to character data minimum....
      /*
		for( line = linetop; line <= linebottom; line++ )
		{
			data = bitmap->buffer + (line) * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( BITMAP_MASK( data, bit ) )
					printf( WIDE("X") );
				else
					printf( WIDE("_") );
			}
         printf( WIDE("\n") );
		}
		*/
		for( line = linetop; line <= linebottom; line++ )
		{
			data = (char*)bitmap->buffer + line * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( BITMAP_MASK(data,bit) )
					break;
			}
			if( bit == character->size )
			{
            //Log( WIDE("Dropping a line...") );
            character->ascent--;
			}
			else
			{
            linetop = line;
				chartop = data;
				break;
			}
		}
		if( line == linebottom + 1 )
		{
			Release( font->character[idx] );
         font->character[idx] = NULL;
			//Log( WIDE("No character data usable...") );
         return;
		}

		for( line = linebottom; line > linetop; line-- )
		{
			data = (char*)bitmap->buffer + line * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( BITMAP_MASK(data,bit) )
					break;
			}
			if( bit == character->size )
			{
            //Log( WIDE("Dropping a line...") );
            character->descent++;
			}
			else
			{
				break;
			}
		}

      linebottom = linetop + character->ascent - character->descent;
		// find character left offset...
		charleft = character->size;
		for( line = linetop; line <= linebottom; line++ )
		{
			data = (char*)bitmap->buffer + (line) * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( BITMAP_MASK( data, bit ) )
				{
					if( bit < charleft )
						charleft = bit;
				}
			}
			if( bit < character->size )
				break;
		}
		if( charleft )
		{
			//Log3( WIDE("Reduced char(%c) size by %d to %d"), idx>32&&idx<127?idx:'.', charleft, character->size-charleft );
		}
		// if zero no change...

		// find character right extent
		charright = 0;
		for( line = linetop; line <= linebottom; line++ )
		{
			data = (char*)bitmap->buffer + (line) * bitmap->pitch;
			for( bit = charleft; bit < character->size; bit++ )
			{
				if( BITMAP_MASK( data, bit ) )
					if( bit > charright )
						charright = bit;
			}
		}
		charright ++; // add one back in... so we have a delta between left and right
		//Log2( WIDE("Reduced char right size %d to %d"), charright, charright - charleft );

		character->offset = charleft;
		character->size = charright - charleft;

		//Log7( WIDE("(%d(%c)) Character parameters: %d %d %d %d %d")
		//	 , idx, idx< 32 ? ' ':idx
		//	 , character->width
      //    , character->offset
		//	 , character->size
		//	 , character->ascent
		//	 , character->descent );
		{
			unsigned char *outdata;
			// now - to copy the pixels...
			for( line = linetop; line <= linebottom; line++ )
			{
				outdata = character->data +
					((line - linetop) * ((character->size + 7)/8));
				data = (char*)bitmap->buffer + (line) * bitmap->pitch;
            for( bit = 0; bit < ((character->size + 7)/8); bit++)
					outdata[bit] = 0;
				for( bit = 0; bit < character->size; bit++ )
				{
					if( BITMAP_MASK( data, bit + character->offset ) )
					{
						outdata[(bit)>>3] |= 0x01 << (bit&7);
					}
				}
			}
		}
		character->offset += left_ofs;
	}
}


#define GREY_MASK(buf,n)  ((bits==2)?(\
	(((buf[n]&0xE0)?2:0)|   \
	 ((buf[n]&0x3F)?1:0))       \
	):(buf[n]))

void RenderGreyChar( PFONT font
						 , _32 idx
						 , FT_Bitmap *bitmap
						 , FT_Glyph_Metrics *metrics
						 , FT_Int left_ofs
						 , FT_Int width
						 , _32 bits // 2 or 8
						 )
{
   //lprintf( WIDE("Rending char %d bits %d"), idx, bits );
	/*
	 Log2( WIDE("Character: %d(%c)"), idx, idx>32&&idx<127?idx:0 );
	 //Log1( WIDE("Font Height %d"), font->height );
	 //Log5( WIDE("Bitmap information: (%d by %d) %d %d %d")
	 //	 , bitmap->width
	//	 , bitmap->rows
	//	 , bitmap->pitch
	//	 , bitmap->pixel_mode
	//	 , bitmap->palette
	//	 );
	Log2( WIDE("Metrics: height %d bearing Y %d"), metrics->height, metrics->horiBearingY );
	Log2( WIDE("Metrics: width %d bearing X %d"), metrics->width, metrics->horiBearingX );
	Log2( WIDE("Metrics: advance %d %d"), metrics->horiAdvance, metrics->vertAdvance );
   */
	if( bitmap->width > 1000 ||
	    bitmap->width < 0 ||
	    bitmap->rows > 1000 ||
	    bitmap->rows < 0 )
	{
		Log3( WIDE("Failing character %") _32fs WIDE(" rows: %d  width: %d"), idx, bitmap->width, bitmap->rows );
		font->character[idx] = NULL;
		return;
	}

	{
		PCHARACTER character = (PCHARACTER)Allocate( sizeof( CHARACTER )
												  + ( bitmap->rows
													  * (bitmap->width+(bits==8?0:bits==2?3:7)/(8/bits)) ) );
		INDEX bit, line, linetop, linebottom;
		INDEX charleft, charright;
      INDEX lines;

      char *data, *chartop;
		font->character[idx] = character;
      if( !character )
      {
      	Log( WIDE("Failed to allocate character") );
			Log2( WIDE("rows: %d width: %d"), bitmap->rows, bitmap->width );
      	return;
      }
		character->width = width /*CEIL(metrics->horiAdvance)*/;
		if( !bitmap->rows && !bitmap->width )
		{
			//Log( WIDE("No physical data for this...") );
			character->ascent = 0;
			character->descent = 0;
			character->offset = 0;
			character->size = 0;
			return;
		}

		if( CEIL( metrics->width ) != bitmap->width )
		{
			Log4( WIDE("%") _32fs WIDE("(%c) metric and bitmap width mismatch : %d %d")
					, idx, (int)((idx>=32 && idx<=127)?idx:'.')
					, CEIL( metrics->width ), bitmap->width );
		}
		if( CEIL( metrics->height ) != bitmap->rows )
		{
			Log4( WIDE("%") _32fs WIDE("(%c) metric and bitmap height mismatch : %d %d")
					, idx, (int)((idx>=32 && idx<=127)?idx:'.')
					, CEIL( metrics->height ), bitmap->rows );
			metrics->height = TOSCALED( bitmap->rows );
		}

		character->ascent = CEIL(metrics->horiBearingY);
      if( metrics->height )
			character->descent = -CEIL(metrics->height - metrics->horiBearingY) + 1;
		else
         character->descent = CEIL( metrics->horiBearingY ) - font->height + 1;
		character->offset = CEIL(metrics->horiBearingX);
		character->size = bitmap->width;
      //charleft = charleft;

		// for all horizontal lines which are blank - decrement ascent...
      chartop = NULL;
		data = (char*)bitmap->buffer;

		linetop = 0;
		linebottom = character->ascent - character->descent;
		//Log2( WIDE("Linetop: %d linebottom: %d"), linetop, linebottom );

		lines = character->ascent - character->descent;
		// reduce ascent to character data minimum....
		for( line = linetop; line <= linebottom; line++ )
		{
			data = (char*)bitmap->buffer + line * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( GREY_MASK(data,bit) )
					break;
			}
			if( bit == character->size )
			{
            //Log( WIDE("Dropping a line...") );
            character->ascent--;
			}
			else
			{
            linetop = line;
				chartop = data;
				break;
			}
		}
		if( line == linebottom + 1 )
		{
         character->size = 0;
			//Log( WIDE("No character data usable...") );
         return;
		}

		for( line = linebottom; line > linetop; line-- )
		{
			data = (char*)bitmap->buffer + line * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( GREY_MASK(data,bit) )
					break;
			}
			if( bit == character->size )
			{
            //Log( WIDE("Dropping a line...") );
            character->descent++;
			}
			else
			{
				break;
			}
		}

      linebottom = linetop + character->ascent - character->descent;
		// find character left offset...
		charleft = character->size;
		for( line = linetop; line <= linebottom; line++ )
		{
			data = (char*)bitmap->buffer + (line) * bitmap->pitch;
			for( bit = 0; bit < character->size; bit++ )
			{
				if( GREY_MASK( data, bit ) )
				{
					if( bit < charleft )
						charleft = bit;
				}
			}
			if( bit < character->size )
				break;
		}

		// find character right extent
		charright = 0;
		for( line = linetop; line <= linebottom; line++ )
		{
			data = (char*)bitmap->buffer + (line) * bitmap->pitch;
			for( bit = charleft; bit < character->size; bit++ )
			{
				if( GREY_MASK( data, bit ) )
					if( bit > charright )
						charright = bit;
			}
		}
		charright ++; // add one back in... so we have a delta between left and right
		//Log2( WIDE("Reduced char right size %d to %d"), charright, charright - charleft );

		character->offset = charleft;
		character->size = charright - charleft;

		{
			unsigned char *outdata;
			// now - to copy the pixels...
			for( line = linetop; line <= linebottom; line++ )
			{
				outdata = character->data +
					((line - linetop) * ((character->size + (bits==8?0:bits==2?3:7))/(8/bits)));
				data = (char*)bitmap->buffer + (line) * bitmap->pitch;
            for( bit = 0; bit < ((character->size + (bits==8?0:bits==2?3:7))/(8/bits)); bit++)
					outdata[bit] = 0;
				for( bit = 0; bit < (character->size); bit++ )
				{
					int grey = GREY_MASK( data, bit + character->offset );
               //printf( WIDE("(%02x)"), grey );
					if( grey )
					{
						if( bits == 2 )
						{
							outdata[bit>>2] |= grey << ((bit % 4) * 2);
							//printf( WIDE("%02x "), outdata[bit>>2] );
                     /*
							switch( grey )
							{
							case 3: printf( WIDE("X") ); break;
							case 2: printf( WIDE("O") ); break;
							case 1: printf( WIDE("o") ); break;
							case 0: printf( WIDE("_") ); break;
							default: printf( WIDE("%02X"), grey );
							}
                     */
						}
						else
                     outdata[bit] = grey;
					}
					//else
               //   printf( WIDE("_") );
				}
            //printf( WIDE("\n") );
			}
		}
		character->offset += left_ofs;
	}
}


static void KillSpaces( TEXTCHAR *string )
{
	int ofs = 0;
	if( !string )
		return;
	while( *string )
	{
		string[ofs] = string[0];
		if( *string == ' ' )
			ofs--;
		string++;
	}
	string[ofs] = 0;
}

extern int InitFont( void );

// everything eventually comes to here to render a font...
// this should therefore cache this information with the fonts
// it has created...

struct font_renderer_tag {
	TEXTCHAR *file;
	S_16 nWidth;
	S_16 nHeight;
	_32 flags; // whether to render 1(0/1), 2(2), 8(3) bits, ...
	POINTER ResultData; // data that was resulted for creating this font.
	_32 ResultDataLen;
	POINTER font_memory;
	FT_Face face;
	FT_Glyph glyph;
	FT_GlyphSlot slot;
	Font ResultFont;
	PFONT font;
	TEXTCHAR fontname[256];
} ;
typedef struct font_renderer_tag FONT_RENDERER;

static PLIST fonts;
CRITICALSECTION cs;

void InternalRenderFontCharacter( PFONT_RENDERER renderer, PFONT font, INDEX idx )
{
	if( font && font->character[idx] )
      return;
	if( !renderer )
	{
		INDEX idx;
		if( font )
			LIST_FORALL( fonts, idx, PFONT_RENDERER, renderer )
		   {
				if( renderer->font == font )
					break;
			}
		if( !renderer )
		{
			lprintf( WIDE("Couldn't find by font either...") );
         DebugBreak();
			return;
		}
	}
	EnterCriticalSec( &cs );
   if( !renderer->font->character[idx] )
	{
      FT_Face face = renderer->face;
		int glyph_index = FT_Get_Char_Index( face, idx );
		//Log2( WIDE("Character %d is glyph %d"), idx, glyph_index );
		FT_Load_Glyph( face
						 , glyph_index
						 , 0
						  | FT_LOAD_FORCE_AUTOHINT
							 );
			if( ( face->glyph->linearVertAdvance>>16 ) > renderer->font->height )
				renderer->font->height = (short)(face->glyph->linearVertAdvance>>16);
			switch( face->glyph->format )
			{
			case FT_GLYPH_FORMAT_OUTLINE:
				//Log( WIDE("Outline render glyph....") );
            if( (( renderer->flags & 3 )<2) )
					FT_Render_Glyph( face->glyph, FT_RENDER_MODE_MONO );
            else
					FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
#ifdef ft_glyph_format_bitmap
			case FT_GLYPH_FORMAT_BITMAP:
#else
			case ft_glyph_format_bitmap:
#endif
            /*
				Log2( WIDE("bitmap params: left %d top %d"), face->glyph->bitmap_left, face->glyph->bitmap_top );
				Log6( WIDE("Glyph Params: %d(%d) %d(%d) %d %d")
					 , face->glyph->linearHoriAdvance
					 , face->glyph->linearHoriAdvance>>16
					 , face->glyph->linearVertAdvance
					 , face->glyph->linearVertAdvance>>16
                 , face->glyph->advance.x
					 , face->glyph->advance.y );
					 Log3( WIDE("bitmap height: %d ascent: %d descent: %d"), face->height>>6, face->ascender>>6, face->descender>>6);
                */
				//Log( WIDE("Bitmap") );
				if( ( renderer->flags & 3 )<2 )
				{
					RenderMonoChar( renderer->font
								 , idx
								 , &face->glyph->bitmap
								 , &face->glyph->metrics
								 , face->glyph->bitmap_left // left offset?
								 , face->glyph->linearHoriAdvance>>16
									  );
				}
				else
				{
					RenderGreyChar( renderer->font
									  , idx
									  , &face->glyph->bitmap
									  , &face->glyph->metrics
									  , face->glyph->bitmap_left // left offset?
									  , face->glyph->linearHoriAdvance>>16
									  , ((renderer->flags & 3)==3)?8:2 );
				}
				break;
			//case FT_GLYPH_FORMAT_COMPOSITE:
			//	Log( WIDE("unsupported Composit") );
			//	break;
			default:
            renderer->font->character[idx] = NULL;
				Log1( WIDE("unsupported Unknown format(%4.4s)"), (char*)&face->glyph->format );
				break;
			}
		}
		for( idx = 0; idx < 256; idx++ )
			if( renderer->font->character[idx] )
				break;
		if( idx == 256 )
		{
         renderer->ResultFont = NULL;
		}
		else
		{
         /*
			//font->height = max_ascent - min_descent + 1;
			renderer->font->baseline = max_ascent +
				( ( renderer->font->height
					- ( max_ascent - min_descent ) )
					/ 2 );
               */
			//Log1( WIDE("baseline is now %d"), font->baseline );
			renderer->ResultFont = LoadFont( (Font)renderer->font );
		}
	LeaveCriticalSec( &cs );
	}


Font InternalRenderFontFile( CTEXTSTR file
										, S_16 nWidth
										, S_16 nHeight
										, _32 flags // whether to render 1(0/1), 2(2), 8(3) bits, ...
										)
{
	int error;
	int bDefaultFile = 0;
   PFONT_RENDERER renderer;
	//static CRITICALSECTION cs;
   EnterCriticalSec( &cs );
	//if( !InitFont() )
	{
      //lprintf( WIDE("Failed to init fonts.. failing font render.") );
      //LeaveCriticalSec( &cs );
	//	return NULL;
	}

	// with the current freetype library which has been grabbed
   // width and height more than these cause errors.
	if( nWidth > 227 ) nWidth = 227;
	if( nHeight > 227 ) nHeight = 227;

try_another_default:	
	if( !file || bDefaultFile )
	{
		switch( bDefaultFile )
		{
		case 0:  	
			file = WIDE("arialbd.ttf"); 	  	
			break;
		case 1:
			file = WIDE("fonts/arialbd.ttf");
			break;
		case 2:
		    lprintf( WIDE("default font arialbd.ttf or fonts/arialbd.ttf did not load.") );
			LeaveCriticalSec( &cs );
			return NULL;
		}
		bDefaultFile++;
	}               	
	{
		INDEX idx;
		LIST_FORALL( fonts, idx, PFONT_RENDERER, renderer )
		{
			if( renderer->nWidth == nWidth
				&& renderer->nHeight == nHeight
				&& renderer->flags == flags
				&& strcmp( renderer->file, file ) == 0 )
			{
            break;
			}
		}
		if( !renderer )
		{
			renderer = (PFONT_RENDERER)Allocate( sizeof( FONT_RENDERER ) );
			MemSet( renderer, 0, sizeof( FONT_RENDERER ) );
			renderer->nWidth = nWidth;
			renderer->nHeight = nHeight;
			renderer->flags = flags;
			renderer->file = sack_prepend_path( 0, file );
			AddLink( &fonts, renderer );
		}
		else
		{
			lprintf( WIDE("using existing renderer for this font...") );
		}
	}

	// this needs to be post processed to
	// center all characters in a font - favoring the
	// bottom...

	if( !renderer->face )
	{
		PTRSZVAL size = 0;

		renderer->font_memory = 
#ifdef UNDER_CE
			NULL;
#else
			OpenSpace( NULL, file, &size );
#endif

		if( renderer->font_memory )
		{
			POINTER p = Allocate( size );
			MemCpy( p, renderer->font_memory, size );
			Release( renderer->font_memory );
			renderer->font_memory = p;

			//lprintf( WIDE("Using memory mapped space...") );
			error = FT_New_Memory_Face( fg.library
											  , (FT_Byte*)renderer->font_memory
											  , size
											  , 0
											  , &renderer->face );
			//if( renderer->font_memory )
			//	Release( renderer->font_memory );
		}
		else
		{
#ifdef __cplusplus_cli
			char *file = CStrDup( renderer->file );
#else
			CTEXTSTR file = renderer->file;
#endif
			lprintf( WIDE("Using file access font...") );
			error = FT_New_Face( fg.library
									 , file
									 , 0
									 , &renderer->face );
         lprintf( "Result %d", error );
#ifdef __cplusplus_cli
			Release( file );
#endif
		}
	}
	else
	{
      error = 0;
	}

	if( !renderer->face || error )
	{
      //DebugBreak();
		//lprintf( WIDE("Failed to load font...Err:%d %s %d %d"), error, file, nWidth, nHeight );
		DeleteLink( &fonts, renderer );
		Release( renderer->file );
      Release( renderer );
      if( bDefaultFile )
      	goto try_another_default;
		LeaveCriticalSec( &cs );
		return NULL;
	}

   if( !renderer->fontname[0] )
	{
      FT_Face face = renderer->face;
		snprintf( renderer->fontname, 256, WIDE("%s%s%s%dBy%d")
				  , face->family_name?face->family_name:"No-Name"
				  , face->style_name?face->style_name:"normal"
				  , (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH)?"fixed":""
				  , nWidth, nHeight
				  );
		KillSpaces( renderer->fontname );
	}
	// this may not be 'wide' enough...
	// there may be characters which overflow the
	// top/bottomm of the cell... probably will
	// ignore that - but if we DO find such a font,
	// maybe this can be adjusted -- NORMALLY
	// this will be sufficient...
   if( !renderer->font )
	{
		int max_ascent = 0;
      int min_descent =0;
      INDEX idx;
		PFONT font;
		renderer->font = (PFONT)Allocate( sizeof( FONT ) + 255 * sizeof(PCHARACTER) );
		// this is okay it's a kludge so some other code worked magically
      // it's redundant and should be dleete...
      renderer->ResultFont = (Font)renderer->font;
		font = (PFONT)renderer->ResultFont;
      MemSet( font, 0, sizeof( FONT )+ 255 * sizeof(PCHARACTER) );
		font->characters = 256;
		font->baseline = 0;
		if( ( flags & 3 ) == 3 )
			font->flags = FONT_FLAG_8BIT;
		else if( ( flags & 3 ) == 2 )
			font->flags = FONT_FLAG_2BIT;
		else
			font->flags = FONT_FLAG_MONO;
		// default rendering for one-to-one pixel sizes.
		/*
		 error = FT_Set_Char_Size( face
		 , nWidth << 6
		 , nHeight << 6
		 , 96
		 , 96 );
		 */
		error = FT_Set_Pixel_Sizes( renderer->face
										  , renderer->nWidth
										  , renderer->nHeight );
		if( error )
		{
			Log1( WIDE("Fault setting char size - %d"), error );
		}
		font->height = 0; //CEIL(face->size->metrics.height);
		font->name = StrDup( renderer->fontname );
		InternalRenderFontCharacter( renderer, NULL, 0 );
		for( idx = 0; idx < 256; idx++ )
		{
			FT_Face face = renderer->face;
			int glyph_index = FT_Get_Char_Index( face, idx );
			//Log2( WIDE("Character %d is glyph %d"), idx, glyph_index );
			FT_Load_Glyph( face
							 , glyph_index
							 , 0
							  | FT_LOAD_FORCE_AUTOHINT
							 );
			{
				int ascent = CEIL(face->glyph->metrics.horiBearingY);
				int descent;
				if( face->glyph->metrics.height )
				{
					descent = -CEIL(face->glyph->metrics.height - face->glyph->metrics.horiBearingY) + 1;
				}
				else
				{
					descent = CEIL( face->glyph->metrics.horiBearingY ) /*- font->height + */ + 1;
				}

				// done when the font is initially loaded
				// loading face characteristics shouldn't matter
				if( ascent > max_ascent )
					max_ascent = ascent;
				if( descent < min_descent )
					min_descent = descent;
			}
		}
		renderer->font->baseline = max_ascent +
			( ( renderer->font->height
				- ( max_ascent - min_descent ) )
			 / 2 );
	}
	//FT_Done_Face( face );
   //if( renderer->font_memory )
	//	Release( renderer->font_memory );

	// just for grins - check all characters defined for font
	// to see if their ascent - descent is less than height
	// and that ascent is less than baseline.
	// however, if we have used a LoadFont function
   // which has replaced the font thing with a handle...
	//DumpFontFile( WIDE("font.h"), font );
   LeaveCriticalSec( &cs );
   return renderer->ResultFont;;
}

int RecheckCache( CTEXTSTR entry, _32 *pe
					 , CTEXTSTR style, _32 *ps
					 , CTEXTSTR filename, _32 *psf );

Font RenderScaledFontData( PFONTDATA pfd, PFRACTION width_scale, PFRACTION height_scale )
{
#define fname(base) base
#define sname(base) fname(base) + (strlen(base)+1)
#define fsname(base) sname(base) + strlen(sname(base)+1)
	extern _64 fontcachetime;
	if( !pfd )
      return NULL;
	LoadAllFonts(); // load cache data so we can resolve user data
	if( pfd->magic == MAGIC_PICK_FONT )
	{
      CTEXTSTR name1 = pfd->names;
      CTEXTSTR name2 = pfd->names + strlen( pfd->names ) + 1;
      CTEXTSTR name3 = name2 + strlen( name2 ) + 1;
		if( pfd->cachefile_time == fontcachetime ||
			RecheckCache( name1/*fname(pfd->names)*/, &pfd->nFamily
							, name2/*sname(pfd->names)*/, &pfd->nStyle
							, name3/*fsname(pfd->names)*/, &pfd->nFile ) )
		{
			Font return_font = InternalRenderFont( pfd->nFamily
															 , pfd->nStyle
															 , pfd->nFile
															 , width_scale?ScaleValue(width_scale,pfd->nWidth):pfd->nWidth
															 , height_scale?ScaleValue(height_scale,pfd->nHeight):pfd->nHeight
															 , pfd->flags );
			pfd->cachefile_time = fontcachetime;
			return return_font;
		}
	}
	else if( pfd->magic == MAGIC_RENDER_FONT )
	{
		PRENDER_FONTDATA prfd = (PRENDER_FONTDATA)pfd;
		return InternalRenderFontFile( prfd->filename
											  , width_scale?ScaleValue(width_scale,prfd->nWidth):prfd->nWidth
											  , height_scale?ScaleValue(height_scale,prfd->nHeight):prfd->nHeight
											  , prfd->flags );
	}
	lprintf( WIDE("Font parameters are no longer valid.... could not be found in cache issue font dialog here?") );
   return NULL;
}

Font InternalRenderFont( _32 nFamily
								  , _32 nStyle
								  , _32 nFile
								  , S_16 nWidth
								  , S_16 nHeight
                          , _32 flags
								  )
{
	PFONT_ENTRY pfe = fg.pFontCache + nFamily;
	TEXTCHAR name[256];
	POINTER result;
	INDEX nAlt = 0;
	//if( !InitFont() )
	//	return NULL;
	snprintf( name, sizeof( name ), WIDE("%s/%s")
			  , pfe->styles[nStyle].files[nFile].path
			 , pfe->styles[nStyle].files[nFile].file );

	Log1( WIDE("Font: %s"), name );
	do
	{
		if( nAlt )
		{
			snprintf( name, sizeof( name ), WIDE("%s/%s")
					  , pfe->styles[nStyle].files[nFile].pAlternate[nAlt-1].path
					  , pfe->styles[nStyle].files[nFile].pAlternate[nAlt-1].file );
		}
		// this is called from pickfont, and we dont' have font space to pass here...
		// it's taken from dialog parameters...
		result = InternalRenderFontFile( name, nWidth, nHeight, flags );
		// this may not be 'wide' enough...
		// there may be characters which overflow the
		// top/bottomm of the cell... probably will
		// ignore that - but if we DO find such a font,
		// maybe this can be adjusted -- NORMALLY
      // this will be sufficient...
	} while( (nAlt++ < pfe->styles[nStyle].files[nFile].nAlt) && !result );

	return (Font)result;
}

void DestroyFont( Font *font )
{
	if( font )
	{
		if( *font )
		{
   		xlprintf( LOG_ALWAYS )( WIDE("font destruction is NOT implemented, this WILL result in a memory leak!!!!!!!") );
			(*font) = NULL;
		}
	}

   // need to find this resource...
	//if( renderer->font_memory )
	//	Release( renderer->font_memory );

// release resources of this font...
   // (also in the image library which may be a network service thing)
}

//-------------------------------------------------------------------------

int RecheckCache( CTEXTSTR entry, _32 *pe
					 , CTEXTSTR style, _32 *ps
					 , CTEXTSTR filename, _32 *psf )
{
	INDEX n;
	for( n = 0; n < fg.nFonts; n++ )
	{
		if( strcmp( fg.pFontCache[n].name, entry ) == 0 )
		{
			INDEX s;
			for( s = 0; s < fg.pFontCache[n].nStyles; s++ )
			{
				if( strcmp( fg.pFontCache[n].styles[s].name, style ) == 0 )
				{
					INDEX sf;
					for( sf = 0; sf < fg.pFontCache[n].styles[s].nFiles; sf++ )
					{
						CTEXTSTR file = pathrchr( filename );
						if( file )
							file++;
						else
							file = filename;
						if( strcmp( fg.pFontCache[n].styles[s].files[sf].file, file ) == 0 )
						{
							(*pe) = n;
							(*ps) = s;
							(*psf) = sf;
							return TRUE;
						}
					}
				}
			}
		}
	}
   return FALSE;
}

void SetFontRendererData( Font font, POINTER pResult, _32 size )
{
	PFONT_RENDERER renderer;
	INDEX idx;
	LIST_FORALL( fonts, idx, PFONT_RENDERER, renderer )
	{
		if( renderer->ResultFont == font )
			break;
	}
	if( renderer )
	{
      Hold( pResult ); // make sure we hang onto this... I think applications are going to release it.
		renderer->ResultData = pResult;
		renderer->ResultDataLen = size;
	}
}

Font RenderFontFileEx( CTEXTSTR file, _32 width, _32 height, _32 flags, P_32 size, POINTER *pFontData )
{
	Font font = InternalRenderFontFile( file, (S_16)(width&0x7FFF), (S_16)(height&0x7FFF), flags );
	if( font && size && pFontData )
	{
      int chars;
		PRENDER_FONTDATA pResult = (PRENDER_FONTDATA)Allocate( (*size)
														= sizeof( RENDER_FONTDATA )
														+ (chars = strlen( file ) + 1)*sizeof(TEXTCHAR) );
		pResult->magic = MAGIC_RENDER_FONT;
		pResult->nHeight = height;
		pResult->nWidth = width;
		pResult->flags = flags;
		StrCpyEx( pResult->filename, file, chars );
		(*pFontData) = (POINTER)pResult;
		SetFontRendererData( font, pResult, (*size) );
	}
	return font;
}

#undef RenderFontFile
Font RenderFontFile( CTEXTSTR file, _32 width, _32 height, _32 flags )
{
   return InternalRenderFontFile( file, width, height, flags );
}

int GetFontRenderData( Font font, POINTER *fontdata, _32 *fontdatalen )
{
	// set pointer and _32 datalen passed by address
	// with the font descriptor block that was used to
	// create the font (there's always something like that)
	PFONT_RENDERER renderer = NULL;
	INDEX idx;
	if( font )
	{
		LIST_FORALL( fonts, idx, PFONT_RENDERER, renderer )
		{
			if( renderer->font == (PFONT)font )
				break;
		}
	}
	if( renderer )
	{
		(*fontdata) = renderer->ResultData;
		(*fontdatalen) = renderer->ResultDataLen;
		return TRUE;
	}
	return FALSE;
}

#endif

#ifdef __cplusplus 
}//namespace sack {
}//namespace image {
#endif

// $Log: fntrender.c,v $
// Revision 1.27  2005/07/27 22:16:56  jim
// Untested openfontfilememory...
//
// Revision 1.26  2005/07/27 21:57:44  jim
// Untested openfontfilememory...
//
// Revision 1.25  2005/07/25 17:49:01  jim
// Use a different load font function so we don't blow up on duplicate font file accesses.
//
// Revision 1.22  2005/05/25 16:50:27  d3x0r
// Synch with working repository.
//
// Revision 1.24  2005/05/18 23:45:43  jim
// Fix where the font name is set...
//
// Revision 1.23  2005/03/21 20:41:35  panther
// Protect against super large fonts, remove edit frame from palette, and clean up some warnings.
//
// Revision 1.22  2004/12/15 03:00:19  panther
// Begin coding to only show valid, renderable fonts in dialog, and update cache, turns out that we'll have to postprocess the cache to remove unused dictionary entries
//
// Revision 1.21  2004/10/24 20:09:47  d3x0r
// Sync to psilib2... stable enough to call it mainstream.
//
// Revision 1.2  2004/10/08 13:07:43  d3x0r
// Okay beginning to look a lot like PRO-GRESS
//
// Revision 1.1  2004/09/19 19:22:31  d3x0r
// Begin version 2 psilib...
//
// Revision 1.20  2004/06/21 07:47:38  d3x0r
// Checkpoint - make system rouding out nicly.
//
// Revision 1.19  2004/03/29 20:08:09  d3x0r
// remove printf'd debugging
//
// Revision 1.18  2004/03/23 17:02:44  d3x0r
// Use common interface library to load video/image interface
//
// Revision 1.17  2003/11/29 00:10:28  panther
// Minor fixes for typecast equation
//
// Revision 1.16  2003/10/07 20:29:49  panther
// Modify render to accept flags, test program to test bits.  Generate multi-bit alpha
//
// Revision 1.15  2003/10/07 02:12:50  panther
// Ug - it's all terribly broken
//
// Revision 1.14  2003/10/07 02:01:09  panther
// 8 bit alpha fonts....
//
// Revision 1.13  2003/10/07 00:37:34  panther
// Prior commit in error - Begin render fonts in multi-alpha.
//
// Revision 1.12  2003/10/07 00:32:08  panther
// Fix default font.  Add bit size flag to font
//
// Revision 1.11  2003/09/26 13:48:39  panther
// Fix export font function
//
// Revision 1.10  2003/09/26 11:21:31  panther
// Export dump font file function to take a font and write it as a C/H file
//
// Revision 1.9  2003/09/25 09:47:15  panther
// Clean up for LCC compilation
//
// Revision 1.8  2003/08/27 07:58:39  panther
// Lots of fixes from testing null pointers in listbox, font generation exception protection
//
// Revision 1.7  2003/06/16 10:17:42  panther
// Export nearly usable renderfont routine... filename, size
//
// Revision 1.6  2003/03/25 08:45:56  panther
// Added CVS logging tag
//
