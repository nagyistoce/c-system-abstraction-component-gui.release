/* Data base of default implicit rules for GNU Make.
Copyright (C) 1988,89,90,91,92,93,94,95,96 Free Software Foundation, Inc.
This file is part of GNU Make.

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
#include "rule.h"
#include "dep.h"
#include "filedef.h"
#include "job.h"
#include "commands.h"
#include "variable.h"

/* Define GCC_IS_NATIVE if gcc is the native development environment on
   your system (gcc/bison/flex vs cc/yacc/lex).  */
#ifdef __MSDOS__
#define GCC_IS_NATIVE
#endif


/* This is the default list of suffixes for suffix rules.
   `.s' must come last, so that a `.o' file will be made from
   a `.c' or `.p' or ... file rather than from a .s file.  */

static char default_suffixes[]
#ifdef VMS
  = ".exe .olb .ln .obj .c .cxx .cc .pas .p .for .f .r .y .l .mar \
.s .ss .i .ii .mod .sym .def .h .info .dvi .tex .texinfo .texi .txinfo \
.w .ch .cweb .web .com .sh .elc .el";
#else
  = ".out .a .ln .o .c .cc .C .cpp .p .f .F .r .y .l .s .S \
.mod .sym .def .h .info .dvi .tex .texinfo .texi .txinfo \
.w .ch .web .sh .elc .el";
#endif

static struct pspec default_pattern_rules[] =
  {
    { "(%)", WIDE("%"),
	"$(AR) $(ARFLAGS) $@ $<" },

    /* The X.out rules are only in BSD's default set because
       BSD Make has no null-suffix rules, so `foo.out' and
       `foo' are the same thing.  */
#ifdef VMS
    { "%.exe", WIDE("%"),
        "copy $< $@" },
#else
    { "%.out", WIDE("%"),
	"@rm -f $@ \n cp $< $@" },
#endif
    /* Syntax is "ctangle foo.w foo.ch foo.c".  */
    { "%.c", WIDE("%.w %.ch"),
	"$(CTANGLE) $^ $@" },
    { "%.tex", WIDE("%.w %.ch"),
	"$(CWEAVE) $^ $@" },

    { 0, 0, 0 }
  };

static struct pspec default_terminal_rules[] =
  {
#ifdef VMS
    /* RCS.  */
    { "%", WIDE("%$$5lv"), /* Multinet style */
        "if f$$search($@) .nes. \"\" then +$(CHECKOUT,v)" },
    { "%", WIDE("[.$$rcs]%$$5lv"), /* Multinet style */
        "if f$$search($@) .nes. \"\" then +$(CHECKOUT,v)" },
    { "%", WIDE("%_v"), /* Normal style */
        "if f$$search($@) .nes. \"\" then +$(CHECKOUT,v)" },
    { "%", WIDE("[.rcs]%_v"), /* Normal style */
        "if f$$search($@) .nes. \"\" then +$(CHECKOUT,v)" },

    /* SCCS.  */
	/* ain't no SCCS on vms */
#else
    /* RCS.  */
    { "%", WIDE("%,v"),
	"$(CHECKOUT,v)" },
    { "%", WIDE("RCS/%,v"),
	"$(CHECKOUT,v)" },
    { "%", WIDE("RCS/%"),
	"$(CHECKOUT,v)" },

    /* SCCS.  */
    { "%", WIDE("s.%"),
	"$(GET) $(GFLAGS) $(SCCS_OUTPUT_OPTION) $<" },
    { "%", WIDE("SCCS/s.%"),
	"$(GET) $(GFLAGS) $(SCCS_OUTPUT_OPTION) $<" },
#endif /* !VMS */
    { 0, 0, 0 }
  };

