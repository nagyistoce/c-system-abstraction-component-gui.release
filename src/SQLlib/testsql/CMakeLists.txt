
cmake_minimum_required(VERSION 2.8)

project( sqlcmd )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    sqlcmd
${LAST_GCC_PROGRAM_SOURCE}
)

if( C++ )
set_source_files_properties( ${BASE_SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} ${SACK_PSI_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )


project( sqlcmd.portable )

if( __WINDOWS__ )
set(NET_SOURCES 
  ../../netlib/net_winsock2.c
)
endif()


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    sqlcmd
    ../../netlib/network.c 
    ../../netlib/udpnetwork.c 
    ../../netlib/tcpnetwork.c 
    ${NET_SOURCES} 
    ../../msgsvr/client/client.c
    ../../SQLlib/sqlstub.c
    ../../SQLlib/sqlutil.c
    ../../SQLlib/sqlwrap.c
    ../../SQLlib/sqlparse3.c
    ../../sqlite/3.7.0/sqlite3.c
    ../../memlib/sharemem.c
    ../../timerlib/timers.c
    ../../idlelib/idle.c
    ../../procreglib/names.c
    ../../configlib/configscript.c
    ../../sysloglib/syslog.c
    ../../systemlib/system.c
    ../../typelib/typecode.c
    ../../typelib/text.c
    ../../typelib/sets.c
    ../../typelib/binarylist.c
    ../../typelib/msgqueue.c
    ../../filesyslib/filescan.c
    ../../filesyslib/pathops.c
    ../../filesyslib/winfiles.c
${LAST_GCC_PROGRAM_SOURCE}
    ../../deadstart/deadstart_core.c 
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -D__STATIC__ -DCLIENTMSG_SOURCE -DNETWORK_SOURCE -DSQLSTUB_SOURCE -DMEM_LIBRARY_SOURCE -DSYSLOG_SOURCE -D_TYPELIBRARY_SOURCE -DSYSTEM_SOURCE -DFILESYSTEM_LIBRARY_SOURCE -DTIMER_SOURCE -DIDLE_SOURCE -DPROCREG_SOURCE -DCONFIGURATION_LIBRARY_SOURCE -DSACK_BAG_CORE_EXPORTS -D__NO_OPTIONS__" 
)
if( __WINDOWS__ )
target_link_libraries( ${PROJECT_NAME} winmm wsock32 ws2_32 )
endif( __WINDOWS__ )
install_default_dest_binary( ${PROJECT_NAME} )





