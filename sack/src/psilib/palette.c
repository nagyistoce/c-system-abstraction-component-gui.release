#include <stdhdrs.h>
#include <stdio.h>
#include <sharemem.h>
#include <colordef.h>
//#include "controlstruc.h"
#include <psi.h>
#include <psi/shadewell.h>
#include <filedotnet.h>

#include "resource.h"

PSI_COLORWELL_NAMESPACE

typedef struct colorwell {
	struct {
		_32 bPickColor : 1;
		_32 bPicking : 1;
	} flags;
	CDATA color;
	
	void(CPROC*UpdateProc)(PTRSZVAL,CDATA);
	PTRSZVAL psvUpdate;
} COLOR_WELL, *PCOLOR_WELL;

enum {
	SHADER_LIGTDARK
	  , SHADER_RED
	  , SHADER_BLUE
     , SHADER_GREEN
};
typedef struct shadewell {
   //int nShaderType;
   CDATA color_min;
   CDATA color_mid;
   CDATA color_max;
} SHADE_WELL, *PSHADE_WELL;

extern CONTROL_REGISTRATION shade_well, color_well;


typedef struct PickColor_tag
{
	PSI_CONTROL frame;
	struct {
		_32 bSettingShade : 1;
	} flags;
	int nGreen; // level of green...
	int Alpha;	 // level of alpha...
	CDATA CurrentColor;
	CDATA Presets[36];
	PCONTROL LastPreset;
   PCONTROL pcZoom;
   PSI_CONTROL psw, pShadeRed, pShadeBlue, pShadeGreen; // shade well data...
	int bSetPreset;
   int ColorDialogDone, ColorDialogOkay;
} PICK_COLOR_DATA, *PPICK_COLOR_DATA;

void CPROC InitColorDataDefault( POINTER );
void SetShaderControls( PPICK_COLOR_DATA ppcd, PSI_CONTROL source );

PUBLIC_DATA( WIDE("Color Choice Data"), PICK_COLOR_DATA, InitColorDataDefault, NULL );

#define nScale 4


#define xbias 1
#define ybias 1
#define xsize 133
#define ysize 133

#define COLOR Color( (255-red)*nScale, (255-green)*nScale, (blue)*nScale )

//----------------------------------------------------------------------------

static void UpdateImage( Image pImage, int nGreen )
{
	int red, blue, green=0;
	if( !pImage )
		return;
	ClearImageTo( pImage, Color( 0, 0, 0 ) );

	for( green = 0; green < nGreen/nScale; green++ )
	{
		blue = 0;
		for( red = 0; red < 256/nScale; red++ )
		{
			plot( pImage, xbias+red + blue+(blue&1), ybias+(green + 128/nScale) - red/2 + blue/2 , COLOR );
		}
		red = 255/nScale;
		for( blue = 0; blue < 256/nScale; blue++ )
		{
			plot( pImage, xbias+red+(red&1) + blue, ybias+(green + 128/nScale) - red/2 + blue/2 , COLOR );
		}
	}

	for( blue = 0; blue <= 255/nScale; blue++ )
	{
		for( red = 0; red <= 255/nScale; red++ )
		{
			plot( pImage, xbias+red + blue+(blue&1), ybias+(green + 128/nScale) - red/2 + blue/2 , COLOR );
		}
   }

   for( ; green <= 255/nScale; green++ )
   {
		blue = 255/nScale;
		for( red = 0; red <= 255/nScale; red++ )
		{
			plot( pImage, xbias+red + blue+(blue&1), ybias+(green + 128/nScale) - red/2 + blue/2 , COLOR );
		}
		red = 0;
		for( blue = 0; blue < 256/nScale; blue++ )
		{
			plot( pImage, xbias+red + blue, ybias+(green + 128/nScale) - red/2 + blue/2 , COLOR );
		}
   }
}

//----------------------------------------------------------------------------

static CDATA ScaleColor( CDATA original, CDATA new_color, int max, int cur )
{
	int orig_r = RedVal( original );
	int orig_g = GreenVal( original );
	int orig_b = BlueVal( original );
   int nr = RedVal( new_color );
	int ng = GreenVal( new_color );
	int nb = BlueVal( new_color );
	orig_r *= max-cur;
	orig_r /= max;
	orig_g *= max-cur;
	orig_g /= max;
	orig_b *= max-cur;
	orig_b /= max;

	nr *= cur;
	nr /= max;
	ng *= cur;
	ng /= max;
	nb *= cur;
	nb /= max;
   
   orig_r += nr; 
   if( orig_r > 255 ) orig_r = 255;
   orig_g += ng; 
   if( orig_g > 255 ) orig_g = 255;
   orig_b += nb; 
   if( orig_b > 255 ) orig_b = 255;
   return Color( orig_r, orig_g, orig_b );
}
//----------------------------------------------------------------------------

