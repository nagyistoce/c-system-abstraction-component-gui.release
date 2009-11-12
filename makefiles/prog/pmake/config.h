/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif
#include <sack_types.h>

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define if the closedir function returns void instead of int.  */
/* #undef CLOSEDIR_VOID */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define for DGUX with <sys/dg_sys_info.h>.  */
/* #undef DGUX */

/* Define if the `getloadavg' function needs to be run setuid or setgid.  */
/* #undef GETLOADAVG_PRIVILEGED */

/* Define to `unsigned long' or `unsigned long long'
   if <inttypes.h> doesn't define.  */
#define uintmax_t unsigned long

/* Define to `int' if <sys/types.h> doesn't define.  */
#if (__WATCOMC__ < 1250 )
#undef gid_t
#define gid_t int
#endif

/* Define if you have alloca, as a function or macro.  */
#undef HAVE_ALLOCA
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/* #undef HAVE_ALLOCA_H */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if your system has a working fnmatch function.  */
/* #undef HAVE_FNMATCH */

/* Define if your system has its own `getloadavg' function.  */
/* #undef HAVE_GETLOADAVG */

/* Define if you have the getmntent function.  */
/* #undef HAVE_GETMNTENT */

/* Define if the `long double' type works.  */
/* #undef HAVE_LONG_DOUBLE */

/* Define if you support file names longer than 14 characters.  */
#undef HAVE_LONG_FILE_NAMES
#define HAVE_LONG_FILE_NAMES 1

/* Define if you have a working `mmap' system call.  */
/* #undef HAVE_MMAP */

/* Define if system calls automatically restart after interruption
   by a signal.  */
/* #undef HAVE_RESTARTABLE_SYSCALLS */

/* Define if your struct stat has st_blksize.  */
/* #undef HAVE_ST_BLKSIZE */

/* Define if your struct stat has st_blocks.  */
/* #undef HAVE_ST_BLOCKS */

/* Define if you have the strcoll function and it is properly defined.  */
#undef HAVE_STRCOLL
#define HAVE_STRCOLL 1

/* Define if your struct stat has st_rdev.  */
#undef HAVE_ST_RDEV
#define HAVE_ST_RDEV 1

/* Define if you have the strftime function.  */
#undef HAVE_STRFTIME
#define HAVE_STRFTIME 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
/* #undef HAVE_SYS_WAIT_H */

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#undef HAVE_TZNAME
#define HAVE_TZNAME 1

/* Define if you have <unistd.h>.  */
/* #undef HAVE_UNISTD_H */

/* Define if utime(file, NULL) sets file's timestamp to the present.  */
#undef HAVE_UTIME_NULL
#define HAVE_UTIME_NULL 1

/* Define if you have <vfork.h>.  */
/* #undef HAVE_VFORK_H */

/* Define if you have the vprintf function.  */
#undef HAVE_VPRINTF
#define HAVE_VPRINTF 1

/* Define if you have the wait3 system call.  */
/* #undef HAVE_WAIT3 */

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define if your struct nlist has an n_un member.  */
/* #undef NLIST_NAME_UNION */

/* Define if you have <nlist.h>.  */
/* #undef NLIST_STRUCT */

/* Define if your C compiler doesn't accept -c and -o together.  */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to `int' if <sys/types.h> doesn't define.  */
#if (__WATCOMC__ < 1250 )
#undef pid_t
#define pid_t int
#endif

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE
#define _POSIX_SOURCE 1

/* Define as the return type of signal handlers (int or void).  */
#undef RETSIGTYPE
#define RETSIGTYPE void

/* Define if the setvbuf function takes the buffering type as its second
   argument and the buffer pointer as the third, as on System V
   before release 3.  */
/* #undef SETVBUF_REVERSED */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
/* #undef STAT_MACROS_BROKEN */

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS
#define STDC_HEADERS 1

/* Define on System V Release 4.  */
/* #undef SVR4 */

/* Define if `sys_siglist' is declared by <signal.h>.  */
/* #undef SYS_SIGLIST_DECLARED */

/* Define to `int' if <sys/types.h> doesn't define.  */
#if (__WATCOMC__ < 1250 )
#undef uid_t
#define uid_t int
#endif

/* Define for Encore UMAX.  */
/* #undef UMAX */

/* Define for Encore UMAX 4.3 that has <inq_status/cpustats.h>
   instead of <sys/cpustats.h>.  */
/* #undef UMAX4_3 */

/* Define vfork as fork if vfork does not work.  */
/* #undef vfork */

/* Name of this package (needed by automake) */
#define PACKAGE "make"

/* Version of this package (needed by automake) */
#define VERSION "3.81"

/* Define to the name of the SCCS `get' command.  */
#undef SCCS_GET
#define SCCS_GET "echo no sccs get"

/* Define to 1 if NLS is requested.  */
/* #undef ENABLE_NLS */

/* Define as 1 if you have dcgettext.  */
/* #undef HAVE_DCGETTEXT */

/* Define as 1 if you have gettext and don't want to use GNU gettext.  */
/* #undef HAVE_GETTEXT */

/* Define if your locale.h file contains LC_MESSAGES.  */
/* #undef HAVE_LC_MESSAGES */

/* Define to the installation directory for locales.  */
#define LOCALEDIR ""

/* Define this if the SCCS `get' command understands the `-G<file>' option.  */
/* #undef SCCS_GET_MINUS_G */

/* Define this to enable job server support in GNU make.  */
/* #undef MAKE_JOBSERVER */

