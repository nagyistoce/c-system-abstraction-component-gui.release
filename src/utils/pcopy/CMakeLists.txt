
cmake_minimum_required(VERSION 2.8)

project( pcopy )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    pcopy docopy
${LAST_GCC_PROGRAM_SOURCE}
)



add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" 
                  FOLDER utils
)

target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest_binary( ${PROJECT_NAME} )

project( pcopy.portable )

set( SOURCES_ROOT ../.. )

if( SNPRINTF_SOURCES )
set( SNPRINTF_SOURCES    ${SOURCES_ROOT}/../${SNPRINTF_SOURCES} )
endif(SNPRINTF_SOURCES)

set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    pcopy docopy
    ${SOURCES_ROOT}/netlib/network.c 
    ${SOURCES_ROOT}/netlib/udpnetwork.c 
    ${SOURCES_ROOT}/netlib/tcpnetwork.c 
    ${SOURCES_ROOT}/netlib/net_winsock2.c 
    ${SOURCES_ROOT}/memlib/sharemem.c
    ${SOURCES_ROOT}/timerlib/timers.c
    ${SOURCES_ROOT}/idlelib/idle.c
    ${SOURCES_ROOT}/procreglib/names.c
    ${SOURCES_ROOT}/configlib/configscript.c
    ${SOURCES_ROOT}/sysloglib/syslog.c
    ${SOURCES_ROOT}/systemlib/system.c
#    ${SOURCES_ROOT}/../systemlib/spawntask.c
    ${SOURCES_ROOT}/typelib/typecode.c
    ${SOURCES_ROOT}/typelib/text.c
    ${SOURCES_ROOT}/typelib/sets.c
    ${SOURCES_ROOT}/typelib/binarylist.c
    ${SOURCES_ROOT}/filesyslib/filescan.c
    ${SOURCES_ROOT}/filesyslib/pathops.c
    ${SOURCES_ROOT}/filesyslib/winfiles.c
    ${SNPRINTF_SOURCES}
${LAST_GCC_PROGRAM_SOURCE}
    ${SOURCES_ROOT}/deadstart/deadstart_core.c 
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} " 
COMPILE_DEFINITIONS "__STATIC__;SACK_BAG_EXPORTS;__NO_OPTIONS__"

                  FOLDER utils
)
target_link_libraries( ${PROJECT_NAME} ws2_32 wsock32 winmm )

install_default_dest_binary( ${PROJECT_NAME} )


