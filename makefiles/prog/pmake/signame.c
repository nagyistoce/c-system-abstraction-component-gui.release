/* Convert between signal names and numbers.
Copyright (C) 1990,92,93,95,96,99, 2002 Free Software Foundation, Inc.
This file was part of the GNU C Library, but is now part of GNU make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "make.h"

/* If the system provides strsignal, we don't need it. */

#if !defined(HAVE_STRSIGNAL)

/* If the system provides sys_siglist, we'll use that.
   Otherwise create our own.
 */

#if !defined(SYS_SIGLIST_DECLARED)

/* Some systems do not define NSIG in <signal.h>.  */
#ifndef	NSIG
#ifdef	_NSIG
#define	NSIG	_NSIG
#else
#define	NSIG	32
#endif
#endif

/* There is too much variation in Sys V signal numbers and names, so
   we must initialize them at runtime.  */

static const char *undoc;

static const char *sys_siglist[NSIG];

/* Table of abbreviations for signals.  Note:  A given number can
   appear more than once with different abbreviations.  */
#define SIG_TABLE_SIZE  (NSIG*2)

typedef struct
  {
    int number;
    const char *abbrev;
  } num_abbrev;

static num_abbrev sig_table[SIG_TABLE_SIZE];

/* Number of elements of sig_table used.  */
static int sig_table_nelts = 0;

/* Enter signal number NUMBER into the tables with ABBREV and NAME.  */

static void
init_sig (number, abbrev, name)
     int number;
     const char *abbrev;
     const char *name;
{
  /* If this value is ever greater than NSIG it seems like it'd be a bug in
     the system headers, but... better safe than sorry.  We know, for
     example, that this isn't always true on VMS.  */

  if (number >= 0 && number < NSIG)
    sys_siglist[number] = name;

  if (sig_table_nelts < SIG_TABLE_SIZE)
    {
      sig_table[sig_table_nelts].number = number;
      sig_table[sig_table_nelts++].abbrev = abbrev;
    }
}

