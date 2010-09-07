

##############################
# test 1 : 3 windows - no content
##############################
#ifndef __WINDOWS__
PROGNAME1=test
SRCS1=test
#SACK_LIBS1=procreg
#SACK_LIBS1+=msgclient shmem syslog timers containers
ifndef __WINDOWS__
SYSLIBS1=pthread
else
SYSLIBS1=wsock32
SYSLIBS2=wsock32
SYSLIBS3=wsock32
SYSLIBS4=wsock32
SYSLIBS5=wsock32
SYSLIBS6=wsock32
SYSLIBS7=wsock32
SYSLIBS8=wsock32
SYSLIBS9=wsock32
SYSLIBSA=wsock32
endif
#SACK_LIBS+=msgclient shmem syslog timers

##############################
# test 2 : 3 windows - draw lines in one - but not on redraw
##############################
PROGNAME2=test2
SRCS2=test2
#SACK_LIBS2=procreg
#SACK_LIBS2+=msgclient shmem syslog timers containers
ifndef __WINDOWS__
SYSLIBS2=pthread
endif

##############################
# test 3 : 3 windows - open color selector dialog
##############################
PROGNAME3=test3
INCLUDEDIRS=$(SACK_BASE)/include
SRCS3=test3
#MORELIBS3=bag.psi #procreg
ifndef __WINDOWS__
SYSLIBS3=pthread
endif

##############################
# test 4 : 3 windows - open color selector and another window (clip1)
##############################
PROGNAME4=test4
SRCS4=test4
#MORELIBS4=bag.psi #procreg
ifndef __WINDOWS__
SYSLIBS4=pthread
endif

##############################
# test 5 : cmd line # windows - one blue, one orange - test blat color redraw
##############################
PROGNAME5=test5
SRCS5=test5
ifndef __WINDOWS__
SYSLIBS5=pthread
endif

##############################
# test 6 : 2 panels - 2 frame controls with no content (title/frame)
##############################
PROGNAME6=test6
SRCS6=test6
#MORELIBS6=bag.psi #procreg
ifndef __WINDOWS__
SYSLIBS6=pthread
endif

##############################
# test7 : 1 frame - edit control (keys), listbox (scrollbars)
##############################
PROGNAME7=test7
SRCS7=test7
#MORELIBS7=bag.psi #procreg
ifndef __WINDOWS__
SYSLIBS7=pthread
endif

##############################
# test8 : 1 display - size of image on command line, loaded and displayed
##############################
PROGNAME8=test8
SRCS8=test8
ifndef __WINDOWS__
SYSLIBS8=pthread
endif

##############################
# testblot : show a background image, profile alpha blot, non alpha blot
##############################
APP_DEFAULT_DATA9=sky.jpg daub.png
PROGNAME9=testblot
SRCS9=testblot
ifndef __WINDOWS__
else
SYSLIBS9=wsock32 
endif

APP_DEFAULT_DATAE=sky.jpg
PROGNAMEE=testblot2
SRCSE=testblot2
ifndef __WINDOWS__
else
SYSLIBSE=wsock32 
endif

##############################
# testblur : show some quick and dirty slot reals with cheap blur effect
##############################
APP_DEFAULT_DATAA=slot_strip.jpg background.jpg slot_strip.2.jpg
PROGNAMEA=testblur
SRCSA=testblur
ifndef __WINDOWS__
else
SYSLIBSA=wsock32
endif


##############################
# testblot : show a background image, profile alpha blot, non alpha blot
##############################
#APP_DEFAULT_DATA9=sky.jpg daub.png
PROGNAMEB=testsprite
SRCSB=testsprite
ifndef __WINDOWS__
else
SYSLIBSB=wsock32 
endif

##############################
# testslot : show some quick and dirty slot reals with cheap blur effect
##############################
APP_DEFAULT_DATAC=slot_strip.jpg background.jpg slot_strip.2.jpg playing.jpg playagain.jpg
PROGNAMEC=testslot
SRCSC=slotgame
ifndef __WINDOWS__
else
SYSLIBSC=wsock32
endif

##############################
# testslot : show some quick and dirty slot reals with cheap blur effect
##############################
# uses testslot resources
APP_DEFAULT_DATAC= 
PROGNAMED=testslot2
SRCSD=slotgame2

#MORELIBS1+=network procreg configscript fraction osalot filesys idle
#MORELIBS2+=network procreg configscript fraction osalot filesys idle
#MORELIBS3+=network procreg configscript fraction osalot filesys idle
#MORELIBS4+=network procreg configscript fraction osalot filesys idle
#MORELIBS5+=network procreg configscript fraction osalot filesys idle
#MORELIBS6+=network procreg configscript fraction osalot filesys idle
#MORELIBS7+=network procreg configscript fraction osalot filesys idle
#MORELIBS8+=network procreg configscript fraction osalot filesys idle
#MORELIBS9+=network procreg configscript fraction osalot filesys idle
#MORELIBSA+=network procreg configscript fraction osalot filesys idle
#MORELIBSB+=network procreg configscript fraction osalot filesys idle


include $(SACK_BASE)/makefiles/makefile.many
