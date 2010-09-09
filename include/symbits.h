#ifndef SYMBOLIC_BITS_DEFINED
#define SYMBOLIC_BITS_DEFINED

#define _8bits(n1,n0)  (((n0)<<4)|(n1))

#if defined( BITS_MONO ) || !( defined( BITS_GREY2 ) || defined( BITS_GREY4 )|| defined( BITS_GREY8 ) )

// default definition - allows fonts to step with 1 << bit
// from left to right - avoids sign problems with 0x80
//#if defined( UNNATURAL_ORDER )
#define ____   ( 0x0 )
#define X___   ( 0x1 )
#define _X__   ( 0x2 )
#define XX__   ( 0x3 )
#define __X_   ( 0x4 )
#define X_X_   ( 0x5 )
#define _XX_   ( 0x6 )
#define XXX_   ( 0x7 )
#define ___X   ( 0x8 )
#define X__X   ( 0x9 )
#define _X_X   ( 0xA )
#define XX_X   ( 0xB )
#define __XX   ( 0xC )
#define X_XX   ( 0xD )
#define _XXX   ( 0xE )
#define XXXX   ( 0xF )
//#else
//#define ____   ( 0x0 )
//#define ___X   ( 0x1 )
//#define __X_   ( 0x2 )
//#define __XX   ( 0x3 )
//#define _X__   ( 0x4 )
//#define _X_X   ( 0x5 )
//#define _XX_   ( 0x6 )
//#define _XXX   ( 0x7 )
//#define X___   ( 0x8 )
//#define X__X   ( 0x9 )
//#define X_X_   ( 0xa )
//#define X_XX   ( 0xb )
//#define XX__   ( 0xc )
//#define XX_X   ( 0xd )
//#define XXX_   ( 0xe )
//#define XXXX   ( 0xF )
//
//#define _8bits(n1,n0)  (((n1)<<4)|(n0))
//#endif

