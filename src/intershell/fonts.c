#include <stdhdrs.h>

#include "intershell_local.h"
#include "resource.h"

#include "widgets/include/banner.h"
#include "fonts.h"

#include <psi.h>
extern CONTROL_REGISTRATION menu_surface;


PRELOAD( RegisterFontConfigurationIDs )
{
	EasyRegisterResource( WIDE( "intershell/font" ), BTN_PICKFONT, NORMAL_BUTTON_NAME );
	EasyRegisterResource( WIDE( "intershell/font" ), BTN_PICKFONT_PRICE, NORMAL_BUTTON_NAME );
	EasyRegisterResource( WIDE( "intershell/font" ), BTN_PICKFONT_QTY, NORMAL_BUTTON_NAME );
	EasyRegisterResource( WIDE( "intershell/font" ), BTN_EDITFONT, NORMAL_BUTTON_NAME );
}

typedef struct font_preset_tag
{
   TEXTCHAR *name;
	Font font;
	POINTER fontdata;
   _32 fontdatalen;
} FONT_PRESET;

typedef struct font_select_tag
{
   // structure for font selection dialog...
	Font font;
	POINTER *fontdata;
   _32 *fontdatalen;
	PFONT_PRESET selected_font;

} FONT_SELECT, *PFONT_SELECT;

static struct local
{
   PLIST fonts;
} l;

PFONT_PRESET _CreateAFont( PCanvasData canvas, CTEXTSTR name, Font font, POINTER data, _32 datalen )
{
	PFONT_PRESET font_preset;
	INDEX idx;
	LIST_FORALL( l.fonts, idx, PFONT_PRESET, font_preset )
	{
		if( stricmp( name, font_preset->name ) == 0 )
         break;
	}
	if( !font_preset )
	{
		font_preset = New( FONT_PRESET );
		font_preset->name = StrDup( name );
		AddLink( &l.fonts, font_preset );
	}
	else
      DestroyFont( &font_preset->font );
	font_preset->fontdata = NewArray( _8, datalen );
	MemCpy( font_preset->fontdata, data, datalen );
	font_preset->fontdatalen = datalen;
	if( font )
		font_preset->font = font;
	else if( font_preset->fontdata )
		font_preset->font = RenderScaledFontData( (struct font_data_tag *)font_preset->fontdata
															 , &canvas->width_scale
															 , &canvas->height_scale );
	else
      font_preset->font = NULL;

   return font_preset;
}


static void CPROC EditPageFont(PTRSZVAL psv, PSI_CONTROL pc )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, g.single_frame );
	PFONT_SELECT font_select = (PFONT_SELECT)psv;
	if( font_select->selected_font )
	{
		POINTER tmp = font_select->selected_font->fontdata;
      _32 tmplen = font_select->selected_font->fontdatalen;
		Font font = PickScaledFont( 0, 0
										  , &canvas->width_scale, &canvas->height_scale
   									  , &tmplen //font_select->fontdata
   									  , &tmp //font_select->fontdatalen
   									  , (PCOMMON)GetFrame(pc) );
		if( font )
		{
         font_select->selected_font->font = font;
         font_select->selected_font->fontdata = tmp;
         font_select->selected_font->fontdatalen = tmplen;
		}

	}
}

Font* CreateAFont( CTEXTSTR name, Font font, POINTER data, _32 datalen )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, g.single_frame );
	PFONT_PRESET preset = _CreateAFont( canvas, name, font, data, datalen );
   if( preset )
		return &preset->font;
   return NULL;
}

void CPROC SetCurrentPreset( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	PFONT_SELECT font_select = (PFONT_SELECT)psv;

	font_select->selected_font = (PFONT_PRESET)GetItemData( pli );
}

static void CPROC CreatePageFont( PTRSZVAL psv, PSI_CONTROL pc )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, g.single_frame );
	PFONT_SELECT font_select = (PFONT_SELECT)psv;
	TEXTCHAR name_buffer[256];
	//if( !font_select->selected_font )
	{
		if( !SimpleUserQuery(  name_buffer, sizeof( name_buffer )
								  , WIDE("Enter new font preset name"), GetFrame( pc ) ) )
			return;
		{
			PFONT_PRESET font_preset;
			INDEX idx;
			LIST_FORALL( l.fonts, idx, PFONT_PRESET, font_preset )
			{
				if( strcmp( font_preset->name, name_buffer ) == 0 )
				{
               BannerMessage( WIDE("Font name already exists") );
					return;
				}
			}
		}
	}
	{
		POINTER tmp = NULL;
      _32 tmplen = 0;
		Font font = PickScaledFont( 0, 0
										  , &canvas->width_scale, &canvas->height_scale
   									  , &tmplen //font_select->fontdata
   									  , &tmp //font_select->fontdatalen
   									  , (PCOMMON)GetFrame(pc) );
		if( font )
		{
			PLISTITEM pli;
         PFONT_PRESET font_preset =  _CreateAFont( canvas, name_buffer
																, font
																, tmp
																, tmplen );
			font_select->fontdata = &font_preset->fontdata;
         font_select->fontdatalen = &font_preset->fontdatalen;
			pli= AddListItem( GetNearControl( pc, LST_FONTS ), font_preset->name );
         SetItemData( pli, (PTRSZVAL)font_preset );
			SetSelectedItem( GetNearControl( pc, LST_FONTS ), pli );
		}
	}
}

