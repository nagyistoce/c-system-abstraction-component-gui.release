
cmake_minimum_required(VERSION 2.8)

project( launchcmd )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchcmd
${LAST_GCC_PROGRAM_SOURCE}
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )

project( launchcmd.portable )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchcmd.c
    ../../../netlib/network.c 
    ../../../netlib/udpnetwork.c 
    ../../../netlib/tcpnetwork.c 
    ../../../netlib/net_winsock2.c 
    ../../../memlib/sharemem.c
    ../../../timerlib/timers.c
    ../../../idlelib/idle.c
    ../../../procreglib/names.c
    ../../../configlib/configscript.c
    ../../../sysloglib/syslog.c
    ../../../systemlib/system.c
#    ../../../systemlib/spawntask.c
    ../../../typelib/typecode.c
    ../../../typelib/text.c
    ../../../typelib/sets.c
    ../../../typelib/binarylist.c
    ../../../filesyslib/filescan.c
    ../../../filesyslib/pathops.c
    ../../../filesyslib/winfiles.c
${LAST_GCC_PROGRAM_SOURCE}
    ../../../deadstart/deadstart_core.c 
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -DNETWORK_SOURCE -DMEM_LIBRARY_SOURCE -DSYSLOG_SOURCE -D_TYPELIBRARY_SOURCE -DSYSTEM_SOURCE -DFILESYSTEM_LIBRARY_SOURCE -DTIMER_SOURCE -DIDLE_SOURCE -DPROCREG_SOURCE -DCONFIGURATION_LIBRARY_SOURCE -DSACK_BAG_CORE_EXPORTS -D__NO_OPTIONS__" )
target_link_libraries( ${PROJECT_NAME} ws2_32 wsock32 winmm)
install_default_dest( ${PROJECT_NAME} )


project( launchwin ) 

set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchcmd
${LAST_GCC_PROGRAM_SOURCE}
)

add_executable(${PROJECT_NAME} WIN32 ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )
