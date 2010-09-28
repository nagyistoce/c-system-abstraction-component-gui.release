
#ifdef SERVICE_SOURCE
#define SERVICE_EXPORT EXPORT_METHOD
#else
#define SERVICE_EXPORT IMPORT_METHOD
#endif


SERVICE_EXPORT void SetupService( TEXTSTR name, void (CPROC*Start)(void) );

