cmake_minimum_required(VERSION 2.8)

project( SACK )

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES} )


if( ${CMAKE_GENERATOR} MATCHES "Watcom WMake" )

#ENABLE_LANGUAGE(RC)
##<FLAGS> flags is not OK.
#SET(CMAKE_COMPILE_RESOURCE "rc")
SET( CMAKE_RC_COMPILER "rc" )
#SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> /fo<OBJECT> <SOURCE>  -i${SACK_SOURCE_DIR}")

 set( ExtraFlags "${ExtraFlags} /fp5 /fpi87" )
 # this flag is for build dll?  emit DllRuntime startup link 
 #set( LibraryExtraFlags "-bd" )
 set( ConsoleProgramExtraFlags "-bc" )
 set( WindowsProgramExtraFlags "-bg" )
 
#if( CMAKE_BUILD_TYPE MATCHES "Debug" )
#  set( CMAKE_C_LINK_EXECUTABLE   "${CMAKE_C_LINK_EXECUTABLE} debug all" )
#endif()


endif( ${CMAKE_GENERATOR} MATCHES "Watcom WMake" )

set(WIN32_VERSION "0x0501" CACHE STRING
      "Set minimum windows build version")
  set_property(CACHE WIN32_VERSION PROPERTY STRINGS
               "0x501" "0x0500" "0x0400")
add_definitions( -D_WIN32_WINNT=${WIN32_VERSION} )



set( SACK_BASE ${PROJECT_SOURCE_DIR} )

set( CMAKE_MODULE_PATH ${SACK_SOURCE_DIR}/makefiles/cmake )
include( DefaultInstall )
#include( FindSqlite )

if( WIN32 )
   set( WIN32 ON )
else()
   if( UNIX )
      set( __LINUX__ ON )
   endif()
endif()

#SET(VLC_PATH "c:/tools/vlc-1.1.4" CACHE STRING "Base Path of VLC Installation 1.1.0 and more" )

if ( UNIX )
        set( USE_INTERNAL_STANDARDLIBS 0 )
        find_package( PNG )
        find_package( JPEG )
        find_package( Freetype )
	find_package( Sqlite )
        find_package( Threads REQUIRED )
	find_package( SDL REQUIRED )
        set( SDL_LIBRARIES SDL SDL SDLmain SDL_gfx SDL_image SDL_ttf )
else ( UNIX )
if( WATCOM OR MSVC )
        set( USE_INTERNAL_STANDARDLIBS 1 )
else()
        set( USE_INTERNAL_STANDARDLIBS 0 )
endif()
        # TODO: Fix this hardcoding
#        set( CMAKE_THREAD_LIBS_INIT pthread )
        set( FREETYPE_LIBRARIES freetype )
#        set( FREETYPE_INCLUDE_DIRS "c:/tools/unix/mingw/include" )
        set( JPEG_LIBRARIES jpeg )
        set( PNG_LIBRARIES png )
#        set( SOCKET_LIBRARIES wsock32 )
#        set( SDL_LIBRARIES SDL SDL SDLmain SDL_gfx SDL_image SDL_ttf )
#        set( SDL_INCLUDE_DIR "c:/tools/unix/mingw/include/SDL" )
endif ( UNIX )

OPTION( __LINUX__ "Enable specific linux features (disable WIN32)" ${UNIX} )
OPTION( __LINUX64__ "Enable specific linux features (disable WIN32)" OFF )
OPTION( __ARM__ "Enable specific arm-linux features (disable WIN32)" OFF )
OPTION( USE_ODBC "Use Odbc" 1 )
OPTION( USE_OPTIONS "Use Option Code all internal option usage (and don't include option code)" 1 )
OPTION( USING_OLD_MINGW "MinGW Has NO UpdateLayeredWindow" ${MINGW} )
OPTION( USE_SQLITE "Use SQLITE (also links static internal)" 1 )
OPTION( USE_SQLITE_EXTERNAL "Use SQLITE (build as external module, included when off)" 0 )
OPTION( EXTREME_CODE_DEBUG "Enable extreme warning levels (mingw)" 0 )

