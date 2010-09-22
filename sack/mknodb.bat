:set NOTQUIET=1
set OPTIONS="__NO_SQL__=1" __NO_OPTIONS__=1
pmakes %OPTIONS% DEST=static_release DEST_PREFIX=nosql_static_
pmakes %OPTIONS% DEST=static_release DEST_PREFIX=nosql_static_ -C src/utils/shell
pmakes %OPTIONS% DEST=static_release DEST_PREFIX=nosql_static_ -C src/streamlib