CUSTOM_CONTROL_DRAW( DrawShadeControl, ( PCONTROL pcShade ) )
{
	ValidatedControlData( PSHADE_WELL, shade_well.TypeID, psw, pcShade );
	_32 black;
	_32 width;
	_32 height;
	Image pSurface;
	if( psw )
	{
		pSurface = GetControlSurface( pcShade );
		width = pSurface->width;
		height = pSurface->height;
		//lprintf( WIDE("----------- DRAW SHADE CONTORL ------------------- %08x %08x %08x")
		//		 , psw->color_min
		//		 , psw->color_mid
		//		 , psw->color_max );
		for( black = 0; black < height/2; black++ )
		{
			do_hline( pSurface, black, 0, width-1, ScaleColor( psw->color_min, psw->color_mid, height/2, black ) );
		}
		for( black = height/2; black < height; black++ )
		{
			do_hline( pSurface, black, 0, width-1, ScaleColor( psw->color_mid, psw->color_max, height/2, black - height/2 ) );
		}
		return TRUE;
	}
   return FALSE;
}

//----------------------------------------------------------------------------

#define PPCD(pc) (PPICK_COLOR_DATA)GetCommonUserData( GetFrame( pc ) )

//----------------------------------------------------------------------------

CUSTOM_CONTROL_MOUSE( PaletteMouse, ( PSI_CONTROL pc, S_32 x, S_32 y, _32 buttons ) )
{
	PPICK_COLOR_DATA ppcd = PPCD(pc);
	if( buttons == -1 )
		return FALSE;
	if( buttons & MK_LBUTTON )
	{
		if( ppcd->psw )
		{
			//lprintf( WIDE("Setting new mid color... update... ------------------------- ") );
			ppcd->CurrentColor = SetAlpha( getpixel( GetControlSurface( pc ), x, y )
												  , ppcd->Alpha );
			SetShaderControls( ppcd, pc );
			//SetColorWell( ppcd->pcZoom, c );
		}
	}
   return TRUE;
}

//----------------------------------------------------------------------------

int CPROC DrawPalette( PSI_CONTROL pc )
{
	PPICK_COLOR_DATA ppcd = PPCD(pc);
	if( ppcd )
	{
		Image Surface = GetControlSurface( pc );
		UpdateImage( Surface, ppcd->nGreen );
		if( GetCheckState( GetControl( ppcd->frame, CHK_ALPHA) ) )
		{
			ppcd->CurrentColor = SetAlpha( ppcd->CurrentColor, ppcd->Alpha );
			BlatColorAlpha( Surface, 0, 0
							  , Surface->width
							  , Surface->height
							  , ppcd->CurrentColor );
		}
	}
   return TRUE;
}

//----------------------------------------------------------------------------

SLIDER_UPDATE( SetGreenLevel, (PTRSZVAL psv, PCONTROL pc, int val) )
{
	//PPALETTE_CONTROL
   CDATA c;
	PPICK_COLOR_DATA ppcd  = PPCD(pc);
	if( GetCheckState( GetControl( ppcd->frame, CHK_ALPHA ) ) )
	{
		c = SetAlpha( ppcd->CurrentColor, val );
		ppcd->Alpha = val;
	}
	else
	{
		//c = SetGreen( ppcd->CurrentColor, val );
		ppcd->nGreen = val;
	}
	//if( !ppcd->flags.bSettingShade )
	//{
	//	SetShadeMid( ppcd->psw, c );
	//	SetColorWell( ppcd->pcZoom, c );
	//}
   SmudgeCommon( GetControl( ppcd->frame, PAL_COLORS ) );
}

//----------------------------------------------------------------------------

static void LoadPresets( PPICK_COLOR_DATA ppcd )
{
	FILE *file;
	Fopen( file, WIDE("Palette.Presets"), WIDE("rt") );
	if( file )
	{
		int i;
		char buf[64];
		for( i = 0; i < 36 && fgets( buf, 64, file ); i++ )
		{
			int red, green, blue, alpha;
#ifdef __cplusplus_cli
			//char *tmp = CStrDup( buf );
#define SCANBUF buf
#else
#define SCANBUF buf
#endif
			if( sscanf( SCANBUF, cWIDE("%d,%d,%d,%d\n"), &red, &green, &blue, &alpha ) == 4 )
			{
				ppcd->Presets[i] = AColor( red, green, blue, alpha );
			}
			else
			{
            ppcd->Presets[i] = 0;
			}
		}
		fclose( file );
	}
	else
	{
		int i;
		for( i = 0; i < 36; i++ )
		{
			ppcd->Presets[i] = 0;
		}
	}
}

//----------------------------------------------------------------------------

static void SavePresets( PPICK_COLOR_DATA ppcd )
{
	FILE *file;
	Fopen( file, WIDE("Palette.Presets"), WIDE("wt") );
	if( file )
	{
		int i;
		for( i = 0; i < 36; i++ )
		{
			int red = RedVal( ppcd->Presets[i] )
			  , green = GreenVal( ppcd->Presets[i] )
			  , blue = BlueVal( ppcd->Presets[i] )
			  , alpha = AlphaVal( ppcd->Presets[i] );
#ifdef UNICODE
			fwprintf
#else
			fprintf
#endif
				( file, WIDE("%d,%d,%d,%d\n"), red, green, blue, alpha );
		}
		fclose( file );
	}
}

