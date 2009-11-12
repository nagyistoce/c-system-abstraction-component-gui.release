/* Miscellaneous global declarations and portability cruft for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Free Software
Foundation, Inc.
Copyright (C) 2008 R. Bernstein <rocky@gnu.org>

This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2, or (at your option) any later version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
GNU Make; see the file COPYING.  If not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.  */

/* We use <config.h> instead of "config.h" so that a compilation
   using -I. -I$srcdir will use ./config.h rather than $srcdir/config.h
   (which it would do because make.h was found in $srcdir).  */

#ifndef MAKE_H
#define MAKE_H

#include <config.h>
#undef  HAVE_CONFIG_H
#define HAVE_CONFIG_H 1

/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif


/* Use prototypes if available.  */
#if defined (__cplusplus) || defined (__STDC__)
# undef  PARAMS
# define PARAMS(protos)  protos
#else /* Not C++ or ANSI C.  */
# undef  PARAMS
# define PARAMS(protos)  ()
#endif /* C++ or ANSI C.  */

/* Specify we want GNU source code.  This must be defined before any
   system headers are included.  */

#define _GNU_SOURCE 1

#include "types.h"
#ifdef  CRAY
/* This must happen before #include <signal.h> so
   that the declaration therein is changed.  */
# define signal bsdsignal
#endif

/* If we're compiling for the dmalloc debugger, turn off string inlining.  */
#if defined(HAVE_DMALLOC_H) && defined(__GNUC__)
# define __NO_STRING_INLINES
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_SYS_TIMEB_H
/* SCO 3.2 "devsys 4.2" has a prototype for `ftime' in <time.h> that bombs
   unless <sys/timeb.h> has been included first.  Does every system have a
   <sys/timeb.h>?  If any does not, configure should check for it.  */
# include <sys/timeb.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <errno.h>

#ifndef errno
extern int errno;
#endif

#ifndef isblank
# define isblank(c)     ((c) == ' ' || (c) == '\t')
#endif

#ifdef  HAVE_UNISTD_H
# include <unistd.h>
/* Ultrix's unistd.h always defines _POSIX_VERSION, but you only get
   POSIX.1 behavior with `cc -YPOSIX', which predefines POSIX itself!  */
# if defined (_POSIX_VERSION) && !defined (ultrix) && !defined (VMS)
#  define POSIX 1
# endif
#endif

/* Some systems define _POSIX_VERSION but are not really POSIX.1.  */
#if (defined (butterfly) || defined (__arm) || (defined (__mips) && defined (_SYSTYPE_SVR3)) || (defined (sequent) && defined (i386)))
# undef POSIX
#endif

#if !defined (POSIX) && defined (_AIX) && defined (_POSIX_SOURCE)
# define POSIX 1
#endif

#ifndef RETSIGTYPE
# define RETSIGTYPE     void
#endif

#ifndef sigmask
# define sigmask(sig)   (1 << ((sig) - 1))
#endif

#ifndef HAVE_SA_RESTART
# define SA_RESTART 0
#endif

#ifdef  HAVE_LIMITS_H
# include <limits.h>
#endif
#ifdef  HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifndef PATH_MAX
# ifndef POSIX
#  define PATH_MAX      MAXPATHLEN
# endif
#endif
#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#ifdef  PATH_MAX
# define GET_PATH_MAX   PATH_MAX
# define PATH_VAR(var)  char var[PATH_MAX]
#else
# define NEED_GET_PATH_MAX 1
# define GET_PATH_MAX   (get_path_max ())
# define PATH_VAR(var)  char *var = (char *) alloca (GET_PATH_MAX)
extern unsigned int get_path_max PARAMS ((void));
#endif

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

/* Nonzero if the integer type T is signed.  */
#define INTEGER_TYPE_SIGNED(t) ((t) -1 < 0)

/* The minimum and maximum values for the integer type T.
   Use ~ (t) 0, not -1, for portability to 1's complement hosts.  */
#define INTEGER_TYPE_MINIMUM(t) \
  (! INTEGER_TYPE_SIGNED (t) ? (t) 0 : ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1))
