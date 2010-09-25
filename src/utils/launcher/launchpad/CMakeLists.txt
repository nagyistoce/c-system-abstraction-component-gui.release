
cmake_minimum_required(VERSION 2.8)

project( launchpad )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad launchpad.rc
${LAST_GCC_PROGRAM_SOURCE}
)

if( MINGW )

SET(CMAKE_RC_COMPILER_INIT "windres")
SET(CMAKE_RC_COMPILER "windres")
SET(CMAKE_RC_COMPILE_OBJECT "windres <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")
ENABLE_LANGUAGE(RC)

endif()


add_executable(${PROJECT_NAME} WIN32 ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" 
                  FOLDER utils/launcher
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest_binary( ${PROJECT_NAME} )

project( launchpad.portable )


set( SOURCES_ROOT ../../.. )

if( SNPRINTF_SOURCES )
set( SNPRINTF_SOURCES    ${SOURCES_ROOT}/../${SNPRINTF_SOURCES} )
endif(SNPRINTF_SOURCES)

if( __WINDOWS__ )
set( NET_SOURCES     ${SOURCES_ROOT}/netlib/net_winsock2.c  )
endif( __WINDOWS__ )

if( MSVC ) 
SET( ExtraSources ${SOURCES_ROOT}/systemlib/args.c )
endif( MSVC )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad launchpad.rc
    ${SOURCES_ROOT}/netlib/network.c 
    ${SOURCES_ROOT}/netlib/udpnetwork.c 
    ${SOURCES_ROOT}/netlib/tcpnetwork.c 
    ${NET_SOURCES}
    ${SOURCES_ROOT}/memlib/sharemem.c
    ${SOURCES_ROOT}/timerlib/timers.c
    ${SOURCES_ROOT}/idlelib/idle.c
    ${SOURCES_ROOT}/procreglib/names.c
    ${SOURCES_ROOT}/configlib/configscript.c
    ${SOURCES_ROOT}/sysloglib/syslog.c
    ${ExtraSources}
    ${SOURCES_ROOT}/systemlib/system.c
#    ${SOURCES_ROOT}/systemlib/spawntask.c
    ${SOURCES_ROOT}/typelib/typecode.c
    ${SOURCES_ROOT}/typelib/text.c
    ${SOURCES_ROOT}/typelib/sets.c
    ${SOURCES_ROOT}/typelib/binarylist.c
    ${SOURCES_ROOT}/filesyslib/filescan.c
    ${SOURCES_ROOT}/filesyslib/pathops.c
    ${SOURCES_ROOT}/filesyslib/winfiles.c
    ${SOURCES_ROOT}/systemlib/spawntask.c
    ${SOURCES_ROOT}/systemlib/args.c
    ${SOURCES_ROOT}/systraylib/systray.c
    ${SNPRINTF_SOURCES}
${LAST_GCC_PROGRAM_SOURCE}
    ${SOURCES_ROOT}/deadstart/deadstart_core.c 
)

if( MINGW )

SET(CMAKE_RC_COMPILER_INIT "windres")
SET(CMAKE_RC_COMPILER "windres")
SET(CMAKE_RC_COMPILE_OBJECT "windres <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")
ENABLE_LANGUAGE(RC)

endif()


add_executable(${PROJECT_NAME} WIN32 ${BASE_SOURCES} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
                  COMPILE_DEFINITIONS "SACK_BAG_EXPORTS;__NO_OPTIONS__;__STATIC__"
                  FOLDER utils/launcher
)
target_link_libraries( ${PROJECT_NAME} ws2_32 wsock32 winmm )
install_default_dest_binary( ${PROJECT_NAME} )


project( launchpad.isp ) 

set(BASE_SOURCES2
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad ishell_plugin
${LAST_GCC_PROGRAM_SOURCE}
)

include_directories( ${SACK_BASE}/src/InterShell )
add_library(${PROJECT_NAME} SHARED ${BASE_SOURCES2} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  PREFIX ""
                  SUFFIX ""
                  FOLDER utils/launcher
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_literal_product( ${PROJECT_NAME} InterShell/plugins ) 


if( altanik_is_built )
project( launchpad.isom ) 

set(BASE_SOURCES3
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad milk_plugin
${LAST_GCC_PROGRAM_SOURCE}
)

add_library(${PROJECT_NAME} SHARED ${BASE_SOURCES3} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags} -DISHELL_PLUGIN"
                  PREFIX ""
                  SUFFIX ""
                  FOLDER utils/launcher
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )
                      endif()

add_subdirectory( frames )