//----------------------------------------------------------------------------

BUTTON_CLICK( PresetButton, ( PTRSZVAL psv, PCONTROL pc ))
{
	// button was pressed...
	PPICK_COLOR_DATA ppcd = (PPICK_COLOR_DATA)psv;
	int idx = GetControlID(pc) - BTN_PRESET_BASE;
	if( ppcd->LastPreset )
		PressButton( ppcd->LastPreset, FALSE );

	if( ppcd->bSetPreset )
	{
		ppcd->Presets[idx] = ppcd->CurrentColor;
		ppcd->bSetPreset = FALSE;
		SavePresets( ppcd );
	}
	PressButton( pc, TRUE );
	ppcd->CurrentColor = ppcd->Presets[GetControlID(pc) - BTN_PRESET_BASE ];
	ppcd->Alpha = AlphaVal( ppcd->CurrentColor );
	// green level is backwards :(
   ppcd->flags.bSettingShade = 1;
	if( GetCheckState( GetControl( ppcd->frame, CHK_ALPHA ) ) )
		SetSliderValues( GetControl( ppcd->frame, SLD_GREENBAR ), 0, ppcd->Alpha, 255 );
	else
		SetSliderValues( GetControl( ppcd->frame, SLD_GREENBAR ), 0, 255-GreenVal(ppcd->CurrentColor ), 255 );
   SetShaderControls( ppcd, NULL );
   ppcd->flags.bSettingShade = 0;


	SmudgeCommon( GetControl( ppcd->frame, CST_ZOOM ) );
	ppcd->LastPreset = pc;
}

//----------------------------------------------------------------------------

BUTTON_DRAW( PresetDraw, ( PTRSZVAL psv, PCONTROL pc ) )
{
	PPICK_COLOR_DATA ppcd = PPCD(pc);
	if( ppcd )
	{
		CDATA color = ppcd->Presets[GetControlID( pc ) - BTN_PRESET_BASE ];
		Image pSurface = GetControlSurface( pc );
		if( !AlphaVal( color ) )
			color = AColor( 0, 0, 0, 1 );
		ClearImageTo( pSurface, color );
	}
}

//----------------------------------------------------------------------------

BUTTON_CLICK( DefinePreset, ( PTRSZVAL unused, PCONTROL pc ) )
{
	PPICK_COLOR_DATA ppcd = (PPICK_COLOR_DATA)unused;
	// put up a message box... 
	ppcd->bSetPreset = TRUE;
}

//----------------------------------------------------------------------------

BUTTON_CHECK( AlphaPressed, ( PTRSZVAL unused, PCONTROL pc ) )
{
	PPICK_COLOR_DATA ppcd = (PPICK_COLOR_DATA)unused;
	if( GetCheckState( pc ) )
	{
		SetSliderValues( GetControl( ppcd->frame, SLD_GREENBAR ), 0, ppcd->Alpha, 255 );
	}
	else
	{
		SetSliderValues( GetControl( ppcd->frame, SLD_GREENBAR ), 0, ppcd->nGreen, 255 );
	}
}

//----------------------------------------------------------------------------

int CPROC PaletteLoad( PTRSZVAL psv, PSI_CONTROL pf, _32 ID )
{
	// hmm don't think there's really anything special I need to...
	// okay yeah...
   ((PPICK_COLOR_DATA)psv)->frame = pf;
	//SetFrameUserData( pf, psv );
   return TRUE;
}

//----------------------------------------------------------------------------

void CPROC InitColorData( PPICK_COLOR_DATA ppcd, CDATA original )
{
	ppcd->ColorDialogDone = ppcd->ColorDialogOkay = FALSE;
	ppcd->Alpha= AlphaVal( original );
   ppcd->nGreen = GreenVal( original );
	ppcd->CurrentColor = original;
	ppcd->LastPreset = NULL;
	ppcd->bSetPreset = FALSE;
	LoadPresets( ppcd );

}

void CPROC InitColorDataDefault( POINTER p )
{
	PPICK_COLOR_DATA ppcd = (PPICK_COLOR_DATA)p;
   InitColorData( ppcd, Color( 127, 127, 127 ) );
}
//----------------------------------------------------------------------------