#define INTEGER_TYPE_MAXIMUM(t) (~ (t) 0 - INTEGER_TYPE_MINIMUM (t))

#ifndef CHAR_MAX
# define CHAR_MAX INTEGER_TYPE_MAXIMUM (char)
#endif

#ifdef STAT_MACROS_BROKEN
# ifdef S_ISREG
#  undef S_ISREG
# endif
# ifdef S_ISDIR
#  undef S_ISDIR
# endif
#endif  /* STAT_MACROS_BROKEN.  */

#ifndef S_ISREG
# define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
# define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifdef VMS
# include <types.h>
# include <unixlib.h>
# include <unixio.h>
# include <perror.h>
/* Needed to use alloca on VMS.  */
# include <builtins.h>
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(x)
# endif
/* The __-protected variants of `format' and `printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif
#define UNUSED  __attribute__ ((unused))

#if defined (STDC_HEADERS) || defined (__GNU_LIBRARY__)
# include <stdlib.h>
# include <string.h>
# define ANSI_STRING 1
#else   /* No standard headers.  */
# ifdef HAVE_STRING_H
#  include <string.h>
#  define ANSI_STRING 1
# else
#  include <strings.h>
# endif
# ifdef HAVE_MEMORY_H
#  include <memory.h>
# endif
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# else
extern char *malloc PARAMS ((int));
extern char *realloc PARAMS ((char *, int));
extern void free PARAMS ((char *));

extern void abort PARAMS ((void)) __attribute__ ((noreturn));
extern void exit PARAMS ((int)) __attribute__ ((noreturn));
# endif /* HAVE_STDLIB_H.  */

#endif /* Standard headers.  */

/* These should be in stdlib.h.  Make sure we have them.  */
#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 0
#endif

#ifdef  ANSI_STRING

# ifndef bcmp
#  define bcmp(s1, s2, n)   memcmp ((s1), (s2), (n))
# endif
# ifndef bzero
#  define bzero(s, n)       memset ((s), 0, (n))
# endif
# if defined(HAVE_MEMMOVE) && !defined(bcopy)
#  define bcopy(s, d, n)    memmove ((d), (s), (n))
# endif

#else   /* Not ANSI_STRING.  */

# ifndef HAVE_STRCHR
#  define strchr(s, c)      index((s), (c))
#  define strrchr(s, c)     rindex((s), (c))
# endif

# ifndef bcmp
extern int bcmp PARAMS ((const char *, const char *, int));
# endif
# ifndef bzero
extern void bzero PARAMS ((char *, int));
#endif
# ifndef bcopy
extern void bcopy PARAMS ((const char *b1, char *b2, int));
# endif

/* SCO Xenix has a buggy macro definition in <string.h>.  */
#undef  strerror
#if !defined(__DECC)
extern char *strerror PARAMS ((int errnum));
#endif

#endif  /* !ANSI_STRING.  */
#undef  ANSI_STRING

#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#define FILE_TIMESTAMP uintmax_t

#if !defined(HAVE_STRSIGNAL)
extern char *strsignal PARAMS ((int signum));
#endif

/* ISDIGIT offers the following features:
   - Its arg may be any int or unsigned int; it need not be an unsigned char.
   - It's guaranteed to evaluate its argument exactly once.
      NOTE!  Make relies on this behavior, don't change it!
   - It's typically faster.
   POSIX 1003.2-1992 section 2.5.2.1 page 50 lines 1556-1558 says that
   only '0' through '9' are digits.  Prefer ISDIGIT to isdigit() unless
   it's important to use the locale's definition of `digit' even when the
   host does not conform to POSIX.  */
#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)

#ifndef iAPX286
# define streq(a, b) \
   ((a) == (b) || \
    (*(a) == *(b) && (*(a) == '\0' || !strcmp ((a) + 1, (b) + 1))))
# ifdef HAVE_CASE_INSENSITIVE_FS
/* This is only used on Windows/DOS platforms, so we assume strcmpi().  */
#  define strieq(a, b) \
    ((a) == (b) \
     || (tolower((unsigned char)*(a)) == tolower((unsigned char)*(b)) \
         && (*(a) == '\0' || !strcmpi ((a) + 1, (b) + 1))))
