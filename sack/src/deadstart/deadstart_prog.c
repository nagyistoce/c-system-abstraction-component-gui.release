#include <stdhdrs.h>
#include <sack_types.h>
#include <logging.h>
#include <deadstart.h>

SACK_DEADSTART_NAMESPACE

#undef PRELOAD
#ifdef __WATCOMC__ 
// this is really nice - to have a prioritized initializer list...
#ifdef __cplusplus
#define PRELOAD(name) MAGIC_PRIORITY_PRELOAD(name,DEADSTART_PRELOAD_PRIORITY)
#else
#define PRELOAD(name) static void name(void); \
	struct rt_init __based(__segname("XI")) name##_ctor_label={0,DEADSTART_PRELOAD_PRIORITY,name}; \
	static void name(void)
#endif
#elif defined( _MSC_VER )
#  ifdef __cplusplus_cli 
#    define PRELOAD(name) static void _##name(void); \
    public ref class name {   \
	public:name() { _##name(); \
		System::Console::WriteLine( /*lprintf( */gcnew System::String( WIDE("Startups.ADSF.. ") ) ); \
	  }  \
	}/* do_schedul_##name*/;     \
	static void _##name(void)
//PRIORITY_PRELOAD(name,DEADSTART_PRELOAD_PRIORITY)
#  elif defined( __cplusplus )
   static class schedule_name {   \
	  public:schedule_name() {  \
	InvokeDeadstart();  \
	  }  \
	} do_schedul_name;     
#  else
#    if (_MSC_VER==1300)
#      define PRELOAD(name) static void name(void); \
	__declspec(allocate(_STARTSEG2_)) void (CPROC *pointer_##name)(void) = name; \
	void name(void)
#    else
#       define PRELOAD(name) static void name(void); \
	__declspec(allocate(_STARTSEG2_)) void (CPROC *pointer_##name)(void) = name; \
	void name(void)
#    endif
#  endif
#elif defined( __GNUC__ )
#    define PRELOAD(name) void name( void ) __attribute__((constructor)); \
void name( void )
#endif

// this one is used when a library is loaded.
#ifndef _WIN64
PRELOAD( RunDeadstart )
{
	InvokeDeadstart(); // call everthing which is logged within SACK to dispatch back to registree's
	MarkRootDeadstartComplete();
}
#endif
SACK_DEADSTART_NAMESPACE_END

//#endif