
PSI_COLORWELL_NAMESPACE
	
// common dialog to get a color... returns TRUE if *result is set
// if result is NULL color is not returned, but still can set presets...
#define  COLORWELL_NAME WIDE("Color Well")

PSI_PROC( int, PickColorEx )( CDATA *result, CDATA original, PSI_CONTROL hAbove, int x, int y );
PSI_PROC( int, PickColor)( CDATA *result, CDATA original, PSI_CONTROL pAbove );
// creates a control which can then select a color

CONTROL_PROC( ColorWell, (CDATA color) );
#define MakeColorWell(f,x,y,w,h,id,c) SetColorWell( MakeNamedControl(f,COLORWELL_NAME,x,y,w,h,id ),c)

PSI_PROC( void, SetShadeMin )( PSI_CONTROL pc, CDATA color );
PSI_PROC( void, SetShadeMax )( PSI_CONTROL pc, CDATA color );
PSI_PROC( void, SetShadeMid )( PSI_CONTROL pc, CDATA color );

PSI_PROC( PSI_CONTROL, SetColorWell )( PSI_CONTROL pc, CDATA color );
PSI_PROC( PSI_CONTROL, EnableColorWellPick )( PSI_CONTROL pc, LOGICAL bEnable );
PSI_PROC( PSI_CONTROL, SetOnUpdateColorWell )( PSI_CONTROL PC, void(CPROC*)(PTRSZVAL,CDATA), PTRSZVAL);
PSI_PROC( CDATA, GetColorFromWell )( PSI_CONTROL pc );
PSI_COLORWELL_NAMESPACE_END
USE_PSI_COLORWELL_NAMESPACE


