
set SACK_BASE=M:\sack
wcc386 -d2 -zq scan_tdump.c
wlink option quiet debug all system nt file scan_tdump
wcc386 -d2 -zq -I%SACK_BASE%\include scan_bin.c
wlink option quiet debug all system nt file scan_bin library %FINALDEST%\lib\debug-wc\filesys.lib