void SetShaderControls( PPICK_COLOR_DATA ppcd, PSI_CONTROL source )
{
	if( source != ppcd->pShadeRed )
	{
      SetShadeMin( ppcd->pShadeRed, Color(   0, GreenVal(ppcd->CurrentColor), BlueVal( ppcd->CurrentColor ) ) );
		SetShadeMax( ppcd->pShadeRed, Color( 255, GreenVal(ppcd->CurrentColor), BlueVal( ppcd->CurrentColor ) ) );
		SetShadeMid( ppcd->pShadeRed, Color( 127, GreenVal(ppcd->CurrentColor), BlueVal( ppcd->CurrentColor ) ) );
	}
	if( source != ppcd->pShadeGreen )
	{
      SetShadeMin( ppcd->pShadeGreen, Color( RedVal(ppcd->CurrentColor),   0 , BlueVal( ppcd->CurrentColor ) ));
		SetShadeMax( ppcd->pShadeGreen, Color( RedVal(ppcd->CurrentColor), 255 , BlueVal( ppcd->CurrentColor ) ));
		SetShadeMid( ppcd->pShadeGreen, Color( RedVal(ppcd->CurrentColor), 127 , BlueVal( ppcd->CurrentColor ) ));
	}
	if( source != ppcd->pShadeBlue )
	{
      SetShadeMin( ppcd->pShadeBlue, Color( RedVal(ppcd->CurrentColor), GreenVal( ppcd->CurrentColor ),   0 ) );
		SetShadeMax( ppcd->pShadeBlue, Color( RedVal(ppcd->CurrentColor), GreenVal( ppcd->CurrentColor ), 255 ) );
		SetShadeMid( ppcd->pShadeBlue, Color( RedVal(ppcd->CurrentColor), GreenVal( ppcd->CurrentColor ), 127 ) );
	}
   if( source != ppcd->psw )
		SetShadeMid( ppcd->psw, ppcd->CurrentColor );

	SetColorWell( ppcd->pcZoom, ppcd->CurrentColor );
}

//----------------------------------------------------------------------------

// LOL that's pretty sexy, huh? LOL
static TEXTCHAR palette_frame_xml[] = {
//#define stuff(...) #__VA_ARGS__
//stuff(
#include "palette.frame"
//    )
 };