OPTION( __NO_GUI__ "No GUI or image parts" 0 )
OPTION( NEED_JPEG "Build internal jpeg lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( NEED_PNG "Build internal png lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( NEED_FREETYPE "Build internal freetype lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( NEED_ZLIB "Build internal zlib lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( __WINDOWS_ARM_CE__ "Build for windows CE 5.0 ARM" 0 )
OPTION( USE_ASSEMBLY "Use Assembly files as available.  NASM required for this." OFF )
OPTION( BUILD_MONOLITHIC "Build one giant library" 1 )

if( EXTREME_CODE_DEBUG )
if(MINGW) 
add_definitions(-Wall -Wextra -Wconversion -Wsign-conversion -Wsign-compare -Wtype-limits)
endif()
endif( EXTREME_CODE_DEBUG )

if( __ARM__ OR __BUILD_ARM_CE__ )
set( CMAKE_C_COMPILER_WORKS 1 )
set( CMAKE_CXX_COMPILER_WORKS 1 )
add_definitions( -D_WIN32_WCE=0x501 -DUNDER_CE -DWINCE -D_WINDOWS -DARM -D_ARM_ -DWIN32_PLATFORM_WFSP -D_UNICODE -DUNICODE -DPOCKETPC2003_UI_MODEL -DWIN32 )
else( )
  if( MSVC )
    add_definitions( -D_X86_ )
  endif( MSVC )
endif()


set( ExtraDefinitions ${ExtraDefinitions};SACK_CORE_BUILD )



if( __LINUX__ )
   add_definitions( -D__LINUX__ )
endif( __LINUX__ )
if( __LINUX64__ )
   add_definitions( -D__LINUX64__ )
endif( __LINUX64__ )


if( USE_ODBC OR USE_SQLITE )

set( ODBC_SOURCES
  src/SQLlib/sqlparse3.c
  src/SQLlib/sqlstub.c
  src/SQLlib/sqlutil.c
  src/SQLlib/sqlwrap.c
)
SET_SOURCE_FILES_PROPERTIES( ${ODBC_SOURCES} PROPERTIES 
		FOLDER "Source Files\\ODBC" )
 if( USE_ODBC )
set( ExtraDefinitions ${ExtraDefinitions};USE_ODBC )
   if( WIN32 )
LINK_LIBRARIES( odbc32 )
   else()
LINK_LIBRARIES( odbc )
   endif()
 endif( USE_ODBC )

 if( USE_OPTIONS )
SET( OPTION_SOURCE
   src/SQLlib/optlib/getoption.c
   src/SQLlib/optlib/getoption_new.c
   src/SQLlib/optlib/optionutil.c
   src/SQLlib/optlib/optionutil_new.c
   )
 else( USE_OPTIONS )
set( ExtraDefinitions ${ExtraDefinitions};__NO_OPTIONS__ )
 endif( USE_OPTIONS )
 if( USE_SQLITE )
  SET( BAG_SQLITE_INTERFACE  src/sqlite/sqlite_interface.c )
#if( !SQLITE3_FOUND )
  SET( SQLITE_VERSION 3.7.0 ) #3.6.23
  SET( SQLITE_SOURCES ${BAG_SQLITE_INTERFACE} src/sqlite/${SQLITE_VERSION}/sqlite3.c  )
  include_directories( ${SACK_BASE}/src/sqlite/${SQLITE_VERSION} )
  
  if( NOT USE_SQLITE_EXTERNAL )
   # setup to include sqlite in the core libraries...
   set( BAG_SQLITE_SOURCES ${SQLITE_SOURCES} )
   set( ExtraDefinitions "${ExtraDefinitions};SQLITE_SOURCE;USE_SQLITE_INTERFACE" )
  else()
   set( ExtraDefinitions "${ExtraDefinitions};USE_SQLITE_INTERFACE" )
  endif()
#else( !SQLITE3_FOUND )
  set( ExtraDefinitions "${ExtraDefinitions};SYSTEM_SQLITE" )
#endif( !SQLITE3_FOUND )

  set( ExtraDefinitions "${ExtraDefinitions};USE_SQLITE" )
    
 endif(USE_SQLITE)

else( USE_ODBC OR USE_SQLITE )
   message( "no options too..." )
   set( ExtraDefinitions ${ExtraDefinitions};__NO_SQL__;__NO_OPTIONS__ )

endif( USE_ODBC OR USE_SQLITE )

# this is where external image libraries are added for windows native builds
if( NEED_JPEG OR NEED_PNG OR NEED_FREETYPE )
include( Externals.cmake )
endif()

  if( ${CMAKE_COMPILER_IS_GNUCC} ) 
  SET( FIRST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/deadstart/deadstart_list.c ) 
  SET( FIRST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_list.c ) 
  SET( LAST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/deadstart/deadstart_lib.c ${SACK_BASE}/src/deadstart/deadstart_end.c ) 
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_lib.c ${SACK_BASE}/src/deadstart/deadstart_prog.c ${SACK_BASE}/src/deadstart/deadstart_end.c ) 
  endif()
  if( ${MSVC}${WATCOM} ) 
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_prog.c ) 
  endif()


