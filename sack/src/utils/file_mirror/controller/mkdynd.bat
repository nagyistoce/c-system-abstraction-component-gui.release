mkdir debug
lcc -g -I\common\include -Fodebug\rcontrol.obj rcontrol.c
lcclnk  -out:debug/rcontrol.exe debug/rcontrol.obj  -L\common\lib\debug\ network.lib shmem.lib syslog.lib typelib.lib wsock32.lib