# else
#  define strieq(a, b) streq(a, b)
# endif
#else
/* Buggy compiler can't handle this.  */
# define streq(a, b) (strcmp ((a), (b)) == 0)
# define strieq(a, b) (strcmp ((a), (b)) == 0)
#endif
#define strneq(a, b, l) (strncmp ((a), (b), (l)) == 0)
#ifdef  VMS
extern int strcmpi (const char *,const char *);
#endif

#if defined(__GNUC__) || defined(ENUM_BITFIELDS)
# define ENUM_BITFIELD(bits)    :bits
#else
# define ENUM_BITFIELD(bits)
#endif

/* Handle gettext and locales.  */

#if HAVE_LOCALE_H
# include <locale.h>
#else
# define setlocale(category, locale)
#endif

#include <gettext.h>

#define _(msgid)            gettext (msgid)
#define N_(msgid)           gettext_noop (msgid)
#define S_(msg1,msg2,num)   ngettext (msg1,msg2,num)

/* Handle other OSs.  */
#if defined(HAVE_DOS_PATHS)
# define PATH_SEPARATOR_CHAR ';'
#elif defined(VMS)
# define PATH_SEPARATOR_CHAR ','
#else
# define PATH_SEPARATOR_CHAR ':'
#endif

/* This is needed for getcwd() and chdir().  */
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined( __WATCOMC__ )
# include <direct.h>
#endif

#include "print.h"

#ifdef WINDOWS32
# include <fcntl.h>
# include <malloc.h>
# define pipe(p) _pipe(p, 512, O_BINARY)
# define kill(pid,sig) w32_kill(pid,sig)

extern void sync_Path_environment(void);
extern int kill(int pid, int sig);
extern char *end_of_token_w32(char *s, char stopchar);
extern int find_and_set_default_shell(char *token);

/* indicates whether or not we have Bourne shell */
extern bool no_default_sh_exe;

/* is default_shell unixy? */
extern int unixy_shell;
#endif  /* WINDOWS32 */

#define STRING_SIZE_TUPLE(_s) (_s), (sizeof (_s)-1)
#define	CALLOC(t, n) ((t *) calloc  (sizeof (t), (n)))
#define MALLOC(t, n) ((t *) xmalloc (sizeof (t) * (n)))
#define REALLOC(o, t, n) ((t *) xrealloc ((void *) (o), sizeof (t) * (n)))
#define FREE(p) if (p) { free(p); p = NULL;}


/* We have to have stdarg.h or varargs.h AND v*printf or doprnt to use
   variadic versions of these functions.  */

#if HAVE_STDARG_H || HAVE_VARARGS_H
# if HAVE_VPRINTF || HAVE_DOPRNT
#  define USE_VARIADIC 1
# endif
#endif

extern void die PARAMS ((int)) __attribute__ ((noreturn));
extern void log_working_directory PARAMS ((int));
extern void perror_with_name PARAMS ((const char *, const char *));
extern char *savestring PARAMS ((const char *, unsigned int));
extern char *concat PARAMS ((const char *, const char *, const char *));
extern char *xmalloc PARAMS ((unsigned int));
extern char *xrealloc PARAMS ((char *, unsigned int));
extern char *xstrdup PARAMS ((const char *));
extern char *find_next_token PARAMS ((char **, unsigned int *));
extern char *next_token PARAMS ((const char *));
extern char *end_of_token PARAMS ((const char *));
extern void collapse_continuations PARAMS ((char *));
extern char *lindex PARAMS ((const char *, const char *, int));
extern int alpha_compare PARAMS ((const void *, const void *));
extern void print_spaces PARAMS ((unsigned int));
extern char *find_percent PARAMS ((char *));
extern FILE *open_tmpfile PARAMS ((char **, const char *));

#ifndef NO_ARCHIVES
extern bool ar_name PARAMS ((char *));
extern void ar_parse_name PARAMS ((char *, char **, char **));
extern int ar_touch PARAMS ((char *));
extern time_t ar_member_date PARAMS ((char *));
#endif

