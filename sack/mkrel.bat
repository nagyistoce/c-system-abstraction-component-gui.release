:set NOTQUIET=1
pmakes DEST=static_release DEST_PREFIX=static_
pmakes DEST=static_release DEST_PREFIX=static_ -C src/utils/shell
pmakes DEST=static_release DEST_PREFIX=static_ -C src/streamlib
