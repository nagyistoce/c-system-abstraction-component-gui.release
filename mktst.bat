:set NOTQUIET=1
set MAKE=make
%MAKE% __NO_SQL__=1 __NO_GUI__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_no_gui_
%MAKE% __NO_SQL__=1 __NO_GUI__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_no_gui_ -C src/utils/shell
%MAKE% __NO_SQL__=1 __NO_GUI__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_no_gui_ -C src/streamlib