PSI_PROC( int, PickColorEx )( CDATA *result, CDATA original, PSI_CONTROL hAbove, int x, int y )
{
	PSI_CONTROL pf = NULL;
	PICK_COLOR_DATA pcd;
	MemSet( &pcd, 0, sizeof( pcd ) );
	InitColorData( &pcd, original );
	// remove test for debugging save/load..
	pf = ParseXMLFrame( palette_frame_xml, sizeof( palette_frame_xml ) );
	if( !pf )
		pf = LoadXMLFrame( WIDE("palette.frame") /*, NULL, PaletteLoad, (PTRSZVAL)&pcd*/ );;
	if( pf )
	{
		int i;
		pcd.frame = pf;
		SetFrameUserData( pf, (PTRSZVAL)&pcd );
		SetSliderValues( GetControl( pf, SLD_GREENBAR ), 0, 255-GreenVal( pcd.CurrentColor ), 255 );
		pcd.pcZoom =  GetControl( pf, CST_ZOOM );
		pcd.psw = GetControl( pf, CST_SHADE );
		pcd.pShadeRed = GetControl( pf, CST_SHADE_RED );
		pcd.pShadeGreen = GetControl( pf, CST_SHADE_GREEN );
		pcd.pShadeBlue = GetControl( pf, CST_SHADE_BLUE );
		SetButtonPushMethod( GetControl( pf, BTN_PRESET ), DefinePreset, (PTRSZVAL)&pcd );
		SetCheckButtonHandler( GetControl( pf, CHK_ALPHA ), AlphaPressed, (PTRSZVAL)&pcd );
		for( i = 0; i < 36; i++ )
		{
			PSI_CONTROL button = GetControl( pf, BTN_PRESET_BASE + i );
			if( button )
			{
				SetButtonDrawMethod( button, PresetDraw, (PTRSZVAL)&pcd );
				SetButtonPushMethod( button, PresetButton, (PTRSZVAL)&pcd );
			}
		}
		SetSliderUpdateHandler( GetControl( pf, SLD_GREENBAR ), SetGreenLevel, (PTRSZVAL)&pcd );
		SetCommonButtons( pf, &pcd.ColorDialogDone, &pcd.ColorDialogOkay );
		SetShaderControls( &pcd, NULL );
		SetSliderValues( GetControl( pf, SLD_GREENBAR ), 0, 255-GreenVal( pcd.CurrentColor ), 255 );
	}

#define SHADER_PAD 3
#define SHADER_WIDTH 15
#define FRAME_WIDTH 226 + ( 3 * ( SHADER_WIDTH + SHADER_PAD ) )
#define FRAME_HEIGHT 259
	if( !pf )
	{
		PSI_CONTROL pc;
		//DumpRegisteredNames();
		pf = CreateFrame( WIDE("Color Select")
							 , x - FRAME_WIDTH/2, y - FRAME_HEIGHT/2
							 , FRAME_WIDTH, FRAME_HEIGHT, BORDER_NORMAL, NULL );
		if( !pf )
			return FALSE;
		pcd.frame = pf;
		SetFrameUserData( pf, (PTRSZVAL)&pcd );
		//MoveFrame( pf, x - FRAME_WIDTH/2, y - FRAME_HEIGHT/2 );
		pc = MakeNamedControl( pf, WIDE("Color Matrix")
									, 5, 5, xsize, ysize
									, PAL_COLORS );

		MakeSlider( pf
					 , 5 + xsize + 3, 1
					 , 18, ysize + 6
					 , SLD_GREENBAR, SLIDER_VERT, SetGreenLevel, (PTRSZVAL)&pcd );

//	MakeTextControl( pf, TEXT_VERTICAL, 8 + xsize + 15, 20, 88, 12, TXT_STATIC, WIDE("Green Level") );
		pcd.psw = MakeNamedControl( pf, WIDE("Shade Well"), 8 + xsize + 18 + 12, 5
												 , SHADER_WIDTH, ysize
												 , CST_SHADE );

		pcd.pShadeRed = MakeNamedControl( pf, WIDE("Shade Well"), 8 + xsize + 18 + 12 + (SHADER_WIDTH + SHADER_PAD), 5
												  , SHADER_WIDTH, ysize
												  , CST_SHADE_RED );

		pcd.pShadeBlue = MakeNamedControl( pf, WIDE("Shade Well"), 8 + xsize + 18 + 12 + (SHADER_WIDTH + SHADER_PAD)*2, 5
													, SHADER_WIDTH, ysize
													, CST_SHADE_BLUE );
		pcd.pShadeGreen = MakeNamedControl( pf, WIDE("Shade Well"), 8 + xsize + 18 + 12 + (SHADER_WIDTH + SHADER_PAD)*3, 5
													 , SHADER_WIDTH, ysize
													 , CST_SHADE_GREEN );

		pcd.pcZoom = MakeNamedControl( pf, WIDE("Color Well")
											  , 8 + xsize + 18 + 12 + (SHADER_WIDTH+SHADER_PAD)*4, 5
											  , 2 * SHADER_WIDTH, ysize
											  , CST_ZOOM
											  );

		MakeTextControl( pf, 5, ysize + 14, 150, 12, TXT_STATIC, WIDE("User-Defined Colors"), 0 );
		{
			int i;
			PCONTROL pc;
			for( i = 0; i < 12; i++ )
			{
				pc = MakeCustomDrawnButton( pf, 5 + 18 * i, ysize + 15 + 13     , 16, 16, BTN_PRESET_BASE+i, 0, PresetDraw, PresetButton, (PTRSZVAL)&pcd );
				SetCommonBorder( pc, BORDER_THINNER );
				SetNoFocus( pc );
			}
			for( i = 0; i < 12; i++ )
			{
				pc = MakeCustomDrawnButton( pf, 5 + 18 * i, ysize + 15 + 13 + 18, 16, 16, BTN_PRESET_BASE+i+12, 0, PresetDraw, PresetButton, (PTRSZVAL)&pcd );
				SetCommonBorder( pc, BORDER_THINNER );
				SetNoFocus( pc );
			}
			for( i = 0; i < 12; i++ )
			{
				pc = MakeCustomDrawnButton( pf, 5 + 18 * i, ysize + 15 + 13 + 36, 16, 16, BTN_PRESET_BASE+i+24, 0, PresetDraw, PresetButton, (PTRSZVAL)&pcd );
				SetCommonBorder( pc, BORDER_THINNER );
				SetNoFocus( pc );
			}
		}
      // button style normal button
		MakeCheckButton( pf, 5, 235, 95, 19
							, BTN_PRESET, WIDE("Set Preset")
							, 0, DefinePreset, (PTRSZVAL)&pcd );
		MakeCheckButton( pf, 5, 218, 95, 14
							, CHK_ALPHA, WIDE("Set Alpha")
							, 0, AlphaPressed, (PTRSZVAL)&pcd );

		AddCommonButtons( pf, &pcd.ColorDialogDone, &pcd.ColorDialogOkay );
		SetShaderControls( &pcd, NULL );
		SetSliderValues( GetControl( pf, SLD_GREENBAR ), 0, 255-GreenVal( pcd.CurrentColor ), 255 );
		//SetShadeMid( pcd.psw, original );
		//SetColorWell( pcd.pcZoom, original );

//#ifndef WIN32
		SaveXMLFrame( pf, WIDE("palette.frame") );
//#endif
	}
	DisplayFrameOver( pf, hAbove );
	EditFrame( pf, TRUE );
	//EditFrame( pf, TRUE );
	CommonWait( pf );
	//CommonLoop( &pcd.ColorDialogDone, &pcd.ColorDialogOkay );
	if( pcd.ColorDialogOkay )
	{
		//SaveFrame( pf, WIDE("palette.frame") );
		if( result )
			*result = pcd.CurrentColor;
		lprintf( WIDE("------- Destroy common.") );
		DestroyCommon( &pf );
	  	return TRUE;
	}
	lprintf( WIDE("------- Destroy common.") );
	DestroyCommon( &pf );
	lprintf( WIDE("Result to application.") );
	return FALSE;
}

//----------------------------------------------------------------------------

PSI_PROC( int, PickColor )( CDATA *result, CDATA original, PSI_CONTROL hAbove )
{
	S_32 x, y;
   GetMousePosition( &x, &y );
	return PickColorEx( result, original, hAbove, x, y );
}

//----------------------------------------------------------------------------

