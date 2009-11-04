
#FONT=LucidaConsole40by24
FONT=LucidaSansUnicode13by26


test.exe: test.obj myfont.obj 
	lcclnk test.obj myfont.obj -L/common/lib/debug videos.lib images.lib shmems.lib syslogs.lib wsock32.lib jpegs.lib pngs.lib zlibs.lib 
	

test.obj: test.c makefile
	lcc -g -D__STATIC__ -I/common/include -DUSEFONT=$(FONT) $<
	
myfont.obj: ../$(FONT).c makefile
	lcc -I/common/include -FoMyFont.obj $<