static int
signame_init ()
{
  int i;

  undoc = xstrdup (_("unknown signal"));

  /* Initialize signal names.  */
  for (i = 0; i < NSIG; i++)
    sys_siglist[i] = undoc;

  /* Initialize signal names.  */
#if defined (SIGHUP)
  init_sig (SIGHUP, WIDE("HUP"), _("Hangup"));
#endif
#if defined (SIGINT)
  init_sig (SIGINT, WIDE("INT"), _("Interrupt"));
#endif
#if defined (SIGQUIT)
  init_sig (SIGQUIT, WIDE("QUIT"), _("Quit"));
#endif
#if defined (SIGILL)
  init_sig (SIGILL, WIDE("ILL"), _("Illegal Instruction"));
#endif
#if defined (SIGTRAP)
  init_sig (SIGTRAP, WIDE("TRAP"), _("Trace/breakpoint trap"));
#endif
  /* If SIGIOT == SIGABRT, we want to print it as SIGABRT because
     SIGABRT is in ANSI and POSIX.1 and SIGIOT isn't.  */
#if defined (SIGABRT)
  init_sig (SIGABRT, WIDE("ABRT"), _("Aborted"));
#endif
#if defined (SIGIOT)
  init_sig (SIGIOT, WIDE("IOT"), _("IOT trap"));
#endif
#if defined (SIGEMT)
  init_sig (SIGEMT, WIDE("EMT"), _("EMT trap"));
#endif
#if defined (SIGFPE)
  init_sig (SIGFPE, WIDE("FPE"), _("Floating point exception"));
#endif
#if defined (SIGKILL)
  init_sig (SIGKILL, WIDE("KILL"), _("Killed"));
#endif
#if defined (SIGBUS)
  init_sig (SIGBUS, WIDE("BUS"), _("Bus error"));
#endif
#if defined (SIGSEGV)
  init_sig (SIGSEGV, WIDE("SEGV"), _("Segmentation fault"));
#endif
#if defined (SIGSYS)
  init_sig (SIGSYS, WIDE("SYS"), _("Bad system call"));
#endif
#if defined (SIGPIPE)
  init_sig (SIGPIPE, WIDE("PIPE"), _("Broken pipe"));
#endif
#if defined (SIGALRM)
  init_sig (SIGALRM, WIDE("ALRM"), _("Alarm clock"));
#endif
#if defined (SIGTERM)
  init_sig (SIGTERM, WIDE("TERM"), _("Terminated"));
#endif
#if defined (SIGUSR1)
  init_sig (SIGUSR1, WIDE("USR1"), _("User defined signal 1"));
#endif
#if defined (SIGUSR2)
  init_sig (SIGUSR2, WIDE("USR2"), _("User defined signal 2"));
#endif
  /* If SIGCLD == SIGCHLD, we want to print it as SIGCHLD because that
     is what is in POSIX.1.  */
#if defined (SIGCHLD)
  init_sig (SIGCHLD, WIDE("CHLD"), _("Child exited"));
#endif
#if defined (SIGCLD)
  init_sig (SIGCLD, WIDE("CLD"), _("Child exited"));
#endif
#if defined (SIGPWR)
  init_sig (SIGPWR, WIDE("PWR"), _("Power failure"));
#endif
#if defined (SIGTSTP)
  init_sig (SIGTSTP, WIDE("TSTP"), _("Stopped"));
#endif
#if defined (SIGTTIN)
  init_sig (SIGTTIN, WIDE("TTIN"), _("Stopped (tty input)"));
#endif
#if defined (SIGTTOU)
  init_sig (SIGTTOU, WIDE("TTOU"), _("Stopped (tty output)"));
#endif
#if defined (SIGSTOP)
  init_sig (SIGSTOP, WIDE("STOP"), _("Stopped (signal)"));
#endif
#if defined (SIGXCPU)
  init_sig (SIGXCPU, WIDE("XCPU"), _("CPU time limit exceeded"));
#endif
#if defined (SIGXFSZ)
  init_sig (SIGXFSZ, WIDE("XFSZ"), _("File size limit exceeded"));
#endif
#if defined (SIGVTALRM)
  init_sig (SIGVTALRM, WIDE("VTALRM"), _("Virtual timer expired"));
#endif
#if defined (SIGPROF)
  init_sig (SIGPROF, WIDE("PROF"), _("Profiling timer expired"));
#endif
#if defined (SIGWINCH)
  /* "Window size changed" might be more accurate, but even if that
     is all that it means now, perhaps in the future it will be
     extended to cover other kinds of window changes.  */
  init_sig (SIGWINCH, WIDE("WINCH"), _("Window changed"));
#endif
#if defined (SIGCONT)
  init_sig (SIGCONT, WIDE("CONT"), _("Continued"));
#endif
#if defined (SIGURG)
  init_sig (SIGURG, WIDE("URG"), _("Urgent I/O condition"));
#endif
#if defined (SIGIO)
  /* "I/O pending" has also been suggested.  A disadvantage is
     that signal only happens when the process has
     asked for it, not everytime I/O is pending.  Another disadvantage
     is the confusion from giving it a different name than under Unix.  */
  init_sig (SIGIO, WIDE("IO"), _("I/O possible"));
#endif
#if defined (SIGWIND)
  init_sig (SIGWIND, WIDE("WIND"), _("SIGWIND"));
#endif
#if defined (SIGPHONE)
  init_sig (SIGPHONE, WIDE("PHONE"), _("SIGPHONE"));
#endif
#if defined (SIGPOLL)
  init_sig (SIGPOLL, WIDE("POLL"), _("I/O possible"));
#endif
#if defined (SIGLOST)
  init_sig (SIGLOST, WIDE("LOST"), _("Resource lost"));
#endif
#if defined (SIGDANGER)
  init_sig (SIGDANGER, WIDE("DANGER"), _("Danger signal"));
#endif
#if defined (SIGINFO)
  init_sig (SIGINFO, WIDE("INFO"), _("Information request"));
#endif
#if defined (SIGNOFP)
  init_sig (SIGNOFP, WIDE("NOFP"), _("Floating point co-processor not available"));
#endif

  return 1;
}

#endif  /* SYS_SIGLIST_DECLARED */


char *
strsignal (signal)
     int signal;
{
  static char buf[] = "Signal 12345678901234567890";

#if !defined(SYS_SIGLIST_DECLARED)
  static char sig_initted = 0;

  if (!sig_initted)
    sig_initted = signame_init ();
#endif

  if (signal > 0 || signal < NSIG)
    return (char *) sys_siglist[signal];

  sprintf (buf, WIDE("Signal %d"), signal);
  return buf;
}

#endif  /* HAVE_STRSIGNAL */
