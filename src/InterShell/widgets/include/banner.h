#ifndef BANNER_WIDGET_DEFINED
#define BANNER_WIDGET_DEFINED

#include <controls.h>


#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef BANNER_SOURCE
#define BANNER_PROC(type,name) __declspec(dllexport) type CPROC name
#else
#define BANNER_PROC(type,name) __declspec(dllimport) type CPROC name
#endif
#else
#ifdef BANNER_SOURCE
#define BANNER_PROC(type,name) EXPORT_METHOD type name
#else
#define BANNER_PROC(type,name) extern type name
#endif
#endif


typedef struct banner_tag *PBANNER;

#define BANNER_CLICK    0x01
#define BANNER_TIMEOUT  0x02
#define BANNER_CLOSED   0x04
#define BANNER_WAITING  0x08
#define BANNER_OKAY     0x10
#define BANNER_DEAD     0x20 // banner does not click, banner does not timeout...
#define BANNER_TOP      0x40 // banner does not click, banner does not timeout...
#define BANNER_NOWAIT   0x80 // don't wait in banner create...
#define BANNER_EXPLORER  0x100 // don't wait in banner create...
#define BANNER_ABSOLUTE_TOP      (0x200|BANNER_TOP) // banner does not click, banner does not timeout...
#define BANNER_EXTENDED_RESULT     0x400  //OKAY/CANCEL on YesNo dialog 

#define BANNER_OPTION_YESNO          0x010000
#define BANNER_OPTION_OKAYCANCEL     0x020000

#define BANNER_OPTION_LEFT_JUSTIFY   0x100000

	// banner will have a keyboard to enter a value...
   // banner text will show above the keyboard?
#include <render.h>

#define BANNER_OPTION_KEYBOARD       0x01000000

BANNER_PROC( int, CreateBanner2Extended )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor, int lines, int cols, int display );
BANNER_PROC( int, CreateBanner2Exx )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor );
BANNER_PROC( int, CreateBanner2Ex )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout );
BANNER_PROC( int, CreateBanner2 )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text );
BANNER_PROC( void, SetBanner2Options )( PBANNER, _32 flags, _32 extra );
BANNER_PROC( void, SetBanner2OptionsEx )( PBANNER*, _32 flags, _32 extra );
#define SetBanner2Options( banner,flags,extra ) SetBanner2OptionsEx( &(banner), flags,extra )

// click, closed, etc...
// if it's in wait, it will not be destroyed until after
// wait results...
// if wait is not active, .. then the resulting kill banner
// will clear the ppBanner originally passed... so that becomes
// the wait state variable...
// results false if no banner to wait for.
BANNER_PROC( void, SetBanner2Text )( PBANNER banner, TEXTCHAR *text );
BANNER_PROC( int, WaitForBanner2 )( PBANNER banner );
BANNER_PROC( int, WaitForBanner2Ex )( PBANNER *banner );
#define WaitForBanner2(b) WaitForBanner2Ex( &(b) )
BANNER_PROC( void, RemoveBanner2Ex )( PBANNER *banner DBG_PASS );
BANNER_PROC( void, RemoveBanner2 )( PBANNER banner );
#define RemoveBanner2(b) RemoveBanner2Ex( &(b) DBG_SRC )
BANNER_PROC( Font, GetBanner2Font )( void );
BANNER_PROC( _32, GetBanner2FontHeight )( void );

BANNER_PROC( PRENDERER, GetBanner2Renderer )( PBANNER banner );
BANNER_PROC( PSI_CONTROL, GetBanner2Control )( PBANNER banner );

typedef void (CPROC *DoConfirmProc)( void );
BANNER_PROC( int, Banner2ThreadConfirm )( CTEXTSTR type, CTEXTSTR msg, DoConfirmProc dokey );
BANNER_PROC( void, Banner2AnswerYes )( CTEXTSTR type );
BANNER_PROC( void, Banner2AnswerNo )( CTEXTSTR type );


