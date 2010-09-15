PROJECT( proxy )

add_executable( ${PROJECT_NAME} ${FIRST_GCC_PROGRAM_SOURCE} proxy.c ${LAST_GCC_PROGRAM_SOURCE} )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
                  FOLDER "Network examples"
)

PROJECT( proxy.portable )
if( __WINDOWS__ )
set( NET_SOURCES     ../net_winsock2.c  ) 
endif( __WINDOWS__ )

if( MSVC ) 
SET( ExtraSources ../../systemlib/args.c )
endif( MSVC )

add_definitions( -D__NO_GUI__ -D__NO_OPTIONS__ )
add_executable( ${PROJECT_NAME} ${FIRST_GCC_PROGRAM_SOURCE} 
    proxy.c 
    ../network.c 
    ../udpnetwork.c 
    ../tcpnetwork.c 
    ${NET_SOURCES}
    ../../memlib/sharemem.c
    ../../timerlib/timers.c
    ../../idlelib/idle.c
    ../../procreglib/names.c
    ../../configlib/configscript.c
    ../../sysloglib/syslog.c
    ${ExtraSources}
    ../../systemlib/system.c
    ../../typelib/typecode.c
    ../../typelib/text.c
    ../../typelib/sets.c
    ../../typelib/binarylist.c
    ../../filesyslib/filescan.c
    ../../filesyslib/pathops.c
    ../../filesyslib/winfiles.c
${LAST_GCC_PROGRAM_SOURCE} 
    ../../deadstart/deadstart_core.c
)
set( CORE_SOURCE_FLAGS
   -DSACK_BAG_EXPORTS
   -DNETWORK_SOURCE
   -DTIMER_SOURCE
   -DSYSTEM_SOURCE
   -DIDLE_SOURCE
   -DFILESYS_SOURCE
   -DSYSLOG_SOURCE
   -DCONFIGLIB_SOURCE
   )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${CORE_SOURCE_FLAGS}"
                  FOLDER "Network examples"
)