static char *default_suffix_rules[] =
  {
#ifdef VMS
    ".obj.exe",
    "$(LINK.obj) $^ $(LOADLIBES) $(LDLIBS) $(CRT0) /exe=$@",
    ".mar.exe",
    "$(COMPILE.mar) $^ \n $(LINK.obj) $(subst .mar,.obj,$^) $(LOADLIBES) $(LDLIBS) $(CRT0) /exe=$@",
    ".s.exe",
    "$(COMPILE.s) $^ \n $(LINK.obj) $(subst .s,.obj,$^) $(LOADLIBES) $(LDLIBS) $(CRT0) /exe=$@",
    ".c.exe",
    "$(COMPILE.c) $^ \n $(LINK.obj) $(subst .c,.obj,$^) $(LOADLIBES) $(LDLIBS) $(CRT0) /exe=$@",
    ".cc.exe",
#ifdef GCC_IS_NATIVE
    "$(COMPILE.cc) $^ \n $(LINK.obj) $(CXXSTARTUP),sys$$disk:[]$(subst .cc,.obj,$^) $(LOADLIBES) $(LXLIBS) $(LDLIBS) $(CXXRT0) /exe=$@",
#else
    "$(COMPILE.cc) $^ \n $(CXXLINK.obj) $(subst .cc,.obj,$^) $(LOADLIBES) $(LXLIBS) $(LDLIBS) $(CXXRT0) /exe=$@",
    ".cxx.exe",
    "$(COMPILE.cxx) $^ \n $(CXXLINK.obj) $(subst .cxx,.obj,$^) $(LOADLIBES) $(LXLIBS) $(LDLIBS) $(CXXRT0) /exe=$@",
#endif
    ".for.exe",
    "$(COMPILE.for) $^ \n $(LINK.obj) $(subst .for,.obj,$^) $(LOADLIBES) $(LDLIBS) /exe=$@",
    ".pas.exe",
    "$(COMPILE.pas) $^ \n $(LINK.obj) $(subst .pas,.obj,$^) $(LOADLIBES) $(LDLIBS) /exe=$@",

    ".com",
    "copy $< >$@",

    ".mar.obj",
    "$(COMPILE.mar) /obj=$@ $<",
    ".s.obj",
    "$(COMPILE.s) /obj=$@ $<",
    ".ss.obj",
    "$(COMPILE.s) /obj=$@ $<",
    ".c.i",
    "$(COMPILE.c)/prep /list=$@ $<",
    ".c.s",
    "$(COMPILE.c)/noobj/machine /list=$@ $<",
    ".i.s",
    "$(COMPILE.c)/noprep/noobj/machine /list=$@ $<",
    ".c.obj",
    "$(COMPILE.c) /obj=$@ $<",
    ".cc.ii",
    "$(COMPILE.cc)/prep /list=$@ $<",
    ".cc.ss",
    "$(COMPILE.cc)/noobj/machine /list=$@ $<",
    ".ii.ss",
    "$(COMPILE.cc)/noprep/noobj/machine /list=$@ $<",
    ".cc.obj",
    "$(COMPILE.cc) /obj=$@ $<",
    ".for.obj",
    "$(COMPILE.for) /obj=$@ $<",
    ".pas.obj",
    "$(COMPILE.pas) /obj=$@ $<",

    ".y.c",
    "$(YACC.y) $< \n rename y_tab.c $@",
    ".l.c",
    "$(LEX.l) $< \n rename lexyy.c $@",

    ".texinfo.info",
    "$(MAKEINFO) $<",

    ".tex.dvi",
    "$(TEX) $<",

#else /* ! VMS */

    ".o",
    "$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".s",
    "$(LINK.s) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".S",
    "$(LINK.S) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".c",
    "$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".cc",
    "$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".C",
    "$(LINK.C) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".cpp",
    "$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".f",
    "$(LINK.f) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".p",
    "$(LINK.p) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".F",
    "$(LINK.F) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".r",
    "$(LINK.r) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".mod",
    "$(COMPILE.mod) -o $@ -e $@ $^",

    ".def.sym",
    "$(COMPILE.def) -o $@ $<",

    ".sh",
    "cat $< >$@ \n chmod a+x $@",

    ".s.o",
    "$(COMPILE.s) -o $@ $<",
    ".S.o",
    "$(COMPILE.S) -o $@ $<",
    ".c.o",
    "$(COMPILE.c) $(OUTPUT_OPTION) $<",
    ".cc.o",
    "$(COMPILE.cc) $(OUTPUT_OPTION) $<",
    ".C.o",
    "$(COMPILE.C) $(OUTPUT_OPTION) $<",
    ".cpp.o",
    "$(COMPILE.cpp) $(OUTPUT_OPTION) $<",
    ".f.o",
    "$(COMPILE.f) $(OUTPUT_OPTION) $<",
    ".p.o",
    "$(COMPILE.p) $(OUTPUT_OPTION) $<",
    ".F.o",
    "$(COMPILE.F) $(OUTPUT_OPTION) $<",
    ".r.o",
    "$(COMPILE.r) $(OUTPUT_OPTION) $<",
    ".mod.o",
    "$(COMPILE.mod) -o $@ $<",

    ".c.ln",
    "$(LINT.c) -C$* $<",
    ".y.ln",
#ifndef __MSDOS__
    "$(YACC.y) $< \n $(LINT.c) -C$* y.tab.c \n $(RM) y.tab.c",
#else
    "$(YACC.y) $< \n $(LINT.c) -C$* y_tab.c \n $(RM) y_tab.c",
#endif
    ".l.ln",
    "@$(RM) $*.c\n $(LEX.l) $< > $*.c\n$(LINT.c) -i $*.c -o $@\n $(RM) $*.c",

    ".y.c",
#ifndef __MSDOS__
    "$(YACC.y) $< \n mv -f y.tab.c $@",
#else
    "$(YACC.y) $< \n mv -f y_tab.c $@",
#endif
    ".l.c",
    "@$(RM) $@ \n $(LEX.l) $< > $@",

    ".F.f",
    "$(PREPROCESS.F) $(OUTPUT_OPTION) $<",
    ".r.f",
    "$(PREPROCESS.r) $(OUTPUT_OPTION) $<",

    /* This might actually make lex.yy.c if there's no %R%
       directive in $*.l, but in that case why were you
       trying to make $*.r anyway?  */
    ".l.r",
    "$(LEX.l) $< > $@ \n mv -f lex.yy.r $@",

    ".S.s",
    "$(PREPROCESS.S) $< > $@",

    ".texinfo.info",
    "$(MAKEINFO) $(MAKEINFO_FLAGS) $< -o $@",

    ".texi.info",
    "$(MAKEINFO) $(MAKEINFO_FLAGS) $< -o $@",

    ".txinfo.info",
    "$(MAKEINFO) $(MAKEINFO_FLAGS) $< -o $@",

    ".tex.dvi",
    "$(TEX) $<",

    ".texinfo.dvi",
    "$(TEXI2DVI) $(TEXI2DVI_FLAGS) $<",

    ".texi.dvi",
    "$(TEXI2DVI) $(TEXI2DVI_FLAGS) $<",

    ".txinfo.dvi",
    "$(TEXI2DVI) $(TEXI2DVI_FLAGS) $<",

    ".w.c",
    "$(CTANGLE) $< - $@",	/* The `-' says there is no `.ch' file.  */

    ".web.p",
    "$(TANGLE) $<",

    ".w.tex",
    "$(CWEAVE) $< - $@",	/* The `-' says there is no `.ch' file.  */

    ".web.tex",
    "$(WEAVE) $<",

#endif /* !VMS */

    0, 0,
  };