static int CPROC ColorWellDraw( PCONTROL pc )
{
	ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
	if( pcw )
	{
		Image surface = GetControlSurface( pc );
		CDATA color = pcw->color;
		if( !AlphaVal( color ) )
			color = SetAlpha( color, 1 );
		//lprintf( WIDE("Clear color well surface to %lX"), pcw->color );
		ClearImageTo( surface, color );
	}
   return TRUE;
}

static int CPROC ColorWellMouse( PSI_CONTROL pc, S_32 x, S_32 y, _32 b )
{
   ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
	//if( pc->flags.bDisable ) // ignore mouse on these...
	//	return FALSE;
	if( b == -1 )
	{
		return FALSE;
	}
	if( pcw->flags.bPickColor )
	{
		if( b & ( MK_LBUTTON | MK_RBUTTON ) )
		{
			CDATA result = pcw->color;
			if( !pcw->flags.bPicking )
			{
				pcw->flags.bPicking = 1;
            lprintf( WIDE("PICK_COLOR") );
				if( PickColorEx( &result, pcw->color, GetFrame( pc ), x + FRAME_WIDTH, y + FRAME_WIDTH ) )
				{
					lprintf( WIDE("PICK_COLOR_DONE") );
               lprintf( WIDE("Updating my color to %08")_32fx WIDE(""), result );
					pcw->color = result;
					if( pcw->UpdateProc )
                  pcw->UpdateProc( pcw->psvUpdate, result );
					SmudgeCommon( pc );
				}
				else
				{
					lprintf( WIDE("Failing to set the color..") );
					lprintf( WIDE("PICK_COLOR_DONE2") );
				}
            //DebugBreak();
				pcw->flags.bPicking = 0;
			}
		}
	}
   return TRUE;
}

PSI_CONTROL EnableColorWellPick( PSI_CONTROL pc, LOGICAL bEnable )
{
	ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
	if( pcw )
		pcw->flags.bPickColor = bEnable;
   return pc;
}

PSI_CONTROL SetOnUpdateColorWell( PSI_CONTROL pc, void(CPROC*update_proc)(PTRSZVAL,CDATA), PTRSZVAL psvUpdate)
{
	ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
	if( pcw )
	{
		pcw->UpdateProc = update_proc;
		pcw->psvUpdate = psvUpdate;
	}
   return pc;
}

//----------------------------------------------------------------------------

static int CPROC ShadeWellMouse( PSI_CONTROL pc, S_32 x, S_32 y, _32 b )
{
	ValidatedControlData( PSHADE_WELL, shade_well.TypeID, psw, pc );
	if( b == -1 )
      return FALSE;
	if( psw && ( b & MK_LBUTTON ) )
	{
		PPICK_COLOR_DATA ppcd = PPCD(pc);
		CDATA c;
		lprintf( WIDE("Setting new mid color... update... ------------------------- ") );
		SetAlpha( c = getpixel( GetControlSurface( pc ), x, y )
				  , ppcd->Alpha );
		//SetColorWell( ppcd->pcZoom, c );
		ppcd->CurrentColor = c;
		ppcd->flags.bSettingShade = 1;
      SetShaderControls( ppcd, pc );
		SetSliderValues( GetNearControl( pc, SLD_GREENBAR ), 0, 255-GreenVal( c ), 255 );
      ppcd->flags.bSettingShade = 0;
		// update ppcd ... how do I get that?
		// it's attached to the frame, right?  so I just
		// have to indicicate that ppcd has changed...
		SmudgeCommon( pc );
	}
   return TRUE;
}

//----------------------------------------------------------------------------

PSI_PROC( void, SetShadeMin )( PSI_CONTROL pc, CDATA color )
{
	ValidatedControlData( PSHADE_WELL, shade_well.TypeID, psw, pc );
	if( psw )
	{
		psw->color_min = color;
      SmudgeCommon( pc );
	}
}

//----------------------------------------------------------------------------

PSI_PROC( void, SetShadeMax )( PSI_CONTROL pc, CDATA color )
{
	ValidatedControlData( PSHADE_WELL, shade_well.TypeID, psw, pc );
	if( psw )
	{
		psw->color_max = color;
      SmudgeCommon( pc );
	}
}

//----------------------------------------------------------------------------

PSI_PROC( void, SetShadeMid )( PSI_CONTROL pc, CDATA color )
{
	ValidatedControlData( PSHADE_WELL, shade_well.TypeID, psw, pc );
	if( psw )
	{
		psw->color_mid = color;
      SmudgeCommon( pc );
	}
}

//----------------------------------------------------------------------------
PSI_PROC( CDATA, GetColorFromWell )( PSI_CONTROL pc )
{
	ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
   if( pcw )
		return pcw->color;
   return 0;
}

//----------------------------------------------------------------------------

PSI_PROC( PSI_CONTROL, SetColorWell )( PSI_CONTROL pc, CDATA color )
{
	ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
	if( pcw )
	{
		pcw->color = color;
		SmudgeCommon( pc );
	}
   return pc;
}
//----------------------------------------------------------------------------

