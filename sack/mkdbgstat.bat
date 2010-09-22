:set NOTQUIET=1
set MAKE=make
set DEST=static_debug
%MAKE%  
%MAKE%  -C src/utils/shell
%MAKE%  -C src/utils/launcher
%MAKE%  -C src/utils/top_picker
%MAKE%  -C src/intershell/widgets
%MAKE%  -C src/intershell/widgets/banner/banner_cmd
%MAKE%  -C src/streamlib
