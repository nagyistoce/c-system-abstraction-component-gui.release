
#ifndef STRUCT_ONLY
#include <ft2build.h>
#ifdef FT_FREETYPE_H
#include FT_FREETYPE_H
#endif
#endif

#include <deadstart.h>
//#ifndef NO_FONT_GLOBAL_DECLARATION
#define fg (*global_font_data)
//#else
//#define fg (*GetGlobalFonts())
//#endif
IMAGE_NAMESPACE

#ifndef FONT_CACHE_STRUCTS

typedef struct alt_size_file_tag
{
   TEXTCHAR *path;
	struct alt_size_file_tag *pAlternate;
   TEXTCHAR file[1];
} ALT_SIZE_FILE, *PALT_SIZE_FILE;

typedef struct size_tag
{
	struct {
		_32 unusable : 1;
	} flags;
	S_16 width;
	S_16 height;
   struct size_tag *next;
} SIZES, *PSIZES;

typedef struct file_size_tag
{
	struct {
		_32 unusable : 1;
	} flags;
	TEXTCHAR *path;
	TEXTCHAR *file;
   _32   nAlt;
	PALT_SIZE_FILE pAlternate;
   _32   nSizes;
   PSIZES sizes; // scaled and fixed sizes available in this file.
} SIZE_FILE, *PSIZE_FILE;

typedef struct font_style_t
{
	struct {
		_32 mono : 1;
		_32 unusable : 1;
	} flags;
	TEXTCHAR        *name;
   _32          nFiles;
   PSIZE_FILE   files;
} FONT_STYLE, *PFONT_STYLE;

typedef struct font_entry_tag
{
	struct {
		_32 unusable : 1;
	} flags;
	TEXTCHAR   *name;
   _32          nStyles;
   PFONT_STYLE  styles; // array of nStyles
} FONT_ENTRY, *PFONT_ENTRY;

#endif

#if !defined( NO_FONT_GLOBAL )
typedef struct font_global_tag
{
#if defined( STRUCT_ONLY )
	PTRSZVAL library;
#else
	FT_Library   library;
#endif
	_32          nFonts;
	FONT_ENTRY  *pFontCache;
} FONT_GLOBAL;
#endif

enum {
 MAGIC_PICK_FONT = 'PICK',
 MAGIC_RENDER_FONT = 'FONT'
};
struct font_data_tag {
   _32 magic;
	_32 nFamily;
	_32 nStyle;
	_32 nFile;
	_32 nWidth;
	_32 nHeight;
   _32 flags;
	_64 cachefile_time;
   TEXTCHAR names[];
};
// defines FONTDATA for internal usage.
typedef struct font_data_tag  FONTDATA;

typedef struct render_font_data_tag {
   _32 magic;
	_32 nWidth;
	_32 nHeight;
   _32 flags;
	TEXTCHAR filename[];
} RENDER_FONTDATA;

/* internal function to load fonts */
void LoadAllFonts( void );
/* internal function to unload fonts */
void UnloadAllFonts( void );


#if !defined( NO_FONT_GLOBAL )
static FONT_GLOBAL *global_font_data;
//#define fg (*global_font_data)
#endif
IMAGE_NAMESPACE_END

// $Log: fntglobal.h,v $
// Revision 1.8  2004/12/15 03:00:19  panther
// Begin coding to only show valid, renderable fonts in dialog, and update cache, turns out that we'll have to postprocess the cache to remove unused dictionary entries
//
// Revision 1.7  2004/10/24 20:09:47  d3x0r
// Sync to psilib2... stable enough to call it mainstream.
//
// Revision 1.1  2004/09/19 19:22:31  d3x0r
// Begin version 2 psilib...
//
// Revision 1.6  2003/10/07 00:37:34  panther
// Prior commit in error - Begin render fonts in multi-alpha.
//
// Revision 1.5  2003/10/07 00:32:08  panther
// Fix default font.  Add bit size flag to font
//
// Revision 1.4  2003/06/16 10:17:42  panther
// Export nearly usable renderfont routine... filename, size
//
// Revision 1.3  2003/03/25 08:45:56  panther
// Added CVS logging tag
//
