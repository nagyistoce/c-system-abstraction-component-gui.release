make DEST=static_release -f makefile.all -C ../../.. DEST_PREFIX=sack_

make SACK_BUILD=1 -f Makefile.test DEST=static_release DEST_PREFIX=sack_
