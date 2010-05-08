:set NOTQUIET=1
set MAKE=make
set __NO_GUI__=1
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_no_gui_
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_no_gui_ -C src/SQLlib/testsql
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_no_gui_ -C src/utils/shell
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_no_gui_ -C src/utils/launcher
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_no_gui_ -C src/utils/top_picker
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_no_gui_ -C src/streamlib
