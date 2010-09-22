lcc -g -D_DEBUG -D__STATIC__ -I../../../include extent_test.c
lcclnk extent_test.obj -L../../../lib/debug shmems.lib syslogs.lib wsock32.lib 