static char *default_variables[] =
  {
#ifdef VMS
#ifdef __ALPHA
    "ARCH", WIDE("ALPHA"),
#else
    "ARCH", WIDE("VAX"),
#endif
    "AR", WIDE("library/obj"),
    "ARFLAGS", WIDE("/replace"),
    "AS", WIDE("macro"),
    "MACRO", WIDE("macro"),
#ifdef GCC_IS_NATIVE
    "CC", WIDE("gcc"),
#else
    "CC", WIDE("cc"),
#endif
    "CD", WIDE("builtin_cd"),
    "MAKE", WIDE("make"),
    "ECHO", WIDE("write sys$$output \")",
#ifdef GCC_IS_NATIVE
    "C++", WIDE("gcc/plus"),
    "CXX", WIDE("gcc/plus"),
#else
    "C++", WIDE("cxx"),
    "CXX", WIDE("cxx"),
    "CXXLD", WIDE("cxxlink"),
#endif
    "CO", WIDE("co"),
    "CPP", WIDE("$(CC) /preprocess_only"),
    "FC", WIDE("fortran"),
    /* System V uses these, so explicit rules using them should work.
       However, there is no way to make implicit rules use them and FC.  */
    "F77", WIDE("$(FC)"),
    "F77FLAGS", WIDE("$(FFLAGS)"),
    "LD", WIDE("link"),
    "LEX", WIDE("lex"),
    "PC", WIDE("pascal"),
    "YACC", WIDE("bison/yacc"),
    "YFLAGS", WIDE("/Define/Verbose"),
    "BISON", WIDE("bison"),
    "MAKEINFO", WIDE("makeinfo"),
    "TEX", WIDE("tex"),
    "TEXINDEX", WIDE("texindex"),

    "RM", WIDE("delete/nolog"),

    "CSTARTUP", WIDE(""),
#ifdef GCC_IS_NATIVE
    "CRT0", WIDE(",sys$$library:vaxcrtl.olb/lib,gnu_cc_library:crt0.obj"),
    "CXXSTARTUP", WIDE("gnu_cc_library:crtbegin.obj"),
    "CXXRT0", WIDE(",sys$$library:vaxcrtl.olb/lib,gnu_cc_library:crtend.obj,gnu_cc_library:gxx_main.obj"),
    "LXLIBS", WIDE(",gnu_cc_library:libstdcxx.olb/lib,gnu_cc_library:libgccplus.olb/lib"),
    "LDLIBS", WIDE(",gnu_cc_library:libgcc.olb/lib"),
#else
    "CRT0", WIDE(""),
    "CXXSTARTUP", WIDE(""),
    "CXXRT0", WIDE(""),
    "LXLIBS", WIDE(""),
    "LDLIBS", WIDE(""),
#endif

    "LINK.obj", WIDE("$(LD) $(LDFLAGS)"),
#ifndef GCC_IS_NATIVE
    "CXXLINK.obj", WIDE("$(CXXLD) $(LDFLAGS)"),
    "COMPILE.cxx", WIDE("$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH)"),
#endif
    "COMPILE.c", WIDE("$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH)"),
    "COMPILE.cc", WIDE("$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH)"),
    "YACC.y", WIDE("$(YACC) $(YFLAGS)"),
    "LEX.l", WIDE("$(LEX) $(LFLAGS)"),
    "COMPILE.for", WIDE("$(FC) $(FFLAGS) $(TARGET_ARCH)"),
    "COMPILE.pas", WIDE("$(PC) $(PFLAGS) $(CPPFLAGS) $(TARGET_ARCH)"),
    "COMPILE.mar", WIDE("$(MACRO) $(MACROFLAGS)"),
    "COMPILE.s", WIDE("$(AS) $(ASFLAGS) $(TARGET_MACH)"),
    "LINT.c", WIDE("$(LINT) $(LINTFLAGS) $(CPPFLAGS) $(TARGET_ARCH)"),

    "MV", WIDE("rename/new_version"),
    "CP", WIDE("copy"),

#else /* !VMS */

    "AR", WIDE("ar"),
    "ARFLAGS", WIDE("rv"),
    "AS", WIDE("as"),
#ifdef GCC_IS_NATIVE
    "CC", WIDE("gcc"),
# ifdef __MSDOS__
    "CXX", WIDE("gpp"),	/* g++ is an invalid name on MSDOS */
# else
    "CXX", WIDE("gcc"),
# endif /* __MSDOS__ */
#else
    "CC", WIDE("cc"),
    "CXX", WIDE("g++"),
#endif

    /* This expands to $(CO) $(COFLAGS) $< $@ if $@ does not exist,
       and to the empty string if $@ does exist.  */
    "CHECKOUT,v", WIDE("+$(if $(wildcard $@),,$(CO) $(COFLAGS) $< $@)"),
    "CO", WIDE("co"),
    "COFLAGS", WIDE(""),

    "CPP", WIDE("$(CC) -E"),
#ifdef	CRAY
    "CF77PPFLAGS", WIDE("-P"),
    "CF77PP", WIDE("/lib/cpp"),
    "CFT", WIDE("cft77"),
    "CF", WIDE("cf77"),
    "FC", WIDE("$(CF)"),
#else	/* Not CRAY.  */
#ifdef	_IBMR2
    "FC", WIDE("xlf"),
#else
#ifdef	__convex__
    "FC", WIDE("fc"),
#else
    "FC", WIDE("f77"),
#endif /* __convex__ */
#endif /* _IBMR2 */
    /* System V uses these, so explicit rules using them should work.
       However, there is no way to make implicit rules use them and FC.  */
    "F77", WIDE("$(FC)"),
    "F77FLAGS", WIDE("$(FFLAGS)"),
#endif	/* Cray.  */
    "GET", SCCS_GET,
    "LD", WIDE("ld"),
#ifdef GCC_IS_NATIVE
    "LEX", WIDE("flex"),
#else
    "LEX", WIDE("lex"),
#endif
    "LINT", WIDE("lint"),
    "M2C", WIDE("m2c"),
#ifdef	pyr
    "PC", WIDE("pascal"),
#else
#ifdef	CRAY
    "PC", WIDE("PASCAL"),
    "SEGLDR", WIDE("segldr"),
#else
    "PC", WIDE("pc"),
#endif	/* CRAY.  */
#endif	/* pyr.  */
#ifdef GCC_IS_NATIVE
    "YACC", WIDE("bison -y"),
#else
    "YACC", WIDE("yacc"),	/* Or "bison -y"  */
#endif
    "MAKEINFO", WIDE("makeinfo"),
    "TEX", WIDE("tex"),
    "TEXI2DVI", WIDE("texi2dvi"),
    "WEAVE", WIDE("weave"),
    "CWEAVE", WIDE("cweave"),
    "TANGLE", WIDE("tangle"),
    "CTANGLE", WIDE("ctangle"),

    "RM", WIDE("rm -f"),

    "LINK.o", WIDE("$(CC) $(LDFLAGS) $(TARGET_ARCH)"),
    "COMPILE.c", WIDE("$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c"),
    "LINK.c", WIDE("$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)"),
    "COMPILE.cc", WIDE("$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c"),
    "COMPILE.C", WIDE("$(COMPILE.cc)"),
    "COMPILE.cpp", WIDE("$(COMPILE.cc)"),
    "LINK.cc", WIDE("$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)"),
    "LINK.C", WIDE("$(LINK.cc)"),
    "LINK.cpp", WIDE("$(LINK.cc)"),
    "YACC.y", WIDE("$(YACC) $(YFLAGS)"),
    "LEX.l", WIDE("$(LEX) $(LFLAGS) -t"),
    "COMPILE.f", WIDE("$(FC) $(FFLAGS) $(TARGET_ARCH) -c"),
    "LINK.f", WIDE("$(FC) $(FFLAGS) $(LDFLAGS) $(TARGET_ARCH)"),
    "COMPILE.F", WIDE("$(FC) $(FFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c"),
    "LINK.F", WIDE("$(FC) $(FFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)"),
    "COMPILE.r", WIDE("$(FC) $(FFLAGS) $(RFLAGS) $(TARGET_ARCH) -c"),
    "LINK.r", WIDE("$(FC) $(FFLAGS) $(RFLAGS) $(LDFLAGS) $(TARGET_ARCH)"),
    "COMPILE.def", WIDE("$(M2C) $(M2FLAGS) $(DEFFLAGS) $(TARGET_ARCH)"),
    "COMPILE.mod", WIDE("$(M2C) $(M2FLAGS) $(MODFLAGS) $(TARGET_ARCH)"),
    "COMPILE.p", WIDE("$(PC) $(PFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c"),
    "LINK.p", WIDE("$(PC) $(PFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)"),
    "LINK.s", WIDE("$(CC) $(ASFLAGS) $(LDFLAGS) $(TARGET_MACH)"),
    "COMPILE.s", WIDE("$(AS) $(ASFLAGS) $(TARGET_MACH)"),
    "LINK.S", WIDE("$(CC) $(ASFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_MACH)"),
    "COMPILE.S", WIDE("$(CC) $(ASFLAGS) $(CPPFLAGS) $(TARGET_MACH) -c"),
    "PREPROCESS.S", WIDE("$(CC) -E $(CPPFLAGS)"),
    "PREPROCESS.F", WIDE("$(FC) $(FFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -F"),
    "PREPROCESS.r", WIDE("$(FC) $(FFLAGS) $(RFLAGS) $(TARGET_ARCH) -F"),
    "LINT.c", WIDE("$(LINT) $(LINTFLAGS) $(CPPFLAGS) $(TARGET_ARCH)"),

#ifndef	NO_MINUS_C_MINUS_O
    "OUTPUT_OPTION", WIDE("-o $@"),
#endif

#ifdef	SCCS_GET_MINUS_G
    "SCCS_OUTPUT_OPTION", WIDE("-G$@"),
#endif

#ifdef _AMIGA
    ".LIBPATTERNS", WIDE("%.lib"),
#else
#ifdef __MSDOS__
    ".LIBPATTERNS", WIDE("lib%.a $(DJDIR)/lib/lib%.a"),
#else
    ".LIBPATTERNS", WIDE("lib%.so lib%.a"),
#endif
#endif

#endif /* !VMS */
    0, 0
  };

/* Set up the default .SUFFIXES list.  */

void
set_default_suffixes ()
{
  suffix_file = enter_file (".SUFFIXES");

  if (no_builtin_rules_flag)
    (void) define_variable ("SUFFIXES", 8, WIDE(""), o_default, 0);
  else
    {
      char *p = default_suffixes;
      suffix_file->deps = (struct dep *)
	multi_glob (parse_file_seq (&p, '\0', sizeof (struct dep), 1),
		    sizeof (struct dep));
      (void) define_variable ("SUFFIXES", 8, default_suffixes, o_default, 0);
    }
}

/* Enter the default suffix rules as file rules.  This used to be done in
   install_default_implicit_rules, but that loses because we want the
   suffix rules installed before reading makefiles, and thee pattern rules
   installed after.  */

void
install_default_suffix_rules ()
{
  register char **s;

  if (no_builtin_rules_flag)
    return;

 for (s = default_suffix_rules; *s != 0; s += 2)
    {
      register struct file *f = enter_file (s[0]);
      /* Don't clobber cmds given in a makefile if there were any.  */
      if (f->cmds == 0)
	{
	  f->cmds = (struct commands *) xmalloc (sizeof (struct commands));
	  f->cmds->fileinfo.filenm = 0;
	  f->cmds->commands = s[1];
	  f->cmds->command_lines = 0;
	}
    }
}


/* Install the default pattern rules.  */

void
install_default_implicit_rules ()
{
  register struct pspec *p;

  if (no_builtin_rules_flag)
    return;

  for (p = default_pattern_rules; p->target != 0; ++p)
    install_pattern_rule (p, 0);

  for (p = default_terminal_rules; p->target != 0; ++p)
    install_pattern_rule (p, 1);
}

void
define_default_variables ()
{
  register char **s;

  if (no_builtin_variables_flag)
    return;

  for (s = default_variables; *s != 0; s += 2)
    (void) define_variable (s[0], strlen (s[0]), s[1], o_default, 1);
}
