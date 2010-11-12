

#MORE_TARGETS+=sqliteh.h

all .DEFAULT: #sqlite3.c
	make -f Makefile.bag


sqlite3.c: sqlite-amalgamation-3_4_1.zip
	-pkzip25 -ext -dir sqlite-amalgamation-3_4_1.zip 
	touch sqlite3.c
	touch sqlite3.h