//CONTROL_PROC_DEF( COLOR_WELL, ColorWell, BORDER_THIN|BORDER_INVERT
//					 , CDATA color )
int CPROC InitColorWell( PSI_CONTROL pc )
{
	if( pc )
	{
		ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
      pcw->color = 0;
	}
	return TRUE;
}

//----------------------------------------------------------------------------

int CPROC InitShadeWell( PSI_CONTROL pc )
{
	SetShadeMin( pc, BASE_COLOR_BLACK );
	SetShadeMid( pc, BASE_COLOR_DARKGREY );
	SetShadeMax( pc, BASE_COLOR_WHITE );
	SetNoFocus( pc );
	return TRUE;
}

//----------------------------------------------------------------------------

int CPROC InitPalette( PSI_CONTROL pc )
{
   SetNoFocus( pc );
   return TRUE;
}

void CPROC ColorWellDestroy( PSI_CONTROL pc )
{
	ValidatedControlData( PCOLOR_WELL, color_well.TypeID, pcw, pc );
	if( pcw )
	{
		if( pcw->flags.bPicking )
		{
			lprintf( WIDE("Uhmm need to kill the parent color picking dialog!") );
		}

	}
}

//----------------------------------------------------------------------------

CONTROL_REGISTRATION color_well = { WIDE("Color Well")
											 , { {32, 32}, sizeof( COLOR_WELL ), BORDER_INVERT|BORDER_THIN }
											 , InitColorWell
											 , NULL
											 , ColorWellDraw
											 , ColorWellMouse
											 , NULL // key
                                  , ColorWellDestroy // if picking a color, destroy dialog
},
shade_well = { WIDE("Shade Well")
				 , { {32, 32}, sizeof( SHADE_WELL ), BORDER_INVERT|BORDER_THIN }
											 , InitShadeWell
											 , NULL
											 , DrawShadeControl
											 , ShadeWellMouse
											 , NULL
},
color_matrix_well = { WIDE("Color Matrix")
						  , { {xsize, ysize}, 0, BORDER_INVERT|BORDER_THIN }
											 , InitPalette
											 , NULL
											 , DrawPalette
											 , PaletteMouse
											 , NULL

}
;
PRIORITY_PRELOAD( register_well, PSI_PRELOAD_PRIORITY )
{
   DoRegisterControl( &color_well );
   DoRegisterControl( &shade_well );
   DoRegisterControl( &color_matrix_well );
}
PSI_COLORWELL_NAMESPACE_END