#define Banner2NoWait( text ) CreateBanner2Ex( NULL, NULL, text, BANNER_NOWAIT|BANNER_DEAD, 0 )
#define Banner2TopNoWait( text ) CreateBanner2Ex( NULL, NULL, text, BANNER_NOWAIT|BANNER_DEAD|BANNER_TOP, 0 )
#define Banner2Top( text ) CreateBanner2Ex( NULL, NULL, text, BANNER_DEAD|BANNER_TOP|BANNER_TIMEOUT, 5000 )
#define Banner2YesNo( renderparent, text ) CreateBanner2Ex( renderparent, NULL, text, BANNER_OPTION_YESNO, 0 )
#define Banner2TopYesNo( renderparent, text ) CreateBanner2Ex( renderparent, NULL, text, BANNER_TOP|BANNER_OPTION_YESNO, 0 )
#define Banner2YesNoEx( renderparent, pb, text ) CreateBanner2Ex( renderparent, pb, text, BANNER_OPTION_YESNO, 0 )
#define Banner2Message( text ) CreateBanner2( NULL, NULL, text )
#define Banner2MessageEx( renderparent,text ) CreateBanner2( renderparent, NULL, text )
#define RemoveBanner2Message() RemoveBanner2( NULL )



BANNER_PROC( int, CreateBanner3Extended )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor, int lines, int cols, int display );
BANNER_PROC( int, CreateBanner3Exx )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor );
BANNER_PROC( int, CreateBanner3Ex )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout );
BANNER_PROC( int, CreateBanner3 )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text );
BANNER_PROC( void, SetBanner3Options )( PBANNER, _32 flags, _32 extra );
BANNER_PROC( void, SetBanner3OptionsEx )( PBANNER*, _32 flags, _32 extra );
#define SetBanner3Options( banner,flags,extra ) SetBanner3OptionsEx( &(banner), flags,extra )

// click, closed, etc...
// if it's in wait, it will not be destroyed until after
// wait results...
// if wait is not active, .. then the resulting kill banner
// will clear the ppBanner originally passed... so that becomes
// the wait state variable...
// results false if no banner to wait for.
BANNER_PROC( void, SetBanner3Text )( PBANNER banner, TEXTCHAR *text );
BANNER_PROC( int, WaitForBanner3 )( PBANNER banner );
BANNER_PROC( int, WaitForBanner3Ex )( PBANNER *banner );
#define WaitForBanner3(b) WaitForBanner3Ex( &(b) )
BANNER_PROC( void, RemoveBanner3Ex )( PBANNER *banner DBG_PASS );
BANNER_PROC( void, RemoveBanner3 )( PBANNER banner );
#define RemoveBanner3(b) RemoveBanner3Ex( &(b) DBG_SRC )
BANNER_PROC( Font, GetBanner3Font )( void );
BANNER_PROC( _32, GetBanner3FontHeight )( void );

BANNER_PROC( PRENDERER, GetBanner3Renderer )( PBANNER banner );
BANNER_PROC( PSI_CONTROL, GetBanner3Control )( PBANNER banner );

typedef void (CPROC *DoConfirmProc)( void );
BANNER_PROC( int, Banner3ThreadConfirm )( CTEXTSTR type, CTEXTSTR msg, DoConfirmProc dokey );
BANNER_PROC( void, Banner3AnswerYes )( CTEXTSTR type );
BANNER_PROC( void, Banner3AnswerNo )( CTEXTSTR type );


#define Banner3NoWait( text ) CreateBanner3Ex( NULL, NULL, text, BANNER_NOWAIT|BANNER_DEAD, 0 )
#define Banner3TopNoWait( text ) CreateBanner3Ex( NULL, NULL, text, BANNER_NOWAIT|BANNER_DEAD|BANNER_TOP, 0 )
#define Banner3Top( text ) CreateBanner3Ex( NULL, NULL, text, BANNER_DEAD|BANNER_TOP|BANNER_TIMEOUT, 5000 )
#define Banner3YesNo( renderparent, text ) CreateBanner3Ex( renderparent, NULL, text, BANNER_OPTION_YESNO, 0 )
#define Banner3TopYesNo( renderparent, text ) CreateBanner3Ex( renderparent, NULL, text, BANNER_TOP|BANNER_OPTION_YESNO, 0 )
#define Banner3YesNoEx( renderparent, pb, text ) CreateBanner3Ex( renderparent, pb, text, BANNER_OPTION_YESNO, 0 )
#define Banner3Message( text ) CreateBanner3( NULL, NULL, text )
#define Banner3MessageEx( renderparent,text ) CreateBanner3( renderparent, NULL, text )
#define RemoveBanner3Message() RemoveBanner3( NULL )



#endif