set(  CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG" )
set(  CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -D_DEBUG" )
set(  CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG" )
set(  CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_DEBUG" )

if( USE_ASSEMBLY )
set( ExtraDefinitions ${ExtraDefinitions};HAS_ASSEMBLY )
set( ImageExtraFlags "${ImageExtraFlags} -DHAS_ASSEMBLY" )
set( IMAGE_ASM_SOURCES
  src/imglib/ImageFile.asm
  src/imglib/alphamac.asm
  src/imglib/blatcolor.asm
  src/imglib/blotdirasm.asm
  src/imglib/blotdirmac.asm
  src/imglib/blotdirmshadeasm.asm
  src/imglib/blotdirshadeasm.asm
  src/imglib/blotscaasm.asm
  src/imglib/blotscamac.asm
  src/imglib/blotscamshadeasm.asm
  src/imglib/blotscashadeasm.asm
  src/imglib/blotshademac.asm
  src/imglib/lineasm.asm
  src/imglib/mmx.asm
  src/imglib/plotasm.asm
)
set( MEM_ASSEMBLY_SOURCES src/memlib/memoryasm.asm )
endif()

# expat is a 'standard' library.  ripped a copy for ease, it's also incredibly simple.
set ( EXPAT_SOURCES 
  src/sexpat/xmlparse.c
  src/sexpat/xmlrole.c
  src/sexpat/xmltok.c
)
SOURCE_GROUP( "Source Files\\XML\\expat" FILES ${EXPAT_SOURCES} )
set( ExtraDefinitions "${ExtraDefinitions};XML_MIN_SIZE" )

# genx is searchable by looking for 'genx xml'
set( GENX_SOURCES
  src/genx/charProps.c
  src/genx/genx.c
)
SET_SOURCE_FILES_PROPERTIES( ${GENX_SOURCES} PROPERTIES 
		FOLDER "Source Files\\XML\\Genx Library" )

if( WIN32 )
	set( VIDLIB_SOURCES
	  src/vidlib/key.c
	  src/vidlib/keydefs.c
	  src/vidlib/vidlib.c
	)

	SET_SOURCE_FILES_PROPERTIES( ${VIDLIB_SOURCES} PROPERTIES 
					FOLDER "Source Files\\Image Output Library" )

	if( NOT __ARM__ )
		set( VIDLIB_SOURCES ${VIDLIB_SOURCES} src/vidlib/opengl.c )
		set( ExtraDefinitions "${ExtraDefinitions};_OPENGL_ENABLED" )
	endif( NOT __ARM__ )
else( WIN32 )

endif( WIN32 )

set( IMAGE_SOURCES
  src/imglib/alphastab.c
  src/imglib/alphatab.c
  src/imglib/blotdirect.c
  src/imglib/blotscaled.c
  src/imglib/bmpimage.c
  src/imglib/fntcache.c
  src/imglib/fntrender.c
  src/imglib/font.c
  src/imglib/gifimage.c
  src/imglib/image.c
  src/imglib/image_client.c
  src/imglib/interface.c
  src/imglib/jpgimage.c
  src/imglib/line.c
  src/imglib/lucidaconsole.c
  src/imglib/math.c
  src/imglib/pngimage.c
  src/imglib/sprite_common.c
  src/imglib/sprite.c
)
SET_SOURCE_FILES_PROPERTIES( ${IMAGE_SOURCES} PROPERTIES 
		FOLDER "Source Files\\Image Library" )

if( NOT __NO_GUI__ )
set( PSI_MORE_CONTROLS
  src/psilib/console/history.c
  src/psilib/console/console_keydefs.c
  src/psilib/console/paste.c
  src/psilib/console/psicon.c
  src/psilib/console/psicon_interface.c
  src/psilib/console/regaccess.c
  src/psilib/console/WinLogic.c
)
set( ExtraDefinitions "${ExtraDefinitions};PSI_CONSOLE_SOURCE" )
endif( NOT __NO_GUI__ )

set( PSI_SOURCES
  src/psilib/borders.c
  src/psilib/calctl/calender.c
  src/psilib/calctl/clock.c
  src/psilib/calctl/analog.c
  src/psilib/controls.c
  src/psilib/control_physical.c
  src/psilib/ctlbutton.c
  src/psilib/ctledit.c
  src/psilib/ctllistbox.c
  src/psilib/ctlmisc.c
  src/psilib/ctlprop.c
  src/psilib/ctlscroll.c
  src/psilib/ctlsheet.c
  src/psilib/ctlslider.c
  src/psilib/ctltext.c
  src/psilib/fileopen.c
  src/psilib/fntdlg.c
  src/psilib/loadsave.c
  src/psilib/mouse.c
  src/psilib/palette.c
  src/psilib/popups.c
  src/psilib/xml_load.c
  src/psilib/xml_save.c
  src/psilib/option_frame.c
  ${PSI_MORE_CONTROLS}
)
SET_SOURCE_FILES_PROPERTIES( ${PSI_SOURCES} PROPERTIES 
		FOLDER "Source Files\\PSI Library" )
SET_SOURCE_FILES_PROPERTIES( ${PSI_MORE_CONTROLS} PROPERTIES 
		FOLDER "Source Files\\PSI Library\\Console" )

if( WIN32 )
  SET( PSI_SOURCES ${PSI_SOURCES}  src/systraylib/systray.c )
else( WIN32 )
  SET( PSI_SOURCES ${PSI_SOURCES}  src/systraylib/systray_client.c )
endif( WIN32 )

if( MSVC )
  SET( SNPRINTF_SOURCES src/snprintf_2.2/snprintf.c )
  set_source_files_properties( ${SNPRINTF_SOURCES} 
  				PROPERTIES
                                COMPILE_DEFINITIONS "SNPRINTF_LONGLONG_SUPPORT;PREFER_PORTABLE_SNPRINTF"
  			)
  install( FILES src/snprintf_2.2/snprintf.h DESTINATION include/SACK/snprintf-2.2 )
endif( MSVC )


if( NOT BUILD_MONOLITHIC )
set( PSI_SOURCES ${PSI_SOURCES} 
  src/psilib/fntcache.c
)
endif()

set( ExtraDefinitions "${ExtraDefinitions};GENX_SOURCE" )

set( SACK_MSG_SERVICES 
  src/msgsvr/client/client.c
  src/msgsvr/summoner/construct.c
  )

set( FILESYSTEM_SOURCES
  src/systemlib/spawntask.c
  src/filesyslib/filemon/allfiles.c
  src/filesyslib/winfiles.c
)

if( __LINUX__ )
set(FILESYSTEM_SOURCES ${FILESYSTEM_SOURCES}
  src/filesyslib/filemon/linuxfiles.c
)
else( __LINUX__ )
set(FILESYSTEM_SOURCES ${FILESYSTEM_SOURCES}
  src/filesyslib/filemon/windowsfiles.c
)
endif( __LINUX__ )


SET_SOURCE_FILES_PROPERTIES( ${FILESYSTEM_SOURCES} PROPERTIES 
		FOLDER "Source Files\\File System Utils" )


if( NOT __ARM__ )
set(BASE_SOURCES ${BASE_SOURCES} ${SACK_MSG_SERVICES}
  ${FILESYSTEM_SOURCES}
)
else()
set( ExtraDefinitions "${ExtraDefinitions};__NO_MSGSVR__" )
endif()

if( WIN32 )
set( MSG_QUEUE_SOURCES
  src/typelib/msgqueue.c
)
endif( WIN32 )

set( TYPE_LIBRARY_SOURCES
  src/typelib/binarylist.c
  src/typelib/familytree.c
  src/typelib/input.c
  src/typelib/sets.c
  src/typelib/text.c
  src/typelib/http.c
  src/typelib/typecode.c
  src/typelib/spacetree.c
)

SET_SOURCE_FILES_PROPERTIES( ${TYPE_LIBRARY_SOURCES} PROPERTIES 
		FOLDER "Source Files\\Types" )

if( WIN32 )
set(MORE_NETWORK_SOURCES
  src/netlib/net_winsock2.c
)
endif( WIN32 )

if( NOT __ARM__ )
set(MORE_NETWORK_SOURCES ${MORE_NETWORK_SOURCES} 
  src/netlib/ping.c
  src/netlib/whois.c
)
endif( NOT __ARM__ )

set( NETWORK_SOURCES
  src/netlib/network.c
  src/netlib/tcpnetwork.c
  src/netlib/udpnetwork.c
  ${MORE_NETWORK_SOURCES}
)
SET_SOURCE_FILES_PROPERTIES( ${NETWORK_SOURCES} PROPERTIES 
		FOLDER "Source Files\\Network Library" )


set(BASE_SOURCES  ${BASE_SOURCES}
  src/sysloglib/syslog.c
  src/memlib/sharemem.c
  ${MEM_ASSEMBLY_SOURCES}
  ${TYPE_LIBRARY_SOURCES}
  ${MSG_QUEUE_SOURCES}
  src/timerlib/timers.c
  src/idlelib/idle.c
  src/fractionlib/fractions.c
  src/configlib/configscript.c
  src/procreglib/names.c
  src/systemlib/args.c
  ${NETWORK_SOURCES}
  src/deadstart/deadstart_core.c
  src/vectlib/vectlib.c
  src/sha1lib/sha1.c
  src/filesyslib/pathops.c
  src/filesyslib/filescan.c
  src/systemlib/system.c
  src/utils/service/service_stub.c

  src/commlib/sackcomm.c
  ${OPTION_SOURCE}
)

set( ExtraDefinitions "${ExtraDefinitions};SACKCOMM_SOURCE" )



include_directories(  ${SACK_BASE}/include )


if( USING_OLD_MINGW )
add_definitions( -DMINGW_SUX )
endif()


if( WIN32 )
add_definitions( -DWIN32 )
LIST(APPEND BASE_SOURCES
   src/netlib/net_winsock2.c
)
endif(WIN32)

if( WIN32 )
LIST(APPEND BASE_SOURCES     all_resources.rc )
if( MINGW )

SET(CMAKE_RC_COMPILER_INIT "windres")
SET(CMAKE_RC_COMPILER "windres")
SET(CMAKE_RC_COMPILE_OBJECT "windres <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")
ENABLE_LANGUAGE(RC)



#set( CMAKE_RC_COMPILER windres )
#add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/all_resources.res
#                     DEPENDS ${SACK_BASE}/all_resources.rc
#                     COMMAND ${CMAKE_RC_COMPILER} -I${SACK_BASE}  -o${PROJECT_BINARY_DIR}/all_resources.res -i${SACK_BASE}/all_resources.rc -O coff
# )
# ADD_CUSTOM_TARGET( generate_foo DEPENDS ${PROJECT_BINARY_DIR}/all_resources.res)
elseif( WATCOM )
string( REPLACE "/" "\\" WATCOM_PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR} )
string( REPLACE "/" "\\" WATCOM_CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/all_resources.res
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/all_resources.rc
                     COMMAND echo ${CMAKE_RC_COMPILER} -i${WATCOM_CMAKE_CURRENT_SOURCE_DIR} -fo${WATCOM_PROJECT_BINARY_DIR}\\all_resources.res ${WATCOM_CMAKE_CURRENT_SOURCE_DIR}\\all_resources.rc
                     COMMAND ${CMAKE_RC_COMPILER} -i${WATCOM_CMAKE_CURRENT_SOURCE_DIR} -fo${WATCOM_PROJECT_BINARY_DIR}\\all_resources.res ${WATCOM_CMAKE_CURRENT_SOURCE_DIR}\\all_resources.rc
 )
 ADD_CUSTOM_TARGET( generate_foo DEPENDS ${PROJECT_BINARY_DIR}/all_resources.res)
endif()
endif( WIN32 ) 

if( BUILD_MONOLITHIC )

add_definitions( -DSACK_MONOLITHIC_BUILD )
if( NOT __NO_GUI__ )
set( BASE_SOURCES ${BASE_SOURCES} 
    ${IMAGE_SOURCES} 
    ${VIDLIB_SOURCES} 
    ${PSI_SOURCES} 
)
endif( NOT __NO_GUI__ )

set( C_SOURCES
    ${JPEG_SOURCE}
    ${ZLIB_SOURCE}
    ${PNG_SOURCE}
    ${FREETYPE_SOURCE}
    ${EXPAT_SOURCES}
)

set( SACK_SOURCES 
  ${FIRST_GCC_LIBRARY_SOURCE} 
    ${BASE_SOURCES} 
    ${SNPRINTF_SOURCES}
    ${ODBC_SOURCES}
    ${BAG_SQLITE_SOURCES}
    ${C_SOURCES}
    ${GENX_SOURCES}
  ${LAST_GCC_LIBRARY_SOURCE} 
)

set( SACK_PLUSPLUS_SOURCES 
  ${FIRST_GCC_LIBRARY_SOURCE} 
    ${BASE_SOURCES} 
    ${SNPRINTF_SOURCES}
    ${ODBC_SOURCES}
    ${GENX_SOURCES}
  ${LAST_GCC_LIBRARY_SOURCE} 
)


set( SACK_PLUSPLUS_HEADERS 
     src/msgsvr/summoner/summoner.h
     src/systemlib/taskinfo.h
     src/filesyslib/filemon/monitor.h
     src/memlib/sharestruc.h
     src/procreglib/registry.h
     src/netlib/netstruc.h
     src/netlib/ping.h
     src/vectlib/vecstruc.h
     src/SQLlib/sqlstruc.h
     src/SQLlib/optlib/optlib.h
     src/SQLlib/optlib/makeopts.mysql
     src/imglib/blotproto.h
    src/imglib/fntglobal.h
    src/imglib/cache.h
    src/imglib/gifimage.h
    src/imglib/pngimage.h
    src/imglib/jpgimage.h
        src/imglib/bmpimage.h
        src/imglib/sprite_local.h
        src/vidlib/local.h
        src/vidlib/glext.h
        src/psilib/controlstruc.h
        src/psilib/global.h
        src/psilib/calctl/local.h
        src/psilib/mouse.h
        src/psilib/borders.h
        src/psilib/resource.h
        src/psilib/CommonEdit.Frame
        src/psilib/palette.frame
        src/psilib/menustruc.h
        src/psilib/console/consolestruc.h
        src/psilib/console/history.h
        src/psilib/console/keydefs.h
        src/psilib/console/histstruct.h
        src/psilib/console/WinLogic.h
        src/genx/genx.h
)
FOREACH(SOURCE ${SACK_PLUSPLUS_HEADERS})
       EXECUTE_PROCESS(COMMAND cmake -E copy_if_different ${PROJECT_SOURCE_DIR}/${SOURCE} ${CMAKE_BINARY_DIR}/${SOURCE} )
ENDFOREACH(SOURCE)

set( cplusplus_sources )

FOREACH(SOURCE ${SACK_PLUSPLUS_SOURCES})
   if( ${SOURCE} MATCHES "(.*)\\.c" )
     set( BASENAME ${CMAKE_MATCH_1} )
       get_source_file_property(SOURCE_FOLDER ${SOURCE} FOLDER)
       if( ${SOURCE} MATCHES "^${PROJECT_SOURCE_DIR}.*" )
          if( NOT ${SOURCE_FOLDER} MATCHES "NOTFOUND" )
            #message( "err folder : ${SOURCE_FOLDER} " )
            SOURCE_GROUP( ${SOURCE_FOLDER} FILES ${SOURCE} )
            SOURCE_GROUP( ${SOURCE_FOLDER} FILES ${CMAKE_BINARY_DIR}/${BASENAME}.cpp )
          endif()
          EXECUTE_PROCESS(COMMAND cmake -E copy_if_different ${SOURCE} ${CMAKE_BINARY_DIR}/${BASENAME}.cpp )
       
       else( ${SOURCE} MATCHES "^${PROJECT_SOURCE_DIR}.*" )
         if( NOT ${SOURCE_FOLDER} MATCHES "NOTFOUND" )
           #message( "err folder : ${SOURCE_FOLDER} " )
           SOURCE_GROUP( ${SOURCE_FOLDER} FILES ${PROJECT_SOURCE_DIR}/${SOURCE} )
           SOURCE_GROUP( ${SOURCE_FOLDER} FILES ${CMAKE_BINARY_DIR}/${BASENAME}.cpp )
         endif()
         EXECUTE_PROCESS(COMMAND cmake -E copy_if_different ${PROJECT_SOURCE_DIR}/${SOURCE} ${CMAKE_BINARY_DIR}/${BASENAME}.cpp )
         #message( "LIB2_SOURCE2 : ${BASENAME}  ${SOURCE}" )
       endif()
     set( cplusplus_sources ${cplusplus_sources} ${CMAKE_BINARY_DIR}/${BASENAME}.cpp )
   else()
      #message( "NOT THING ${SOURCE}" )
   endif()
ENDFOREACH(SOURCE)

add_library( sack_bag SHARED ${SACK_SOURCES} )
SET_TARGET_PROPERTIES(sack_bag PROPERTIES
                  DEFINE_SYMBOL SACK_BAG_EXPORTS
                  COMPILE_FLAGS  "-DTARGET_LABEL=sack_bag ${ExtraFlags} ${TARGETNAME} ${ExtraRenderFlags}"
                  COMPILE_DEFINITIONS "${ExtraDefinitions}"
)

if( WATCOM )
set( SACK_BAG_PLUSPLUS sack_bag_pp )
else(WATCOM)
set( SACK_BAG_PLUSPLUS sack_bag++ )
endif(WATCOM)

add_library( ${SACK_BAG_PLUSPLUS} SHARED ${cplusplus_sources} )
SET_TARGET_PROPERTIES(${SACK_BAG_PLUSPLUS} PROPERTIES
                  DEFINE_SYMBOL SACK_BAG_EXPORTS
                  COMPILE_FLAGS  "${ExtraFlags} ${ExtraRenderFlags}"
                  COMPILE_DEFINITIONS "${ExtraDefinitions};TARGET_LABEL=sack_bag_plusplus;TARGETNAME=\"${CMAKE_SHARED_LIBRARY_PREFIX}sack_bag${CMAKE_SHARED_LIBRARY_SUFFIX}\""
)

if( __WINDOWS_ARM_CE__ )
  SET_TARGET_PROPERTIES( sack_bag PROPERTIES LINK_LIBRARIES "" )
endif()

if( WATCOM )
  ADD_DEPENDENCIES( sack_bag generate_foo )
  SET_TARGET_PROPERTIES(sack_bag PROPERTIES LINK_FLAGS "option resource=all_resources.res")
  SET_TARGET_PROPERTIES(${SACK_BAG_PLUSPLUS} PROPERTIES LINK_FLAGS "option resource=all_resources.res")
elseif( MINGW )
  ADD_DEPENDENCIES( sack_bag generate_foo )
  #ADD_DEPENDENCIES( sack_bag all_resources.res )
  #SET_TARGET_PROPERTIES(sack_bag PROPERTIES LINK_FLAGS "all_resources.res")
endif()

set( SACK_LIBRARY sack_bag )
set( SACK_LIBRARIES sack_bag ${SACK_BAG_PLUSPLUS} )

if( WIN32 )
  if( __ARM__ )
    target_link_libraries( sack_bag ws2 mmtimer iphlpapi coredll corelibc.lib)
    target_link_libraries( ${SACK_BAG_PLUSPLUS} sack_bag ws2 mmtimer iphlpapi coredll corelibc.lib)
  else()
    target_link_libraries( sack_bag wsock32 ws2_32 opengl32 winmm iphlpapi)
    target_link_libraries( ${SACK_BAG_PLUSPLUS} sack_bag wsock32 ws2_32 opengl32 winmm iphlpapi)
  endif()
else( WIN32 )
  target_link_libraries( sack_bag odbc ${SQLITE3_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} )
  target_link_libraries( ${SACK_BAG_PLUSPLUS} sack_bag odbc ${SQLITE3_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} )
endif( WIN32 )

install_mode_dest( sack_bag )
install_mode_dest( ${SACK_BAG_PLUSPLUS} )
INSTALL(
  FILES
    data/interface.conf.new
  DESTINATION
    # this could also be /etc
    bin/\${CMAKE_INSTALL_CONFIG_NAME}
  RENAME interface.conf
)

#install_default_dest( sack_bag )
#install_default_dest( ${SACK_BAG_PLUSPLUS} )

install( FILES makefiles/cmake/DefaultInstall.cmake DESTINATION . )

if( NOT NEED_PNG ) 
  target_link_libraries( sack_bag png )
  target_link_libraries( ${SACK_BAG_PLUSPLUS} png )
endif() 
if( NOT NEED_JPEG ) 
  target_link_libraries( sack_bag jpeg   )
  target_link_libraries( ${SACK_BAG_PLUSPLUS} jpeg   )
endif() 
if( NOT NEED_FREETYPE )
  target_link_libraries( sack_bag  freetype  )
  target_link_libraries( ${SACK_BAG_PLUSPLUS}  freetype  )
endif()


#------------
else( BUILD_MONOLITHIC )
#------------

add_library( bag SHARED 
  ${FIRST_GCC_LIBRARY_SOURCE} 
    ${BASE_SOURCES} 
    ${ODBC_SOURCES}
    ${BAG_SQLITE_SOURCES}
  ${LAST_GCC_LIBRARY_SOURCE}  
)

SET_TARGET_PROPERTIES(bag PROPERTIES 
                  DEFINE_SYMBOL SACK_BAG_EXPORTS
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag ${ExtraFlags}"
)

if( __WINDOWS_ARM_CE__ )
SET_TARGET_PROPERTIES( bag PROPERTIES LINK_LIBRARIES "" )
endif( __WINDOWS_ARM_CE__ )

if( __WINDOWS_ARM_CE__ )
SET( LINK_LIBRARIES "" )
endif( __WINDOWS_ARM_CE__ )

if( __ARM__ )
target_link_libraries( bag ws2 mmtimer iphlpapi coredll corelibc.lib )
else()
target_link_libraries( bag wsock32 ws2_32 opengl32 winmm iphlpapi)
endif()
set( SACK_LIBRARIES bag bag.psi bag.image bag.video )

###########

if( NEED_FREETYPE OR NEED_JPEG OR NEED_PNG )
set( SACK_LIBRARIES ${SACK_LIBRARIES} bag.externals )

add_library( bag.externals SHARED 
    ${JPEG_SOURCE}
    ${ZLIB_SOURCE}
    ${PNG_SOURCE}
    ${FREETYPE_SOURCE}
)
SET_TARGET_PROPERTIES(bag.externals PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_externals ${ExternalExtraFlags} -DBAG_EXTERNALS -DPNG_INTERNAL"
)
target_link_libraries( bag.externals bag )
set( EXTERNALS bag.externals )
endif()
###########

add_library( bag.image SHARED 
  ${FIRST_GCC_LIBRARY_SOURCE} 
  ${IMAGE_SOURCES} 
  ${LAST_GCC_LIBRARY_SOURCE} 
)
SET_TARGET_PROPERTIES(bag.image PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_image ${ImageExtraFlags}"
)
target_link_libraries( bag.image bag ${EXTERNALS} )
set( SACK_IMAGE_LIBRARY bag.video )

###########

add_library( bag.video SHARED 
  ${FIRST_GCC_LIBRARY_SOURCE} 
  ${VIDLIB_SOURCES} 
  ${LAST_GCC_LIBRARY_SOURCE} 
)
SET_TARGET_PROPERTIES(bag.video PROPERTIES
                  DEFINE_SYMBOL VIDEO_LIBRARY_SOURCE
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_video -DRENDER_LIBRARY_SOURCE ${ExtraRenderFlags}"
)
target_link_libraries( bag.video bag.image bag ${EXTERNALS} )
set( SACK_VIDEO_LIBRARY bag.video )

###########

add_library( bag.psi SHARED 
  ${FIRST_GCC_LIBRARY_SOURCE} 
  ${PSI_SOURCES} 
  ${LAST_GCC_LIBRARY_SOURCE} 
      )
SET_TARGET_PROPERTIES(bag.psi PROPERTIES
                  DEFINE_SYMBOL PSI_SOURCE
                  COMPILE_FLAGS  "${ExtraFlags}"
                  COMPILE_DEFINTIIONS  "TARGET_LABEL=bag_psi;SOURCE_PSI2;SYSTRAY_LIBRARAY;FORCE_NO_INTERFACE"
)
target_link_libraries( bag.psi bag.video bag.image bag )
set( SACK_PSI_LIBRARY bag.psi )

install_default_dest( bag bag.image bag.video bag.psi ${EXTERNALS} )

if( NOT NEED_PNG ) 
  target_link_libraries( bag.image png )
endif() 
if( NOT NEED_JPEG ) 
  target_link_libraries( bag.image jpeg   )
endif() 
if( NOT NEED_FREETYPE )
  target_link_libraries( bag.image  freetype  )
  target_link_libraries( bag.psi  freetype  )
endif()

endif( BUILD_MONOLITHIC )


# this SACK_CORE_BUILD is for building before we get 'installed' some of the header locations get moved.
if( WATCOM )
   set( ExtraDefinitions "${ExtraDefinitions};__OBSCURE_STREAM_INTERNALS" )
endif( WATCOM )


  if( USE_SQLITE_EXTERNAL )
   set( sqlite_external bag.sqlite3.external )
   set( ExtraDefinitions "${ExtraDefinitions};USE_SQLITE_INTERFACE" )
   add_library( ${sqlite_external} SHARED 
      ${FIRST_GCC_LIBRARY_SOURCE} 
      ${SQLITE_SOURCES}
      ${LAST_GCC_LIBRARY_SOURCE} )
   # required link back to bag for register interface
   target_link_libraries( ${sqlite_external} ${SACK_LIBRARIES} )
   install_default_dest( ${sqlite_external} )
   SET_TARGET_PROPERTIES( ${sqlite_external} PROPERTIES 
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_sqlite3 -DSQLITE_SOURCE"
                  SUFFIX ""
                  PREFIX ""
                  FOLDER sqlite
   )
  else(USE_SQLITE_EXTERNAL)
  
  endif(USE_SQLITE_EXTERNAL)

if( !MSVC )
add_subdirectory( src/displaylib )
endif( !MSVC )

#add_subdirectory( src/InterShell )

# parts of SDK ...
add_subdirectory( include )
add_subdirectory( src/utils/runner )
add_subdirectory( src/deadstart )

add_subdirectory( src/utils/deploy )
add_subdirectory( src/utils/pcopy )
 if( NOT __NO_GUI__ )
  if( USE_OPTIONS )
   add_subdirectory( src/SQLlib/optlib/editoption )
  endif( USE_OPTIONS )
 endif( NOT __NO_GUI__ )

SET( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP 1 )

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SACK Layer 1")
SET(CPACK_PACKAGE_VENDOR "Freedom Collective")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/howto.compile")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/read.this")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "0")

if( SUPPORTS_PARALLEL_BUILD_TYPE )
SET( VERSION_TYPE ${CMAKE_INSTALL_CONFIG_TYPE} )
else( SUPPORTS_PARALLEL_BUILD_TYPE )
SET( VERSION_TYPE ${CMAKE_BUILD_TYPE} )
endif( SUPPORTS_PARALLEL_BUILD_TYPE )

	SET(CPACK_PACKAGE_VERSION_PATCH "478-${CMAKE_GENERATOR}-${VERSION_TYPE}")

#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/doc\\\\BlankButton.jpg")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\sack_bag.dll")
  SET(CPACK_NSIS_DISPLAY_NAME "SACK ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\code.google.com\\\\p\\\\c-system-abstraction-component-gui")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\code.google.com\\\\p\\\\c-system-abstraction-component-gui")
  SET(CPACK_NSIS_CONTACT "d3ck0r@gmail.com")
  SET(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Set Build Mode.lnk' '\$INSTDIR\\\\bin\\\\${VERSION_TYPE}\\\\sack_deploy.exe'" )
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"\$INSTDIR\\\\bin\\\\${VERSION_TYPE}\\\\sack_deploy.exe\\\"'")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/MyExecutable")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
#SET(CPACK_PACKAGE_EXECUTABLES "" "My Executable")
INCLUDE(CPack)