extern void define_default_variables PARAMS ((void));
extern void set_default_suffixes PARAMS ((void));
extern void install_default_suffix_rules PARAMS ((void));
extern void install_default_implicit_rules PARAMS ((void));

extern void build_vpath_lists PARAMS ((void));
extern void construct_vpath_list PARAMS ((char *pattern, char *dirpath));
extern int vpath_search PARAMS ((char **file, FILE_TIMESTAMP *mtime_ptr));
extern int gpath_search PARAMS ((char *file, unsigned int len));

extern void construct_include_path PARAMS ((char **arg_dirs));

extern void user_access PARAMS ((void));
extern void make_access PARAMS ((void));
extern void child_access PARAMS ((void));

extern void close_stdout PARAMS ((void));

extern char *strip_whitespace PARAMS ((const char **begpp, const char **endpp));

/* String caching  */
extern void strcache_init PARAMS ((void));
extern void strcache_print_stats PARAMS ((const char *prefix));
extern int strcache_iscached PARAMS ((const char *str));
extern const char *strcache_add PARAMS ((const char *str));
extern const char *strcache_add_len PARAMS ((const char *str, int len));
extern int strcache_setbufsize PARAMS ((int size));

#ifdef  HAVE_VFORK_H
# include <vfork.h>
#endif

/* We omit these declarations on non-POSIX systems which define _POSIX_VERSION,
   because such systems often declare them in header files anyway.  */

#if !defined (__GNU_LIBRARY__) && !defined (POSIX) && !defined (_POSIX_VERSION) && !defined(WINDOWS32)

extern long int atol ();
# ifndef VMS
extern long int lseek ();
# endif

#endif  /* Not GNU C library or POSIX.  */

#ifdef  HAVE_GETCWD
# if !defined(VMS) && !defined(__DECC)
extern char *getcwd ();
# endif
#else
extern char *getwd ();
# define getcwd(buf, len)       getwd (buf)
#endif

extern const struct floc *reading_file;
extern const struct floc **expanding_var;

//extern char **environ;

/*! The recognized command switches.  */

/*! Nonzero means do not print commands to be executed (-s).  */
extern int silent_flag;

/*! Nonzero means just touch the files
   that would appear to need remaking (-t)  */
extern int touch_flag;

/*! Nonzero means just print what commands would need to be executed,
   don't actually execute them (-n).  */
extern int just_print_flag;

/*! Environment variables override makefile definitions.  */
extern int env_overrides;

/*! Nonzero means ignore status codes returned by commands
   executed to remake files.  Just treat them all as successful (-i).  */

extern int ignore_errors_flag;

/*! Nonzero means don't remake anything, just print the data base
   that results from reading the makefile (-p).  */

extern int print_data_base_flag;

/*! Nonzero means don't remake anything; just return a nonzero status
   if the specified targets are not up to date (-q).  */

extern int question_flag;

/*! Nonzero means do not use any of the builtin rules (-r) / variables
  (-R).  */
extern int no_builtin_rules_flag;
extern int no_builtin_variables_flag;

/*! Nonzero means keep going even if remaking some file fails
  (-k).  */
extern int keep_going_flag;

/*! Nonzero means check symlink mtimes. (-L) */
extern int check_symlink_flag;

/*! Nonzero means print directory before starting and when done
  (-w).  */
extern int print_directory_flag;

extern int always_make_flag;
extern int print_directory_flag;
extern int warn_undefined_variables_flag;
extern int rebuilding_makefiles;

/*! True if some rule detected clock skew; we keep track so (a) we only
   print one warning about it during the run, and (b) we can print a final
   warning at the end of the run. */

extern bool clock_skew_detected;

extern int print_version_flag;
extern bool not_parallel;

/*! True if we have seen the magic `.POSIX' target.
   This turns on pedantic compliance with POSIX.2.  */

extern bool posix_pedantic;

/*! True if we have seen the '.SECONDEXPANSION' target.
   This turns on secondary expansion of prerequisites.  */

extern bool second_expansion;

