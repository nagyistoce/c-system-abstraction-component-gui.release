
set TOOLDRIVE=c:
set TOOLS=%TOOLDRIVE%/tools
set TOOLS2=%TOOLDRIVE%\tools
set FTNBASE=c:\ftn3000
set PATH=c:\windows;c:\windows\system32;c:\misc;%TOOLS2%;%FTNBASE%\bin;%FTNBASE%\utility
set CVSEDITOR=fte
set CVS_RSH=ssh
set __WINDOWS__=1
call watcom

echo INVOKE YOUR SHELL HERE :%TOOLDRIVE%\misc\far17\far.exe
cmd.exe