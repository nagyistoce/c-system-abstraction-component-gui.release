
PROGNAME=image_colorizer
SRCS=image_colorizer imagepng
MORELIBS=bag.image
SYSLIBS=$(if $(__LINUX__),png,)

PROGNAME2=file_image_colorizer
SRCS2=file_image_colorizer imagepng
MORELIBS2=bag.image
SYSLIBS2=$(if $(__LINUX__),png,)

include $(SACK_BASE)/makefiles/makefile.single
