
#ifndef __TEMPLATE_H_DEFINED__
#define __TEMPLATE_H_DEFINED__

#include <stdhdrs.h>
	// and most defination IS WINDOWS
   // need a CUPs personality.
	
#include <sack_types.h> // PLIST
#include <image.h>

//--------------------------------------------------------------

typedef struct device_tag {
	int width, height;
	int scalex, scaley; // pixels per 'inch'
	//HDC hdc;
   Image image;
   char filename[256]; // well ... yeah :)
} DEVICE, *PDEVICE;

//--------------------------------------------------------------

#include "fractions.h"

typedef struct printimage_tag {
	COORDPAIR location;
	COORDPAIR size;
	Image image;
	struct printimage_tag *next;
} PRINTIMAGE, *PPRINTIMAGE;

typedef struct printfont_tag {
	char name[64];
	Font hFont;
   COORDPAIR size;
	char text[64];
	_32 color;
	struct {
		unsigned int fixed:1;
		unsigned int bold:1;
		unsigned int center:1;
		unsigned int script:1;
		unsigned int inverted:1;
		unsigned int vertical:1;
	} flags;
	struct printfont_tag *next;
} PRINTFONT, *PPRINTFONT;

typedef struct printtext_tag {
   struct {
      unsigned int resolvedfont:1;
   } flags;
	char *text;
	PPRINTFONT font;
	COORDPAIR position;
	struct printtext_tag *next;
} PRINTTEXT, *PPRINTTEXT;

typedef struct printrect_tag {
	COORDPAIR location;
	COORDPAIR size;
	_32 color;
	struct printrect_tag *next;
} PRINTRECT, *PPRINTRECT;

typedef struct printline_tag {
	COORDPAIR from;
	COORDPAIR to;
	_32 color;
	struct printline_tag *next;
} PRINTLINE, *PPRINTLINE;

typedef struct printbarcode_tag {
	COORDPAIR origin;
	COORDPAIR size;
	struct {
		unsigned int vertical:1;
	} flags;
	char *value;
	struct printbarcode_tag *next;
} PRINTBARCODE, *PPRINTBARCODE;

typedef struct printvalidate_tag {
	char *text;
} PRINTVALIDATE, *PPRINTVALIDATE;

typedef struct printpage_tag {
   struct printmacro_tag *DefinedIn;

	struct {
		unsigned int bLandscape : 1; // if not landscape is portrait
	} flags;
	COORDPAIR pagesize;

	DEVICE output;
	PPRINTLINE    lines;
	PPRINTRECT    rects;
	PPRINTIMAGE   images;
   PPRINTBARCODE barcodes;

	PPRINTFONT    fonts;
	PPRINTTEXT    texts;

   PLIST         MacroList;      // list of MACRORUNs to be executed...

   // current run state information....
   COORDPAIR    Origin; // used for offseting positions

	struct printpage_tag *next;
} PRINTPAGE, *PPRINTPAGE;

typedef struct printoperation_tag {
	int nType;
	union {
		// do not allow fonts as macro operations?
		PPRINTPAGE  page;
		PPRINTLINE  line;
		PPRINTFONT  font;
		PPRINTTEXT  text;
		PPRINTRECT  rect;
		PPRINTIMAGE image;
		PPRINTBARCODE barcode;
		PPRINTVALIDATE validate; // can abort macro usage if validation fails(no such card)
      // move_origin also uses this value...
		COORDPAIR   origin; // offset from current origin by this amount...
      struct printmacrorun_tag *runmacro;
	} data;
} PRINTOPERATION, *PPRINTOPERATION;


enum {
	PO_PAGE
, PO_LINE
, PO_TEXT
, PO_RECT
, PO_IMAGE	
, PO_BARCODE
, PO_VALIDATE
, PO_RUNMACRO
, PO_ORIGIN
	  , PO_FONT
     , PO_MOVE_ORIGIN
// looks like I'm considering nesting macros... 
}; // print op codes...

// ends up being multiply defined.... *shrug*
static char *opnames[] = { "Page", WIDE("Line"), WIDE("Text"), WIDE("Rect"), WIDE("Image")
								 , WIDE("Barcode"), WIDE("Validate")
								 , WIDE("RunningMacro"), WIDE("Origin")
                         , WIDE("Font")
								 , WIDE("MoveOrigin")
};

typedef struct printmacro_tag {
	char *name;
	int   namelen;
	PLIST Parameters; // names of things which are parameters
	PLIST Operations; // list of strict text operations which should be performed 
   struct printmacro_tag *next; // next macro in list
	struct printmacro_tag *priorrecording;
} PRINTMACRO, *PPRINTMACRO;

typedef struct printmacrorun_tag {
	// used when assigned to RUN a macro...
	PPRINTMACRO macro;
	PLIST 		Values; // values to substitute when doing the macro...
	COORDPAIR   Origin, OriginalOrigin;
   struct printmacrorun_tag *priorrunning;
} PRINTMACRORUN, *PPRINTMACRORUN;

//--------------------------------------------------------------------------

typedef struct template_tag {
	char 			*name;
   int   		 namelen;

   // collections of pages.... need a page to mount
   // rectangles, lines, images, etc, all but fonts and macros.
	PPRINTPAGE   pages;
   PPRINTPAGE   currentpage; // which page we are building.

   // global resources for the template....
	PPRINTMACRO  macros;         // list of all macros
   PPRINTMACRO  MacroRecording; // macro which is currently being recorded

   // these macros are  run ONCE after the template
   // loads - they will result in defining pages....
   // then this list can/will be destroyed.... 
	PLIST        MacroList;      // list of MACRORUNs to be executed...

	struct template_tag *next;
} TEMPLATE, *PTEMPLATE;

//--------------------------------------------------------------

int LoadTemplate( char *templatename, char *templatefilename );
PTEMPLATE FindTemplate( char *name );

PPRINTMACRO FindMacro( char *name, PTEMPLATE template );
PPRINTFONT FindFont( char *name, PPRINTPAGE page );

// unsure where this routine should be located - needs much information
// from the core information....
int SubstPrint( char *dest, char *src, PPRINTMACRORUN Macro );


#endif
