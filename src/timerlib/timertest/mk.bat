lcc -g -I../../../include -D__STATIC__ timertest.c
lcclnk timertest.obj -L../../../lib/debug timerss.lib shmems.lib syslogs.lib wsock32.lib 