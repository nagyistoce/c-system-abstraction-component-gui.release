:set NOTQUIET=1
set DEST_PREFIX=con_no_opt_
set __NO_GUI__=1
set __NO_OPTIONS__=1
set MAKE=make
%MAKE% DEST=static_release 
%MAKE% DEST=static_release -C src/SQLlib/testsql
%MAKE% DEST=static_release -C src/utils/shell
%MAKE% DEST=static_release -C src/utils/launcher
%MAKE% DEST=static_release -C src/utils/top_picker
%MAKE% DEST=static_release -C ../altanik/src/utils/aristocrat_convert