#define ________     _8bits(____,____)
#define _______X     _8bits(____,___X)
#define ______X_     _8bits(____,__X_)
#define ______XX     _8bits(____,__XX)
#define _____X__     _8bits(____,_X__)
#define _____X_X     _8bits(____,_X_X)
#define _____XX_     _8bits(____,_XX_)
#define _____XXX     _8bits(____,_XXX)
#define ____X___     _8bits(____,X___)
#define ____X__X     _8bits(____,X__X)
#define ____X_X_     _8bits(____,X_X_)
#define ____X_XX     _8bits(____,X_XX)
#define ____XX__     _8bits(____,XX__)
#define ____XX_X     _8bits(____,XX_X)
#define ____XXX_     _8bits(____,XXX_)
#define ____XXXX     _8bits(____,XXXX)
#define ___X____     _8bits(___X,____)
#define ___X___X     _8bits(___X,___X)
#define ___X__X_     _8bits(___X,__X_)
#define ___X__XX     _8bits(___X,__XX)
#define ___X_X__     _8bits(___X,_X__)
#define ___X_X_X     _8bits(___X,_X_X)
#define ___X_XX_     _8bits(___X,_XX_)
#define ___X_XXX     _8bits(___X,_XXX)
#define ___XX___     _8bits(___X,X___)
#define ___XX__X     _8bits(___X,X__X)
#define ___XX_X_     _8bits(___X,X_X_)
#define ___XX_XX     _8bits(___X,X_XX)
#define ___XXX__     _8bits(___X,XX__)
#define ___XXX_X     _8bits(___X,XX_X)
#define ___XXXX_     _8bits(___X,XXX_)
#define ___XXXXX     _8bits(___X,XXXX)
#define __X_____     _8bits(__X_,____)
#define __X____X     _8bits(__X_,___X)
#define __X___X_     _8bits(__X_,__X_)
#define __X___XX     _8bits(__X_,__XX)
#define __X__X__     _8bits(__X_,_X__)
#define __X__X_X     _8bits(__X_,_X_X)
#define __X__XX_     _8bits(__X_,_XX_)
#define __X__XXX     _8bits(__X_,_XXX)
#define __X_X___     _8bits(__X_,X___)
#define __X_X__X     _8bits(__X_,X__X)
#define __X_X_X_     _8bits(__X_,X_X_)
#define __X_X_XX     _8bits(__X_,X_XX)
#define __X_XX__     _8bits(__X_,XX__)
#define __X_XX_X     _8bits(__X_,XX_X)
#define __X_XXX_     _8bits(__X_,XXX_)
#define __X_XXXX     _8bits(__X_,XXXX)
#define __XX____     _8bits(__XX,____)
#define __XX___X     _8bits(__XX,___X)
#define __XX__X_     _8bits(__XX,__X_)
#define __XX__XX     _8bits(__XX,__XX)
#define __XX_X__     _8bits(__XX,_X__)
#define __XX_X_X     _8bits(__XX,_X_X)
#define __XX_XX_     _8bits(__XX,_XX_)
#define __XX_XXX     _8bits(__XX,_XXX)
#define __XXX___     _8bits(__XX,X___)
#define __XXX__X     _8bits(__XX,X__X)
#define __XXX_X_     _8bits(__XX,X_X_)
#define __XXX_XX     _8bits(__XX,X_XX)
#define __XXXX__     _8bits(__XX,XX__)
#define __XXXX_X     _8bits(__XX,XX_X)
#define __XXXXX_     _8bits(__XX,XXX_)
#define __XXXXXX     _8bits(__XX,XXXX)
#define _X______     _8bits(_X__,____)
#define _X_____X     _8bits(_X__,___X)
#define _X____X_     _8bits(_X__,__X_)
#define _X____XX     _8bits(_X__,__XX)
#define _X___X__     _8bits(_X__,_X__)
#define _X___X_X     _8bits(_X__,_X_X)
#define _X___XX_     _8bits(_X__,_XX_)
#define _X___XXX     _8bits(_X__,_XXX)
#define _X__X___     _8bits(_X__,X___)
#define _X__X__X     _8bits(_X__,X__X)
#define _X__X_X_     _8bits(_X__,X_X_)
#define _X__X_XX     _8bits(_X__,X_XX)
#define _X__XX__     _8bits(_X__,XX__)
#define _X__XX_X     _8bits(_X__,XX_X)
#define _X__XXX_     _8bits(_X__,XXX_)
#define _X__XXXX     _8bits(_X__,XXXX)
#define _X_X____     _8bits(_X_X,____)
#define _X_X___X     _8bits(_X_X,___X)
#define _X_X__X_     _8bits(_X_X,__X_)
#define _X_X__XX     _8bits(_X_X,__XX)
#define _X_X_X__     _8bits(_X_X,_X__)
#define _X_X_X_X     _8bits(_X_X,_X_X)
#define _X_X_XX_     _8bits(_X_X,_XX_)
#define _X_X_XXX     _8bits(_X_X,_XXX)
#define _X_XX___     _8bits(_X_X,X___)
#define _X_XX__X     _8bits(_X_X,X__X)
#define _X_XX_X_     _8bits(_X_X,X_X_)
#define _X_XX_XX     _8bits(_X_X,X_XX)
#define _X_XXX__     _8bits(_X_X,XX__)
#define _X_XXX_X     _8bits(_X_X,XX_X)
#define _X_XXXX_     _8bits(_X_X,XXX_)
#define _X_XXXXX     _8bits(_X_X,XXXX)
#define _XX_____     _8bits(_XX_,____)
#define _XX____X     _8bits(_XX_,___X)
#define _XX___X_     _8bits(_XX_,__X_)
#define _XX___XX     _8bits(_XX_,__XX)
#define _XX__X__     _8bits(_XX_,_X__)
#define _XX__X_X     _8bits(_XX_,_X_X)
#define _XX__XX_     _8bits(_XX_,_XX_)
#define _XX__XXX     _8bits(_XX_,_XXX)
#define _XX_X___     _8bits(_XX_,X___)
#define _XX_X__X     _8bits(_XX_,X__X)
#define _XX_X_X_     _8bits(_XX_,X_X_)
#define _XX_X_XX     _8bits(_XX_,X_XX)
#define _XX_XX__     _8bits(_XX_,XX__)
#define _XX_XX_X     _8bits(_XX_,XX_X)
#define _XX_XXX_     _8bits(_XX_,XXX_)
#define _XX_XXXX     _8bits(_XX_,XXXX)
#define _XXX____     _8bits(_XXX,____)
#define _XXX___X     _8bits(_XXX,___X)
#define _XXX__X_     _8bits(_XXX,__X_)
#define _XXX__XX     _8bits(_XXX,__XX)
#define _XXX_X__     _8bits(_XXX,_X__)
#define _XXX_X_X     _8bits(_XXX,_X_X)
#define _XXX_XX_     _8bits(_XXX,_XX_)
#define _XXX_XXX     _8bits(_XXX,_XXX)
#define _XXXX___     _8bits(_XXX,X___)
#define _XXXX__X     _8bits(_XXX,X__X)
#define _XXXX_X_     _8bits(_XXX,X_X_)
#define _XXXX_XX     _8bits(_XXX,X_XX)
#define _XXXXX__     _8bits(_XXX,XX__)
#define _XXXXX_X     _8bits(_XXX,XX_X)
#define _XXXXXX_     _8bits(_XXX,XXX_)
#define _XXXXXXX     _8bits(_XXX,XXXX)
#define X_______     _8bits(X___,____)
#define X______X     _8bits(X___,___X)
#define X_____X_     _8bits(X___,__X_)
#define X_____XX     _8bits(X___,__XX)
#define X____X__     _8bits(X___,_X__)
#define X____X_X     _8bits(X___,_X_X)
#define X____XX_     _8bits(X___,_XX_)
#define X____XXX     _8bits(X___,_XXX)
#define X___X___     _8bits(X___,X___)
#define X___X__X     _8bits(X___,X__X)
#define X___X_X_     _8bits(X___,X_X_)
#define X___X_XX     _8bits(X___,X_XX)
#define X___XX__     _8bits(X___,XX__)
#define X___XX_X     _8bits(X___,XX_X)
#define X___XXX_     _8bits(X___,XXX_)
#define X___XXXX     _8bits(X___,XXXX)
#define X__X____     _8bits(X__X,____)
#define X__X___X     _8bits(X__X,___X)
#define X__X__X_     _8bits(X__X,__X_)
#define X__X__XX     _8bits(X__X,__XX)
#define X__X_X__     _8bits(X__X,_X__)
#define X__X_X_X     _8bits(X__X,_X_X)
#define X__X_XX_     _8bits(X__X,_XX_)
#define X__X_XXX     _8bits(X__X,_XXX)
#define X__XX___     _8bits(X__X,X___)
#define X__XX__X     _8bits(X__X,X__X)
#define X__XX_X_     _8bits(X__X,X_X_)
#define X__XX_XX     _8bits(X__X,X_XX)
#define X__XXX__     _8bits(X__X,XX__)
#define X__XXX_X     _8bits(X__X,XX_X)
#define X__XXXX_     _8bits(X__X,XXX_)
#define X__XXXXX     _8bits(X__X,XXXX)
#define X_X_____     _8bits(X_X_,____)
#define X_X____X     _8bits(X_X_,___X)
#define X_X___X_     _8bits(X_X_,__X_)
#define X_X___XX     _8bits(X_X_,__XX)
#define X_X__X__     _8bits(X_X_,_X__)
#define X_X__X_X     _8bits(X_X_,_X_X)
#define X_X__XX_     _8bits(X_X_,_XX_)
#define X_X__XXX     _8bits(X_X_,_XXX)
#define X_X_X___     _8bits(X_X_,X___)
#define X_X_X__X     _8bits(X_X_,X__X)
#define X_X_X_X_     _8bits(X_X_,X_X_)
#define X_X_X_XX     _8bits(X_X_,X_XX)
#define X_X_XX__     _8bits(X_X_,XX__)
#define X_X_XX_X     _8bits(X_X_,XX_X)
#define X_X_XXX_     _8bits(X_X_,XXX_)
#define X_X_XXXX     _8bits(X_X_,XXXX)
#define X_XX____     _8bits(X_XX,____)
#define X_XX___X     _8bits(X_XX,___X)
#define X_XX__X_     _8bits(X_XX,__X_)
#define X_XX__XX     _8bits(X_XX,__XX)
#define X_XX_X__     _8bits(X_XX,_X__)
#define X_XX_X_X     _8bits(X_XX,_X_X)
#define X_XX_XX_     _8bits(X_XX,_XX_)
#define X_XX_XXX     _8bits(X_XX,_XXX)
#define X_XXX___     _8bits(X_XX,X___)
#define X_XXX__X     _8bits(X_XX,X__X)
#define X_XXX_X_     _8bits(X_XX,X_X_)
#define X_XXX_XX     _8bits(X_XX,X_XX)
#define X_XXXX__     _8bits(X_XX,XX__)
#define X_XXXX_X     _8bits(X_XX,XX_X)
#define X_XXXXX_     _8bits(X_XX,XXX_)
#define X_XXXXXX     _8bits(X_XX,XXXX)
#define XX______     _8bits(XX__,____)
#define XX_____X     _8bits(XX__,___X)
#define XX____X_     _8bits(XX__,__X_)
#define XX____XX     _8bits(XX__,__XX)
#define XX___X__     _8bits(XX__,_X__)
#define XX___X_X     _8bits(XX__,_X_X)
#define XX___XX_     _8bits(XX__,_XX_)
#define XX___XXX     _8bits(XX__,_XXX)
#define XX__X___     _8bits(XX__,X___)
#define XX__X__X     _8bits(XX__,X__X)
#define XX__X_X_     _8bits(XX__,X_X_)
#define XX__X_XX     _8bits(XX__,X_XX)
#define XX__XX__     _8bits(XX__,XX__)
#define XX__XX_X     _8bits(XX__,XX_X)
#define XX__XXX_     _8bits(XX__,XXX_)
#define XX__XXXX     _8bits(XX__,XXXX)
#define XX_X____     _8bits(XX_X,____)
#define XX_X___X     _8bits(XX_X,___X)
#define XX_X__X_     _8bits(XX_X,__X_)
#define XX_X__XX     _8bits(XX_X,__XX)
#define XX_X_X__     _8bits(XX_X,_X__)
#define XX_X_X_X     _8bits(XX_X,_X_X)
#define XX_X_XX_     _8bits(XX_X,_XX_)
#define XX_X_XXX     _8bits(XX_X,_XXX)
#define XX_XX___     _8bits(XX_X,X___)
#define XX_XX__X     _8bits(XX_X,X__X)
#define XX_XX_X_     _8bits(XX_X,X_X_)
#define XX_XX_XX     _8bits(XX_X,X_XX)
#define XX_XXX__     _8bits(XX_X,XX__)
#define XX_XXX_X     _8bits(XX_X,XX_X)
#define XX_XXXX_     _8bits(XX_X,XXX_)
#define XX_XXXXX     _8bits(XX_X,XXXX)
#define XXX_____     _8bits(XXX_,____)
#define XXX____X     _8bits(XXX_,___X)
#define XXX___X_     _8bits(XXX_,__X_)
#define XXX___XX     _8bits(XXX_,__XX)
#define XXX__X__     _8bits(XXX_,_X__)
#define XXX__X_X     _8bits(XXX_,_X_X)
#define XXX__XX_     _8bits(XXX_,_XX_)
#define XXX__XXX     _8bits(XXX_,_XXX)
#define XXX_X___     _8bits(XXX_,X___)
#define XXX_X__X     _8bits(XXX_,X__X)
#define XXX_X_X_     _8bits(XXX_,X_X_)
#define XXX_X_XX     _8bits(XXX_,X_XX)
#define XXX_XX__     _8bits(XXX_,XX__)
#define XXX_XX_X     _8bits(XXX_,XX_X)
#define XXX_XXX_     _8bits(XXX_,XXX_)
#define XXX_XXXX     _8bits(XXX_,XXXX)
#define XXXX____     _8bits(XXXX,____)
#define XXXX___X     _8bits(XXXX,___X)
#define XXXX__X_     _8bits(XXXX,__X_)
#define XXXX__XX     _8bits(XXXX,__XX)
#define XXXX_X__     _8bits(XXXX,_X__)
#define XXXX_X_X     _8bits(XXXX,_X_X)
#define XXXX_XX_     _8bits(XXXX,_XX_)
#define XXXX_XXX     _8bits(XXXX,_XXX)
#define XXXXX___     _8bits(XXXX,X___)
#define XXXXX__X     _8bits(XXXX,X__X)
#define XXXXX_X_     _8bits(XXXX,X_X_)
#define XXXXX_XX     _8bits(XXXX,X_XX)
#define XXXXXX__     _8bits(XXXX,XX__)
#define XXXXXX_X     _8bits(XXXX,XX_X)
#define XXXXXXX_     _8bits(XXXX,XXX_)
#define XXXXXXXX     _8bits(XXXX,XXXX)