//----------------------------------------------------------------------------
// $Log: palette.c,v $
// Revision 1.75  2005/06/20 17:48:03  jim
// Disable/fix some of the load/save code
//
// Revision 1.74  2005/06/20 17:34:14  jim
// Fix XML saving
//
// Revision 1.73  2005/06/03 22:39:12  jim
// Add support for gtframepos and getframe size...
//
// Revision 1.72  2005/06/03 21:39:14  jim
// Added support to make save/load almost work.
//
// Revision 1.71  2005/03/30 03:26:37  panther
// Checkpoint on stabilizing display projects, and the exiting thereof
//
// Revision 1.70  2005/03/21 20:41:35  panther
// Protect against super large fonts, remove edit frame from palette, and clean up some warnings.
//
// Revision 1.69  2005/03/12 23:31:21  panther
// Edit controls nearly works... have some issues with those dang popups.
//
// Revision 1.68  2005/02/18 19:42:38  panther
// fix some update issues with hiding and revealing controls/frames... minor fixes for new API changes
//
// Revision 1.67  2005/02/10 16:55:51  panther
// Fixing warnings...
//
// Revision 1.66  2005/02/09 21:23:44  panther
// Update macros and function definitions to follow the common MakeControl parameter ordering.
//
// Revision 1.65  2005/01/28 00:14:43  bjerke
// fix palette.
//
// Revision 1.64  2004/12/05 10:48:21  panther
// Fix palette result color.  Fix check button for set alpha.  To make a radio/check button, 4 parameters, not two are needed.  Edit controls respond to mouse better.  Copy and paste function returned, no right click on edit fields :(
//
// Revision 1.63  2004/12/05 00:22:44  panther
// Fix focus flag reference and edit controls begin to work.  Sheet controls are still flaky
//
// Revision 1.62  2004/11/29 07:00:32  panther
// Makesystem fixes to build with minimal config under linux.
//
// Revision 1.61  2004/10/31 17:22:28  d3x0r
// Minor fixes to control library...
//
// Revision 1.60  2004/10/24 20:09:47  d3x0r
// Sync to psilib2... stable enough to call it mainstream.
//
// Revision 1.13  2004/10/22 09:23:32  d3x0r
// Caption scaling correct.... all is going more well... need to test the verification thing tomowrrow...
//
// Revision 1.12  2004/10/21 16:45:51  d3x0r
// Updaes to dialog handling... still ahve  aproblem with caption resize
//
// Revision 1.11  2004/10/11 08:45:10  d3x0r
// checkpoint.
//
// Revision 1.10  2004/10/10 11:57:12  d3x0r
// Okay looks like cleanup time... tie up some loose ends and all should be well.
//
// Revision 1.9  2004/10/10 09:07:58  d3x0r
// Short a couple frames on some of the updates... but looks like progress is positive.
//
// Revision 1.8  2004/10/08 13:07:43  d3x0r
// Okay beginning to look a lot like PRO-GRESS
//
// Revision 1.7  2004/10/07 18:34:18  d3x0r
// checkponit
//
// Revision 1.6  2004/10/07 06:08:46  d3x0r
// okay restore some functions to be backwards compat - there are more I've deleted but let's start here
//
// Revision 1.5  2004/10/07 04:37:16  d3x0r
// Okay palette and listbox seem to nearly work... controls draw, now about that mouse... looks like my prior way of cheating is harder to step away from than I thought.
//
// Revision 1.4  2004/10/06 09:52:16  d3x0r
// checkpoint... total conversion... now how does it work?
//
// Revision 1.3  2004/10/05 00:20:29  d3x0r
// Break out these rather meaty parts from controls.c
//
// Revision 1.2  2004/09/27 20:44:28  d3x0r
// Sweeping changes only a couple modules left...
//
// Revision 1.1  2004/09/19 19:22:31  d3x0r
// Begin version 2 psilib...
//
// Revision 1.55  2004/09/17 16:18:29  d3x0r
// ...
//
// Revision 1.54  2004/09/09 00:54:20  d3x0r
// Compiles...
//
// Revision 1.53  2004/09/07 01:13:01  d3x0r
// Checkpoint - really really tempting to break all existing code....
//
// Revision 1.52  2004/09/06 23:38:56  d3x0r
// This control thing... I think I took a wrong turn somewhere there...
//
// Revision 1.51  2004/08/25 15:01:06  d3x0r
// Checkpoint - more vc compat fixes
//
// Revision 1.50  2004/08/24 17:18:00  d3x0r
// Fix last couple c files for new control_proc_def macro
//
// Revision 1.49  2004/08/24 11:15:15  d3x0r
// Checkpoint Visual studio mods.
//
// Revision 1.48  2004/06/16 10:27:36  d3x0r
// Added key events to display library...
//
// Revision 1.47  2004/06/15 19:34:47  d3x0r
// Define additional color controls
//
// Revision 1.46  2004/05/24 23:23:01  d3x0r
// Checkpoint.
//
// Revision 1.45  2004/05/24 21:05:54  d3x0r
// Checkpoint - all builds.
//
// Revision 1.44  2004/05/24 16:03:36  d3x0r
// Begin using registered data type
//
// Revision 1.43  2004/05/23 09:50:44  d3x0r
// Updates to extend dynamic edit dialogs.
//
// Revision 1.42  2004/05/21 07:23:42  d3x0r
// Stupid things to make controls actually init and work.
//
// Revision 1.41  2004/03/23 22:58:33  d3x0r
// Oops minor fix after removing the palette-frame loading code.
//
// Revision 1.40  2004/03/23 17:02:44  d3x0r
// Use common interface library to load video/image interface
//
// Revision 1.39  2004/01/29 02:30:50  d3x0r
// Quick Merge mod... better usage of freetype-config
//
// Revision 1.38  2004/01/16 08:52:50  panther
// move frame/control save/load external - working on properties, editing
//
// Revision 1.37  2003/11/29 00:10:28  panther
// Minor fixes for typecast equation
//
// Revision 1.36  2003/09/18 12:14:49  panther
// MergeRectangle Added.  Seems Control edit near done (fixing move/size errors)
//
// Revision 1.35  2003/09/15 01:02:25  panther
// Well most of the work recovered... but still need partial update
//
// Revision 1.34  2003/09/13 17:06:29  panther
// Okay - and now we use stdargs... ugly kinda but okay...
//
// Revision 1.33  2003/09/13 11:33:42  panther
// Checkpoint dialog edit
//
// Revision 1.32  2003/09/12 11:32:41  panther
// Okay first dialog to save and load works (mostly) some redraw issues exist though...
//
// Revision 1.31  2003/09/11 13:09:25  panther
// Looks like we maintained integrety while overhauling the Make/Create/Init/Config interface for controls
//
// Revision 1.30  2003/07/24 23:47:22  panther
// 3rd pass visit of CPROC(cdecl) updates for callbacks/interfaces
//
// Revision 1.29  2003/05/02 01:11:26  panther
// Many minor fixes, tweaks....
//
// Revision 1.28  2003/05/01 21:31:57  panther
// Cleaned up from having moved several methods into frame/control common space
//
// Revision 1.27  2003/05/01 19:18:15  panther
// broken - but will fix
//
// Revision 1.26  2003/04/30 16:12:05  panther
// Fix button def
//
// Revision 1.25  2003/04/10 14:24:12  panther
// Fixed palette control problems
//
// Revision 1.24  2003/04/10 14:08:53  panther
// Fix default palette.presets
//
// Revision 1.23  2003/03/30 19:40:14  panther
// Encapsulate pick color data better.
//
// Revision 1.22  2003/03/25 08:45:57  panther
// Added CVS logging tag
//