/* can we run commands via 'sh -c xxx' or must we use batch files? */
extern bool batch_mode_shell;

/*! This character introduces a command: it's the first char on the
    line.  */

extern char cmd_prefix;

extern unsigned int job_slots;
extern int job_fds[2];
extern int job_rfd;
#ifndef NO_FLOAT
extern double max_load_average;
#else
extern int max_load_average;
#endif

extern char *program;

/*! Our initial arguments -- used for debugger restart execvp.  */
extern char **global_argv;

extern char *starting_directory;

/*! Our current directory before processing any -C options.  */
extern char *directory_before_chdir;

/*! Value of the MAKELEVEL variable at startup (or 0).  */
extern unsigned int makelevel;

/*! If nonzero, the basename of filenames is in giving locations. Normally,
    giving a file directory location helps a debugger frontend
    when we change directories. For regression tests it is helpful to 
    list just the basename part as that doesn't change from installation
    to installation. Users may have their preferences too.
*/
extern int basename_filenames;

extern char *version_string, *remote_description, *make_host;

extern unsigned int commands_started;

extern int handling_fatal_signal;

/* is default_shell unixy? */
extern int unixy_shell;

/**! The default value of SHELL and the shell that is used when issuing
   commands on targets.
*/
extern char* default_shell;

/*! Print version information.
*/
extern void print_version (void);

#ifndef MIN
#define MIN(_a,_b) ((_a)<(_b)?(_a):(_b))
#endif
#ifndef MAX
#define MAX(_a,_b) ((_a)>(_b)?(_a):(_b))
#endif

/* \brief the exit codes that the GNU Make gives. */
typedef enum {
  MAKE_SUCCESS = 0, /**< GNU Make completed okay */
  MAKE_TROUBLE = 1, /**< A we ran failed */
  MAKE_FAILURE = 2  /**< GNU Make had an internal error/failure */
} make_exit_code_t;

/** This variable is trickery to force the above enum symbol values to
    be recorded in debug symbol tables. It is used to allow one refer
    to above enumeration values in a debugger and debugger
    expressions */
extern make_exit_code_t make_exit_code;

/* Set up heap debugging library dmalloc.  */

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#ifndef initialize_main
# ifdef __EMX__
#  define initialize_main(pargc, pargv) \
                          { _wildcard(pargc, pargv); _response(pargc, pargv); }
# else
#  define initialize_main(pargc, pargv)
# endif
#endif


#ifdef __EMX__
# if !HAVE_STRCASECMP
#  define strcasecmp stricmp
# endif

# if !defined chdir
#  define chdir _chdir2
# endif
# if !defined getcwd
#  define getcwd _getcwd2
# endif

/* NO_CHDIR2 causes make not to use _chdir2() and _getcwd2() instead of
   chdir() and getcwd(). This avoids some error messages for the
   make testsuite but restricts the drive letter support. */
# ifdef NO_CHDIR2
#  warning NO_CHDIR2: usage of drive letters restricted
#  undef chdir
#  undef getcwd
# endif
#endif

#ifndef initialize_main
# define initialize_main(pargc, pargv)
#endif


/* Some systems (like Solaris, PTX, etc.) do not support the SA_RESTART flag
   properly according to POSIX.  So, we try to wrap common system calls with
   checks for EINTR.  Note that there are still plenty of system calls that
   can fail with EINTR but this, reportedly, gets the vast majority of
   failure cases.  If you still experience failures you'll need to either get
   a system where SA_RESTART works, or you need to avoid -j.  */

#define EINTRLOOP(_v,_c)   while (((_v)=_c)==-1 && errno==EINTR)

/* While system calls that return integers are pretty consistent about
   returning -1 on failure and setting errno in that case, functions that
   return pointers are not always so well behaved.  Sometimes they return
   NULL for expected behavior: one good example is readdir() which returns
   NULL at the end of the directory--and _doesn't_ reset errno.  So, we have
   to do it ourselves here.  */

#define ENULLLOOP(_v,_c)   do{ errno = 0; \
                               while (((_v)=_c)==0 && errno==EINTR); }while(0)

#endif /*MAKE_H*/
