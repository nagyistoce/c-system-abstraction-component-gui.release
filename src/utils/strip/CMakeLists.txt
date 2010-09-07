
cmake_minimum_required(VERSION 2.8)

project( lstrip )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    strip.c
${LAST_GCC_PROGRAM_SOURCE}
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )

project( lstrip.portable )

if( __WINDOWS__ )
set( NET_SOURCES     ../../../netlib/net_winsock2.c  )
endif( __WINDOWS__ )


set( SOURCES_ROOT ../.. )
add_definitions( -D__NO_NETWORK__ )
set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    strip.c
    ${SOURCES_ROOT}/memlib/sharemem.c
    ${SOURCES_ROOT}/timerlib/timers.c
    ${SOURCES_ROOT}/idlelib/idle.c
    ${SOURCES_ROOT}/procreglib/names.c
    ${SOURCES_ROOT}/configlib/configscript.c
    ${SOURCES_ROOT}/sysloglib/syslog.c
    ${SOURCES_ROOT}/systemlib/system.c
    ${SOURCES_ROOT}/typelib/typecode.c
    ${SOURCES_ROOT}/typelib/text.c
    ${SOURCES_ROOT}/typelib/sets.c
    ${SOURCES_ROOT}/typelib/binarylist.c
    ${SOURCES_ROOT}/filesyslib/filescan.c
    ${SOURCES_ROOT}/filesyslib/pathops.c
    ${SOURCES_ROOT}/filesyslib/winfiles.c
${LAST_GCC_PROGRAM_SOURCE}
    ${SOURCES_ROOT}/deadstart/deadstart_core.c 
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -DNETWORK_SOURCE -DMEM_LIBRARY_SOURCE -DSYSLOG_SOURCE -D_TYPELIBRARY_SOURCE -DSYSTEM_SOURCE -DFILESYSTEM_LIBRARY_SOURCE -DTIMER_SOURCE -DIDLE_SOURCE -DPROCREG_SOURCE -DCONFIGURATION_LIBRARY_SOURCE -DSACK_BAG_CORE_EXPORTS -D__NO_OPTIONS__" )
if( __WINDOWS__ )
target_link_libraries( ${PROJECT_NAME} ws2_32 wsock32 winmm)
endif( __WINDOWS__ )
install_default_dest( ${PROJECT_NAME} )


