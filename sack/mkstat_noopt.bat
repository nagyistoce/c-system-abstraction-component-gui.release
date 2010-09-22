:set NOTQUIET=1
set MAKE=make
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/SQLlib/testsql
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/utils/shell
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/intershell/widgets
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/intershell/widgets/banner/banner_cmd
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/utils/launcher
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/utils/top_picker
%MAKE% __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_sql_ -C src/streamlib
