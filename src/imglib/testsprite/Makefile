
APP_DEFAULT_DATA1=$(foreach img,$(shell ls images),images/$(img))

PROGNAME1=testsprite
SRCS1=test
ifdef __WINDOWS__
MORELIBS1=bag.image.plugin bag.video.plugin
else                 
MORELIBS1=bag.display
endif

PROGNAME2=testsprite2
SRCS2=clicktest
ifdef __WINDOWS__
MORELIBS2=bag.image.plugin bag.video.plugin
else                 
MORELIBS2=bag.display
endif

include $(SACK_BASE)/makefiles/makefile.many

