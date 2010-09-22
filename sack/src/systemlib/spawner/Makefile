
PROGNAME1=spawner
SRCS1=spawner
MORELIBS1=osalot configscript shmem networkservice
ifndef __WINDOWS__

else
# handle static linking...
MORELIBS1+=syslog containers timers filesys network idle systray
SYSLIBS1+=wsock32
endif
SUBSYSTEM=windows

#PROGNAME2=spawncon
SRCS2=spawncon



include $(SACK_BASE)/makefiles/makefile.many
