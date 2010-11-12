lcc -I/common/include alphatab.c
lcclnk -L/common/lib/debug alphatab.obj video.lib image.lib
lcc mkalphatab.c
lcclnk mkalphatab.obj