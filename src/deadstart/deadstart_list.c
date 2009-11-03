
#include <sack_types.h>
#include <deadstart.h>
#ifndef TARGET_LABEL
//#error WTF!
#endif

#ifdef GCC
#define paste(a,b) a##b
#define paste2(a,b) paste(a,b)
#define DeclareList(n) paste2(n,TARGET_LABEL)
#ifdef __cplusplus
//struct rt_init DeclareList(begin_deadstart_) __attribute__((section("deadstart_list"))) = { (_8)-1 };
#else
struct rt_init DeclareList(begin_deadstart_) __attribute__((section("deadstart_list"))) = { -1 };
#endif
#endif
