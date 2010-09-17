#define USES_MILK_INTERFACE
#define DEFINES_MILK_INTERFACE
#include <milk_export.h>
#include <milk_button.h>
#include <milk_registry.h>
#include <sqlstub.h>
#include <filesys.h>
#include <widgets/banner.h>

#ifdef __WINDOWS__
#define MEDIA_ROOT_PATH "c:/ftn3000/etc/images/"
#else
#define MEDIA_ROOT_PATH "/storage/media/"
#endif

//TXT_MEDIA_NAME
enum{  TXT_NEW_MEDIA_PATH = 4510
	 , TXT_NEW_MEDIA_NAME
};

PRELOAD( InitControllerImportMedia )
{
	EasyRegisterResource( "Video Link/Controller Import Media" , TXT_NEW_MEDIA_NAME , EDIT_FIELD_NAME );
	EasyRegisterResource( "Video Link/Controller Import Media" , TXT_NEW_MEDIA_PATH , EDIT_FIELD_NAME );
}

typedef struct button_info_for_import_media
{
	struct {
		_32 bOn:1;
	} flags;
	PMENU_BUTTON button;
	PVARIABLE label_var;
	CTEXTSTR label_str;
   PLIST listMyTitles;
} BUTTON_IMPORT_MEDIA_INFO, *PBUTTON_IMPORT_MEDIA_INFO;

_32 MeasureFile ( CTEXTSTR which )
{
	int retval = -1;
	FILE * pFile;

	pFile = fopen( which, "rb" );
	if( pFile )
	{
		fseek( pFile, 0 , SEEK_END );
		retval = ftell( pFile );
		rewind( pFile );
      fclose( pFile );
	}
	return ( ( retval > 0 )?( (_32)retval ):( 0 ) ) ;
}
LOGICAL PlopBlob( CTEXTSTR path , CTEXTSTR name )
{
   LOGICAL retval = FALSE;
   TEXTSTR pBlob;
	_32 size = 0;
   int measurement = -1;
	POINTER p;

	if( ( measurement = MeasureFile( path ) ) > 0 )
	{
		p = OpenSpace( NULL , path , &size );
		lprintf( "name is %s path is %s size is %lu measurement is %d" , name, path , size, measurement);
//		pBlob = EscapeBinary( p ,  size );
		pBlob = EscapeBinary( p ,  measurement );
		{
			char buf[64];
         snprintf( buf, sizeof(buf), "Please wait for %s", name);
			BannerMessage(buf);
		}
		if( DoSQLCommandf( "INSERT INTO media ( media_name, content ) VALUES ( '%s', '%s' ) "
							  , name
							  , pBlob
							  ) )
		{
			BannerMessage( "Done");
			retval = TRUE;
		}
		else
		{
			CTEXTSTR errorresult;
			char * buf = Allocate(63);
			char * p;
			MemSet( buf, 0, 63 );
			lprintf("OOps.  What happened?  Something went wrong");
			GetSQLError( &errorresult );
			p = strchr( errorresult, '\'' );
			MemCpy( buf, p, 62 );
			lprintf("Looks like a SQL error of: %s", buf );
			BannerNoWait( buf );
			retval = FALSE;
		}

		Release( pBlob );
		Release( p );
	}
	else
	{
      char buf[64];
		lprintf("For some reason, %s could not be measured.  Zero length?", path );
      snprintf(buf, sizeof(buf),"%s not found", path);
      BannerMessage( buf );
	}

   return retval;
}

OnKeyPressEvent(WIDE( "Video Link/Controller Import Media" ) )( PTRSZVAL psv )
{
	PBUTTON_IMPORT_MEDIA_INFO pButtonInfo = ( PBUTTON_IMPORT_MEDIA_INFO  )psv;
	TEXTCHAR result[256];
	PSI_CONTROL parent = GetFrame( pButtonInfo->button );

	MemSet( result, 0 , 254 );
	if( PSI_PickFile ( parent , MEDIA_ROOT_PATH , /*"*.ISO"*/NULL , result, ( sizeof(result) ), FALSE ) )
	{
		if( result )
		{
			CTEXTSTR name = ( pathrchr( result ) + 1 );
         lprintf("Calling PlopBlob with %s %s", result, name );
			if( PlopBlob ( result, name ) )
			{
            lprintf("PlopBlob'd for %s", name );
			}
			else
			{
            lprintf("Could not PlopBlob for %s", name );
			}
		}
	}
}
OnCreateMenuButton( WIDE( "Video Link/Controller Import Media" ) )( PMENU_BUTTON button )
{
   PBUTTON_IMPORT_MEDIA_INFO pButtonInfo = Allocate( sizeof( BUTTON_IMPORT_MEDIA_INFO  ) );
//	pButtonInfo->button = button;
	MILK_SetButtonText( pButtonInfo->button = button, "Import_Media");

	return (PTRSZVAL)pButtonInfo;
}