Font * UseAFont( CTEXTSTR name )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, g.single_frame );
	PFONT_PRESET font_preset;
	INDEX idx;
	LIST_FORALL( l.fonts, idx, PFONT_PRESET, font_preset )
	{
		if( !strcmp( font_preset->name, name ) )
		{
			if( !font_preset->font )
				font_preset->font = RenderScaledFontData( (struct font_data_tag *)font_preset->fontdata
																	 , &canvas->width_scale
																	 , &canvas->height_scale );
         return &font_preset->font;
		}
	}
   return CreateAFont( name, NULL, NULL, 0 );
//   return NULL;
}

// default name for having already chosen a font preset...
// result is a pointer to the preset's name.
Font *SelectAFont( PSI_CONTROL parent, CTEXTSTR*default_name )
{
	FONT_SELECT font_select;
	PSI_CONTROL frame;
	int okay = 0;
	int done = 0;
   font_select.selected_font = NULL;
	//font_select.fontdata = pfontdata;
   //font_select.fontdatalen = pfontdatalen;
	frame = LoadXMLFrame( WIDE("font_preset_property.isframe") );
	if( frame )
	{

		SetCommonButtons( frame, &done, &okay );
		{
			//could figure out a way to register methods under
			PSI_CONTROL list = GetControl( frame, LST_FONTS );
			SetButtonPushMethod( GetControl( frame, BTN_PICKFONT ), CreatePageFont, (PTRSZVAL)&font_select );
			SetButtonPushMethod( GetControl( frame, BTN_EDITFONT ), EditPageFont, (PTRSZVAL)&font_select );
			if( list )
			{
				PFONT_PRESET font_preset;
				INDEX idx;
				LIST_FORALL( l.fonts, idx, PFONT_PRESET, font_preset )
				{
					PLISTITEM pli = AddListItem( list, font_preset->name );
					SetItemData( pli, (PTRSZVAL)font_preset );
					if( default_name && default_name[0] && stricmp( font_preset->name, default_name[0] )==0 )
					{
						SetSelectedItem( list, pli );
						font_select.selected_font = font_preset;
					}
				}
            SetSelChangeHandler( list, SetCurrentPreset, (PTRSZVAL)&font_select );
			}
		}
		DisplayFrameOver( frame, parent );
      //EditFrame( frame, TRUE );
		CommonWait( frame );
		if( !okay )
		{
         font_select.selected_font = NULL;
		}
		DestroyFrame( &frame );
		if( font_select.selected_font )
		{
			if( default_name )
			{
				if( default_name[0] )
					Release( (POINTER)default_name[0] );
				default_name[0] = StrDup( font_select.selected_font->name );
			}
			return &font_select.selected_font->font;
		}
	}
   return NULL;
}

OnSaveCommon( WIDE( "Common Fonts" ) )( FILE *out )
{
   PFONT_PRESET preset;
	INDEX idx;
	LIST_FORALL( l.fonts, idx, PFONT_PRESET, preset )
	{
      TEXTCHAR *data;
      if( preset->fontdata && preset->fontdatalen )
      {
			EncodeBinaryConfig( &data, preset->fontdata, preset->fontdatalen );
			fprintf( out, WIDE("font preset %s=%s\n")
					 , preset->name
					 , data );
			Release( data );
		}
		else
			fprintf( out, WIDE("font preset %s={}\n"), preset->name );
	}
}

static PTRSZVAL CPROC RecreateFont( PTRSZVAL psv, arg_list args )
{
	PARAM( args, TEXTCHAR *, name );
	PARAM( args, _32, length );
	PARAM( args, POINTER, data );
	CreateAFont( name, NULL, data, length );
	return 0;
}

OnLoadCommon( WIDE( "Common Fonts" ) )( PCONFIG_HANDLER pch )
{
	AddConfigurationMethod( pch, WIDE("font preset %m=%B"), RecreateFont );
}

