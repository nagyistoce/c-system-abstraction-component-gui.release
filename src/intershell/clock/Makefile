

INCLUDEDIRS=..

APP_DEFAULT_DATA1=$(foreach frame,$(shell ls frames),frames/$(frame))  \
	$(foreach frame,$(shell ls images),images/$(frame)) 

LITERAL_LIBNAME5=clock.isp
SRCS5=clock_addon


include $(SACK_BASE)/makefiles/makefile.many

