/*
 * cderror.h
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines the error and message codes for the cjpeg/djpeg
 * applications.  These strings are not needed as part of the JPEG library
 * proper.
 * Edit this file to add new codes, or to translate the message strings to
 * some other language.
 */

/*
 * To define the enum list of message codes, include this file without
 * defining macro JMESSAGE.  To create a message string table, include it
 * again with a suitable JMESSAGE definition (see jerror.c for an example).
 */
#ifndef JMESSAGE
#ifndef CDERROR_H
#define CDERROR_H
/* First time through, define the enum list */
#define JMAKE_ENUM_LIST
#else
/* Repeated inclusions of this file are no-ops unless JMESSAGE is defined */
#define JMESSAGE(code,string)
#endif /* CDERROR_H */
#endif /* JMESSAGE */

#ifdef JMAKE_ENUM_LIST

typedef enum {

#define JMESSAGE(code,string)	code ,

#endif /* JMAKE_ENUM_LIST */

JMESSAGE(JMSG_FIRSTADDONCODE=1000, NULL) /* Must be first entry! */

#ifdef BMP_SUPPORTED
JMESSAGE(JERR_BMP_BADCMAP, WIDE("Unsupported BMP colormap format"))
JMESSAGE(JERR_BMP_BADDEPTH, WIDE("Only 8- and 24-bit BMP files are supported"))
JMESSAGE(JERR_BMP_BADHEADER, WIDE("Invalid BMP file: bad header length"))
JMESSAGE(JERR_BMP_BADPLANES, WIDE("Invalid BMP file: biPlanes not equal to 1"))
JMESSAGE(JERR_BMP_COLORSPACE, WIDE("BMP output must be grayscale or RGB"))
JMESSAGE(JERR_BMP_COMPRESSED, WIDE("Sorry, compressed BMPs not yet supported"))
JMESSAGE(JERR_BMP_NOT, WIDE("Not a BMP file - does not start with BM"))
JMESSAGE(JTRC_BMP, WIDE("%ux%u 24-bit BMP image"))
JMESSAGE(JTRC_BMP_MAPPED, WIDE("%ux%u 8-bit colormapped BMP image"))
JMESSAGE(JTRC_BMP_OS2, WIDE("%ux%u 24-bit OS2 BMP image"))
JMESSAGE(JTRC_BMP_OS2_MAPPED, WIDE("%ux%u 8-bit colormapped OS2 BMP image"))
#endif /* BMP_SUPPORTED */

#ifdef GIF_SUPPORTED
JMESSAGE(JERR_GIF_BUG, WIDE("GIF output got confused"))
JMESSAGE(JERR_GIF_CODESIZE, WIDE("Bogus GIF codesize %d"))
JMESSAGE(JERR_GIF_COLORSPACE, WIDE("GIF output must be grayscale or RGB"))
JMESSAGE(JERR_GIF_IMAGENOTFOUND, WIDE("Too few images in GIF file"))
JMESSAGE(JERR_GIF_NOT, WIDE("Not a GIF file"))
JMESSAGE(JTRC_GIF, WIDE("%ux%ux%d GIF image"))
JMESSAGE(JTRC_GIF_BADVERSION,
	 "Warning: unexpected GIF version number '%c%c%c'")
JMESSAGE(JTRC_GIF_EXTENSION, WIDE("Ignoring GIF extension block of type 0x%02x"))
JMESSAGE(JTRC_GIF_NONSQUARE, WIDE("Caution: nonsquare pixels in input"))
JMESSAGE(JWRN_GIF_BADDATA, WIDE("Corrupt data in GIF file"))
JMESSAGE(JWRN_GIF_CHAR, WIDE("Bogus char 0x%02x in GIF file, ignoring"))
JMESSAGE(JWRN_GIF_ENDCODE, WIDE("Premature end of GIF image"))
JMESSAGE(JWRN_GIF_NOMOREDATA, WIDE("Ran out of GIF bits"))
#endif /* GIF_SUPPORTED */

#ifdef PPM_SUPPORTED
JMESSAGE(JERR_PPM_COLORSPACE, WIDE("PPM output must be grayscale or RGB"))
JMESSAGE(JERR_PPM_NONNUMERIC, WIDE("Nonnumeric data in PPM file"))
JMESSAGE(JERR_PPM_NOT, WIDE("Not a PPM/PGM file"))
JMESSAGE(JTRC_PGM, WIDE("%ux%u PGM image"))
JMESSAGE(JTRC_PGM_TEXT, WIDE("%ux%u text PGM image"))
JMESSAGE(JTRC_PPM, WIDE("%ux%u PPM image"))
JMESSAGE(JTRC_PPM_TEXT, WIDE("%ux%u text PPM image"))
#endif /* PPM_SUPPORTED */

#ifdef RLE_SUPPORTED
JMESSAGE(JERR_RLE_BADERROR, WIDE("Bogus error code from RLE library"))
JMESSAGE(JERR_RLE_COLORSPACE, WIDE("RLE output must be grayscale or RGB"))
JMESSAGE(JERR_RLE_DIMENSIONS, WIDE("Image dimensions (%ux%u) too large for RLE"))
JMESSAGE(JERR_RLE_EMPTY, WIDE("Empty RLE file"))
JMESSAGE(JERR_RLE_EOF, WIDE("Premature EOF in RLE header"))
JMESSAGE(JERR_RLE_MEM, WIDE("Insufficient memory for RLE header"))
JMESSAGE(JERR_RLE_NOT, WIDE("Not an RLE file"))
JMESSAGE(JERR_RLE_TOOMANYCHANNELS, WIDE("Cannot handle %d output channels for RLE"))
JMESSAGE(JERR_RLE_UNSUPPORTED, WIDE("Cannot handle this RLE setup"))
JMESSAGE(JTRC_RLE, WIDE("%ux%u full-color RLE file"))
JMESSAGE(JTRC_RLE_FULLMAP, WIDE("%ux%u full-color RLE file with map of length %d"))
JMESSAGE(JTRC_RLE_GRAY, WIDE("%ux%u grayscale RLE file"))
JMESSAGE(JTRC_RLE_MAPGRAY, WIDE("%ux%u grayscale RLE file with map of length %d"))
JMESSAGE(JTRC_RLE_MAPPED, WIDE("%ux%u colormapped RLE file with map of length %d"))
#endif /* RLE_SUPPORTED */

#ifdef TARGA_SUPPORTED
JMESSAGE(JERR_TGA_BADCMAP, WIDE("Unsupported Targa colormap format"))
JMESSAGE(JERR_TGA_BADPARMS, WIDE("Invalid or unsupported Targa file"))
JMESSAGE(JERR_TGA_COLORSPACE, WIDE("Targa output must be grayscale or RGB"))
JMESSAGE(JTRC_TGA, WIDE("%ux%u RGB Targa image"))
JMESSAGE(JTRC_TGA_GRAY, WIDE("%ux%u grayscale Targa image"))
JMESSAGE(JTRC_TGA_MAPPED, WIDE("%ux%u colormapped Targa image"))
#else
JMESSAGE(JERR_TGA_NOTCOMP, WIDE("Targa support was not compiled"))
#endif /* TARGA_SUPPORTED */

JMESSAGE(JERR_BAD_CMAP_FILE,
	 "Color map file is invalid or of unsupported format")
JMESSAGE(JERR_TOO_MANY_COLORS,
	 "Output file format cannot handle %d colormap entries")
JMESSAGE(JERR_UNGETC_FAILED, WIDE("ungetc failed"))
#ifdef TARGA_SUPPORTED
JMESSAGE(JERR_UNKNOWN_FORMAT,
	 "Unrecognized input file format --- perhaps you need -targa")
#else
JMESSAGE(JERR_UNKNOWN_FORMAT, WIDE("Unrecognized input file format"))
#endif
JMESSAGE(JERR_UNSUPPORTED_FORMAT, WIDE("Unsupported output file format"))

#ifdef JMAKE_ENUM_LIST

  JMSG_LASTADDONCODE
} ADDON_MESSAGE_CODE;

#undef JMAKE_ENUM_LIST
#endif /* JMAKE_ENUM_LIST */

/* Zap JMESSAGE macro so that future re-inclusions do nothing by default */
#undef JMESSAGE
// $Log: $
