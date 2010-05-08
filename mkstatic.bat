:set NOTQUIET=1
set MAKE=make
set __NO_SQL__=1
set __NO_OPTIONS__=1
set DEST=static_release
set DEST_PREFIX=no_opt_

%MAKE% __NO_SQL__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_
:cmd
%MAKE% __NO_SQL__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_ -C src/utils/launcher
%MAKE% __NO_SQL__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_ -C src/utils/shell
%MAKE% __NO_SQL__=1 __NO_OPTIONS__=1 DEST=static_release DEST_PREFIX=no_opt_ -C src/streamlib
