lcc -g -I\common\include -D_DEBUG -D__STATIC__ cmdline.c
lcclnk /out:scmdline.exe cmdline.obj  \common\lib\debug\shmems.lib \common\lib\debug\syslogs.lib wsock32.lib 
lcc -g -I\common\include -D_DEBUG cmdline.c
lcclnk /out:cmdline.exe cmdline.obj  \common\lib\debug\shmem.lib  \common\lib\debug\syslog.lib
copy \common\dll\debug\shmem.dll .
copy \common\dll\debug\syslog.dll .