#endif

#ifdef BITS_GREY2

#define __  0
#define o_  1
#define O_  2
#define X_  3
#define _o  4
#define oo  5
#define Oo  6
#define Xo  7
#define _O  8
#define oO  9
#define OO  10
#define XO  11
#define _X  12
#define oX  13
#define OX  14
#define XX  15

#define ____  _8bits( __,__ )
#define o___  _8bits( o_,__ )
#define O___  _8bits( O_,__ )
#define X___  _8bits( X_,__ )
#define _o__  _8bits( _o,__ )
#define oo__  _8bits( oo,__ )
#define Oo__  _8bits( Oo,__ )
#define Xo__  _8bits( Xo,__ )
#define _O__  _8bits( _O,__ )
#define oO__  _8bits( oO,__ )
#define OO__  _8bits( OO,__ )
#define XO__  _8bits( XO,__ )
#define _X__  _8bits( _X,__ )
#define oX__  _8bits( oX,__ )
#define OX__  _8bits( OX,__ )
#define XX__  _8bits( XX,__ )
#define __o_  _8bits( __,o_ )
#define o_o_  _8bits( o_,o_ )
#define O_o_  _8bits( O_,o_ )
#define X_o_  _8bits( X_,o_ )
#define _oo_  _8bits( _o,o_ )
#define ooo_  _8bits( oo,o_ )
#define Ooo_  _8bits( Oo,o_ )
#define Xoo_  _8bits( Xo,o_ )
#define _Oo_  _8bits( _O,o_ )
#define oOo_  _8bits( oO,o_ )
#define OOo_  _8bits( OO,o_ )
#define XOo_  _8bits( XO,o_ )
#define _Xo_  _8bits( _X,o_ )
#define oXo_  _8bits( oX,o_ )
#define OXo_  _8bits( OX,o_ )
#define XXo_  _8bits( XX,o_ )
#define __O_  _8bits( __,O_ )
#define o_O_  _8bits( o_,O_ )
#define O_O_  _8bits( O_,O_ )
#define X_O_  _8bits( X_,O_ )
#define _oO_  _8bits( _o,O_ )
#define ooO_  _8bits( oo,O_ )
#define OoO_  _8bits( Oo,O_ )
#define XoO_  _8bits( Xo,O_ )
#define _OO_  _8bits( _O,O_ )
#define oOO_  _8bits( oO,O_ )
#define OOO_  _8bits( OO,O_ )
#define XOO_  _8bits( XO,O_ )
#define _XO_  _8bits( _X,O_ )
#define oXO_  _8bits( oX,O_ )
#define OXO_  _8bits( OX,O_ )
#define XXO_  _8bits( XX,O_ )
#define __X_  _8bits( __,X_ )
#define o_X_  _8bits( o_,X_ )
#define O_X_  _8bits( O_,X_ )
#define X_X_  _8bits( X_,X_ )
#define _oX_  _8bits( _o,X_ )
#define ooX_  _8bits( oo,X_ )
#define OoX_  _8bits( Oo,X_ )
#define XoX_  _8bits( Xo,X_ )
#define _OX_  _8bits( _O,X_ )
#define oOX_  _8bits( oO,X_ )
#define OOX_  _8bits( OO,X_ )
#define XOX_  _8bits( XO,X_ )
#define _XX_  _8bits( _X,X_ )
#define oXX_  _8bits( oX,X_ )
#define OXX_  _8bits( OX,X_ )
#define XXX_  _8bits( XX,X_ )
#define ___o  _8bits( __,_o )
#define o__o  _8bits( o_,_o )
#define O__o  _8bits( O_,_o )
#define X__o  _8bits( X_,_o )
#define _o_o  _8bits( _o,_o )
#define oo_o  _8bits( oo,_o )
#define Oo_o  _8bits( Oo,_o )
#define Xo_o  _8bits( Xo,_o )
#define _O_o  _8bits( _O,_o )
#define oO_o  _8bits( oO,_o )
#define OO_o  _8bits( OO,_o )
#define XO_o  _8bits( XO,_o )
#define _X_o  _8bits( _X,_o )
#define oX_o  _8bits( oX,_o )
#define OX_o  _8bits( OX,_o )
#define XX_o  _8bits( XX,_o )
#define __oo  _8bits( __,oo )
#define o_oo  _8bits( o_,oo )
#define O_oo  _8bits( O_,oo )
#define X_oo  _8bits( X_,oo )
#define _ooo  _8bits( _o,oo )
#define oooo  _8bits( oo,oo )
#define Oooo  _8bits( Oo,oo )
#define Xooo  _8bits( Xo,oo )
#define _Ooo  _8bits( _O,oo )
#define oOoo  _8bits( oO,oo )
#define OOoo  _8bits( OO,oo )
#define XOoo  _8bits( XO,oo )
#define _Xoo  _8bits( _X,oo )
#define oXoo  _8bits( oX,oo )
#define OXoo  _8bits( OX,oo )
#define XXoo  _8bits( XX,oo )
#define __Oo  _8bits( __,Oo )
#define o_Oo  _8bits( o_,Oo )
#define O_Oo  _8bits( O_,Oo )
#define X_Oo  _8bits( X_,Oo )
#define _oOo  _8bits( _o,Oo )
#define ooOo  _8bits( oo,Oo )
#define OoOo  _8bits( Oo,Oo )
#define XoOo  _8bits( Xo,Oo )
#define _OOo  _8bits( _O,Oo )
#define oOOo  _8bits( oO,Oo )
#define OOOo  _8bits( OO,Oo )
#define XOOo  _8bits( XO,Oo )
#define _XOo  _8bits( _X,Oo )
#define oXOo  _8bits( oX,Oo )
#define OXOo  _8bits( OX,Oo )
#define XXOo  _8bits( XX,Oo )
#define __Xo  _8bits( __,Xo )
#define o_Xo  _8bits( o_,Xo )
#define O_Xo  _8bits( O_,Xo )
#define X_Xo  _8bits( X_,Xo )
#define _oXo  _8bits( _o,Xo )
#define ooXo  _8bits( oo,Xo )
#define OoXo  _8bits( Oo,Xo )
#define XoXo  _8bits( Xo,Xo )
#define _OXo  _8bits( _O,Xo )
#define oOXo  _8bits( oO,Xo )
#define OOXo  _8bits( OO,Xo )
#define XOXo  _8bits( XO,Xo )
#define _XXo  _8bits( _X,Xo )
#define oXXo  _8bits( oX,Xo )
#define OXXo  _8bits( OX,Xo )
#define XXXo  _8bits( XX,Xo )
#define ___O  _8bits( __,_O )
#define o__O  _8bits( o_,_O )
#define O__O  _8bits( O_,_O )
#define X__O  _8bits( X_,_O )
#define _o_O  _8bits( _o,_O )
#define oo_O  _8bits( oo,_O )
#define Oo_O  _8bits( Oo,_O )
#define Xo_O  _8bits( Xo,_O )
#define _O_O  _8bits( _O,_O )
#define oO_O  _8bits( oO,_O )
#define OO_O  _8bits( OO,_O )
#define XO_O  _8bits( XO,_O )
#define _X_O  _8bits( _X,_O )
#define oX_O  _8bits( oX,_O )
#define OX_O  _8bits( OX,_O )
#define XX_O  _8bits( XX,_O )
#define __oO  _8bits( __,oO )
#define o_oO  _8bits( o_,oO )
#define O_oO  _8bits( O_,oO )
#define X_oO  _8bits( X_,oO )
#define _ooO  _8bits( _o,oO )
#define oooO  _8bits( oo,oO )
#define OooO  _8bits( Oo,oO )
#define XooO  _8bits( Xo,oO )
#define _OoO  _8bits( _O,oO )
#define oOoO  _8bits( oO,oO )
#define OOoO  _8bits( OO,oO )
#define XOoO  _8bits( XO,oO )
#define _XoO  _8bits( _X,oO )
#define oXoO  _8bits( oX,oO )
#define OXoO  _8bits( OX,oO )
#define XXoO  _8bits( XX,oO )
#define __OO  _8bits( __,OO )
#define o_OO  _8bits( o_,OO )
#define O_OO  _8bits( O_,OO )
#define X_OO  _8bits( X_,OO )
#define _oOO  _8bits( _o,OO )
#define ooOO  _8bits( oo,OO )
#define OoOO  _8bits( Oo,OO )
#define XoOO  _8bits( Xo,OO )
#define _OOO  _8bits( _O,OO )
#define oOOO  _8bits( oO,OO )
#define OOOO  _8bits( OO,OO )
#define XOOO  _8bits( XO,OO )
#define _XOO  _8bits( _X,OO )
#define oXOO  _8bits( oX,OO )
#define OXOO  _8bits( OX,OO )
#define XXOO  _8bits( XX,OO )
#define __XO  _8bits( __,XO )
#define o_XO  _8bits( o_,XO )
#define O_XO  _8bits( O_,XO )
#define X_XO  _8bits( X_,XO )
#define _oXO  _8bits( _o,XO )
#define ooXO  _8bits( oo,XO )
#define OoXO  _8bits( Oo,XO )
#define XoXO  _8bits( Xo,XO )
#define _OXO  _8bits( _O,XO )
#define oOXO  _8bits( oO,XO )
#define OOXO  _8bits( OO,XO )
#define XOXO  _8bits( XO,XO )
#define _XXO  _8bits( _X,XO )
#define oXXO  _8bits( oX,XO )
#define OXXO  _8bits( OX,XO )
#define XXXO  _8bits( XX,XO )
#define ___X  _8bits( __,_X )
#define o__X  _8bits( o_,_X )
#define O__X  _8bits( O_,_X )
#define X__X  _8bits( X_,_X )
#define _o_X  _8bits( _o,_X )
#define oo_X  _8bits( oo,_X )
#define Oo_X  _8bits( Oo,_X )
#define Xo_X  _8bits( Xo,_X )
#define _O_X  _8bits( _O,_X )
#define oO_X  _8bits( oO,_X )
#define OO_X  _8bits( OO,_X )
#define XO_X  _8bits( XO,_X )
#define _X_X  _8bits( _X,_X )
#define oX_X  _8bits( oX,_X )
#define OX_X  _8bits( OX,_X )
#define XX_X  _8bits( XX,_X )
#define __oX  _8bits( __,oX )
#define o_oX  _8bits( o_,oX )
#define O_oX  _8bits( O_,oX )
#define X_oX  _8bits( X_,oX )
#define _ooX  _8bits( _o,oX )
#define oooX  _8bits( oo,oX )
#define OooX  _8bits( Oo,oX )
#define XooX  _8bits( Xo,oX )
#define _OoX  _8bits( _O,oX )
#define oOoX  _8bits( oO,oX )
#define OOoX  _8bits( OO,oX )
#define XOoX  _8bits( XO,oX )
#define _XoX  _8bits( _X,oX )
#define oXoX  _8bits( oX,oX )
#define OXoX  _8bits( OX,oX )
#define XXoX  _8bits( XX,oX )
#define __OX  _8bits( __,OX )
#define o_OX  _8bits( o_,OX )
#define O_OX  _8bits( O_,OX )
#define X_OX  _8bits( X_,OX )
#define _oOX  _8bits( _o,OX )
#define ooOX  _8bits( oo,OX )
#define OoOX  _8bits( Oo,OX )
#define XoOX  _8bits( Xo,OX )
#define _OOX  _8bits( _O,OX )
#define oOOX  _8bits( oO,OX )
#define OOOX  _8bits( OO,OX )
#define XOOX  _8bits( XO,OX )
#define _XOX  _8bits( _X,OX )
#define oXOX  _8bits( oX,OX )
#define OXOX  _8bits( OX,OX )
#define XXOX  _8bits( XX,OX )
#define __XX  _8bits( __,XX )
#define o_XX  _8bits( o_,XX )
#define O_XX  _8bits( O_,XX )
#define X_XX  _8bits( X_,XX )
#define _oXX  _8bits( _o,XX )
#define ooXX  _8bits( oo,XX )
#define OoXX  _8bits( Oo,XX )
#define XoXX  _8bits( Xo,XX )
#define _OXX  _8bits( _O,XX )
#define oOXX  _8bits( oO,XX )
#define OOXX  _8bits( OO,XX )
#define XOXX  _8bits( XO,XX )
#define _XXX  _8bits( _X,XX )
#define oXXX  _8bits( oX,XX )
#define OXXX  _8bits( OX,XX )
#define XXXX  _8bits( XX,XX )

#endif
        
#ifdef BITS_GREY4
// probably will just record literal values...
// symbolics will just confuse the issue....
#endif

#ifdef BITS_GREY8
// probably will just record literal values...
// symbolics will just confuse the issue....
#endif

#endif
// okay I do need this everywhere that sues these symbols :/
//#undef _8bits  // we don't need this any longer...// $Log: symbits.h,v $
//#undef _8bits  // we don't need this any longer...// Revision 1.3  2003/03/25 08:38:11  panther
//#undef _8bits  // we don't need this any longer...// Add logging
//#undef _8bits  // we don't need this any longer...//
