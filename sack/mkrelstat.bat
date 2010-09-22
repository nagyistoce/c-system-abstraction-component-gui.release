:set NOTQUIET=1
set MAKE=make

%MAKE% DEST=static_release 
%MAKE% DEST=static_release -C src/SQLlib/testsql
%MAKE% DEST=static_release -C src/utils/shell
%MAKE% DEST=static_release -C src/utils/launcher
%MAKE% DEST=static_release -C src/utils/top_picker
%MAKE% DEST=static_release -C src/intershell/widgets
%MAKE% DEST=static_release -C src/intershell/widgets/banner/banner_cmd
%MAKE% DEST=static_release -C src/streamlib
