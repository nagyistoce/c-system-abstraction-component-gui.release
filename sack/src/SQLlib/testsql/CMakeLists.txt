
cmake_minimum_required(VERSION 2.8)

project( sqlcmd )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    sqlcmd
${LAST_GCC_PROGRAM_SOURCE}
)


add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" 
                  FOLDER utils
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} ${SACK_PSI_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )


project( sqlcmd.portable )

if( __WINDOWS__ )
set(NET_SOURCES 
  ../../netlib/net_winsock2.c
)
endif()

set( SOURCES_ROOT ../.. )

if( SNPRINTF_SOURCES )
set( SNPRINTF_SOURCES    ${SOURCES_ROOT}/../${SNPRINTF_SOURCES} )
endif(SNPRINTF_SOURCES)


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    sqlcmd
#    ${SOURCES_ROOT}/netlib/network.c 
#    ${SOURCES_ROOT}/netlib/udpnetwork.c 
#    ${SOURCES_ROOT}/netlib/tcpnetwork.c 
#    ${NET_SOURCES} 
#    ${SOURCES_ROOT}/msgsvr/client/client.c
    ${SOURCES_ROOT}/SQLlib/sqlstub.c
    ${SOURCES_ROOT}/SQLlib/sqlutil.c
    ${SOURCES_ROOT}/SQLlib/sqlwrap.c
    ${SOURCES_ROOT}/SQLlib/sqlparse3.c
    ${SOURCES_ROOT}/sqlite/3.7.3/sqlite3.c
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
    ${SOURCES_ROOT}/typelib/msgqueue.c
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
    COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}" 
                  COMPILE_DEFINITIONS "SACK_BAG_EXPORTS;__STATIC__;__NO_GUI__;__NO_OPTIONS__;USE_ODBC;__DISABLE_UDP_SYSLOG__;__NO_MSGSVR__"
                  FOLDER utils
)
if( WIN32 )
#target_link_libraries( ${PROJECT_NAME} winmm wsock32 ws2_32 )
target_link_libraries( ${PROJECT_NAME} winmm odbc32 )
endif( WIN32 )
install_default_dest_binary( ${PROJECT_NAME} )