/* Define to be the nanoseconds member of struct stat's st_mtim,
   if it exists.  */
/* #undef ST_MTIM_NSEC */

/* Define this if the C library defines the variable `sys_siglist'.  */
/* #undef HAVE_SYS_SIGLIST */

/* Define this if the C library defines the variable `_sys_siglist'.  */
/* #undef HAVE__SYS_SIGLIST */

/* Define this if you have the `union wait' type in <sys/wait.h>.  */
/* #undef HAVE_UNION_WAIT */

/* Define if you have the dup2 function.  */
#undef HAVE_DUP2
#define HAVE_DUP2 1

/* Define if you have the getcwd function.  */
#undef HAVE_GETCWD
#define HAVE_GETCWD 1

/* Define if you have the getgroups function.  */
/* #undef HAVE_GETGROUPS */

/* Define if you have the gethostbyname function.  */
/* #undef HAVE_GETHOSTBYNAME */

/* Define if you have the gethostname function.  */
/* #undef HAVE_GETHOSTNAME */

/* Define if you have the getloadavg function.  */
/* #undef HAVE_GETLOADAVG */

/* Define if you have the memmove function.  */
#undef HAVE_MEMMOVE
#define HAVE_MEMMOVE 1

/* Define if you have the mktemp function.  */
//#undef HAVE_MKTEMP
//#define HAVE_MKTEMP 1

/* Define if you have the psignal function.  */
/* #undef HAVE_PSIGNAL */

/* Define if you have the pstat_getdynamic function.  */
/* #undef HAVE_PSTAT_GETDYNAMIC */

/* Define if you have the setegid function.  */
/* #undef HAVE_SETEGID */

/* Define if you have the seteuid function.  */
/* #undef HAVE_SETEUID */

/* Define if you have the setlinebuf function.  */
/* #undef HAVE_SETLINEBUF */

/* Define if you have the setregid function.  */
/* #undef HAVE_SETREGID */

/* Define if you have the setreuid function.  */
/* #undef HAVE_SETREUID */

/* Define if you have the sigsetmask function.  */
/* #undef HAVE_SIGSETMASK */

/* Define if you have the socket function.  */
/* #undef HAVE_SOCKET */

/* Define if you have the strcasecmp function.  */
/* #undef HAVE_STRCASECMP */

/* Define if you have the strerror function.  */
#undef HAVE_STRERROR
#define HAVE_STRERROR 1

/* Define if you have the strsignal function.  */
/* #undef HAVE_STRSIGNAL */

/* Define if you have the wait3 function.  */
/* #undef HAVE_WAIT3 */

/* Define if you have the waitpid function.  */
/* #undef HAVE_WAITPID */

/* Define if you have the <dirent.h> header file.  */
#undef HAVE_DIRENT_H
#define HAVE_DIRENT_H 1

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#undef HAVE_LIMITS_H
#define HAVE_LIMITS_H 1

/* Define if you have the <mach/mach.h> header file.  */
/* #undef HAVE_MACH_MACH_H */

/* Define if you have the <memory.h> header file.  */
#undef HAVE_MEMORY_H
#define HAVE_MEMORY_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <string.h> header file.  */
#undef HAVE_STRING_H
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/param.h> header file.  */
/* #undef HAVE_SYS_PARAM_H */

/* Define if you have the <sys/timeb.h> header file.  */
#undef HAVE_SYS_TIMEB_H
#define HAVE_SYS_TIMEB_H 1

/* Define if you have the <sys/wait.h> header file.  */
/* #undef HAVE_SYS_WAIT_H */

/* Define if you have the <unistd.h> header file.  */
/* #undef HAVE_UNISTD_H */

/* Define if you have the dgc library (-ldgc).  */
/* #undef HAVE_LIBDGC */

/* Define if you have the kstat library (-lkstat).  */
/* #undef HAVE_LIBKSTAT */

/* Define if you have the sun library (-lsun).  */
/* #undef HAVE_LIBSUN */

/* Use high resolution file timestamps if nonzero. */
#define FILE_TIMESTAMP_HI_RES 0

/* Build host information. */
#define MAKE_HOST "Windows32"

/* Grok DOS paths (drive specs and backslash path element separators) */
#define HAVE_DOS_PATHS

/*
 * Refer to README.W32 for info on the following settings
 */

/*
 * If you have a shell that does not grok 'sh -c quoted-command-line'
 * correctly, you need this setting. Please see below for specific
 * shell support.
 */
#undef BATCH_MODE_ONLY_SHELL
#define BATCH_MODE_ONLY_SHELL 1

/*
 * Define if you have the Cygnus "Cygwin" GNU Windows32 tool set.
 * Do NOT define BATCH_MODE_ONLY_SHELL if you define HAVE_CYGWIN_SHELL
 */
#undef HAVE_CYGWIN_SHELL

/*
 * Define if you have the MKS tool set or shell. Do NOT define
 * BATCH_MODE_ONLY_SHELL if you define HAVE_MKS_SHELL
 */
#undef HAVE_MKS_SHELL

/*
 * Enforce the mutual exclusivity restriction.
 */
#ifdef HAVE_MKS_SHELL
#undef BATCH_MODE_ONLY_SHELL
#endif

#ifdef HAVE_CYGWIN_SHELL
#undef BATCH_MODE_ONLY_SHELL
#endif

/* Define if you prefer Case Insensitive behavior */
#undef HAVE_CASE_INSENSITIVE_FS
