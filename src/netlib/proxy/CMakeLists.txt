PROJECT( proxy )

add_executable( ${PROJECT_NAME} ${FIRST_GCC_PROGRAM_SOURCE} proxy.c ${LAST_GCC_PROGRAM_SOURCE} )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
                  FOLDER "Network examples"
)
install_default_dest( ${PROJECT_NAME} )


PROJECT( proxy.portable )
if( __WINDOWS__ )
set( NET_SOURCES     ../net_winsock2.c  ) 
endif( __WINDOWS__ )

if( MSVC ) 
SET( ExtraSources ../../systemlib/args.c )
endif( MSVC )

set( my_add_definitions "-D__NO_GUI__ -D__NO_OPTIONS__ -D__STATIC__" )
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
                  COMPILE_FLAGS  "${my_add_definitions} -DTARGET_LABEL=${TARGET_LABEL} ${CORE_SOURCE_FLAGS}"
                  FOLDER "Network examples"
)
install_default_dest( ${PROJECT_NAME} )


PROJECT( proxy.service )
if( __WINDOWS__ )
set( NET_SOURCES     ../net_winsock2.c  ) 
endif( __WINDOWS__ )

if( MSVC ) 
SET( ExtraSources ../../systemlib/args.c )
endif( MSVC )

set( SOURCES_ROOT ../.. )

if( SNPRINTF_SOURCES )
set( SNPRINTF_SOURCES    ${SOURCES_ROOT}/../${SNPRINTF_SOURCES} )
endif(SNPRINTF_SOURCES)


add_executable( ${PROJECT_NAME} ${FIRST_GCC_PROGRAM_SOURCE} 
    proxy.c 
    ../network.c 
    ../udpnetwork.c 
    ../tcpnetwork.c 
    ${NET_SOURCES}
    ${SOURCES_ROOT}/memlib/sharemem.c
    ${SOURCES_ROOT}/timerlib/timers.c
    ${SOURCES_ROOT}/idlelib/idle.c
    ${SOURCES_ROOT}/procreglib/names.c
    ${SOURCES_ROOT}/configlib/configscript.c
    ${SOURCES_ROOT}/sysloglib/syslog.c
    ${ExtraSources}
    ${SOURCES_ROOT}/systemlib/system.c
    ${SOURCES_ROOT}/typelib/typecode.c
    ${SOURCES_ROOT}/typelib/text.c
    ${SOURCES_ROOT}/typelib/sets.c
    ${SOURCES_ROOT}/typelib/binarylist.c
    ${SOURCES_ROOT}/filesyslib/filescan.c
    ${SOURCES_ROOT}/filesyslib/pathops.c
    ${SOURCES_ROOT}/filesyslib/winfiles.c
    ${SOURCES_ROOT}/utils/service/service_stub.c
    ${SOURCES_ROOT}/systemlib/args.c
    ${SOURCES_ROOT}/systemlib/spawntask.c
    ${SNPRINTF_SOURCES}
${LAST_GCC_PROGRAM_SOURCE} 
    ${SOURCES_ROOT}/deadstart/deadstart_core.c
)
target_link_libraries( ${PROJECT_NAME} wsock32 ws2_32 winmm )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
                  COMPILE_DEFINITIONS "SACK_BAG_EXPORTS;BUILD_SERVICE;__STATIC__;__NO_GUI__;__NO_OPTIONS__"
                  FOLDER "Network examples"
)
install_default_dest( ${PROJECT_NAME} )


