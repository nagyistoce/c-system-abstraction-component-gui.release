cmake_minimum_required(VERSION 2.8)

#set( CMAKE_VERBOSE_MAKEFILE  1 )
#message( ${CMAKE_GENERATOR} )

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Set build type")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
               "Debug" "Release" "MinSizeRel" "RelWithDebInfo")


if( ${CMAKE_GENERATOR} MATCHES "Watcom WMake" )
 SET( CFLAGS "-bd" )
if( CMAKE_BUILD_TYPE MATCHES "Debug" )
  SET( CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_FLAGS_DEBUG} -d2" )
  #SET( CMAKE_C_FLAGS_DEBUG  "-d2 -od -oe=0 -bm -zld -zq  /fp5 /fpi87 -st -s" )
  set( CMAKE_C_LINK_EXECUTABLE   "${CMAKE_C_LINK_EXECUTABLE} debug all" )
  add_definitions( -D_DEBUG )
endif()

set(WIN32_VERSION "0x0500" CACHE STRING
      "Set minimum windows build version")
  set_property(CACHE WIN32_VERSION PROPERTY STRINGS
               "0x0500" "0x0400")
add_definitions( -D_WIN32_WINNT=${WIN32_VERSION} )

endif( ${CMAKE_GENERATOR} MATCHES "Watcom WMake" )


project( SACK )


set( SACK_BASE ${PROJECT_SOURCE_DIR} )
set( CMAKE_MODULE_PATH ${SACK_SOURCE_DIR}/makefiles/cmake )
set( DEST_DIR ${sack_BINARY_DIR}/dest )

include( DefaultInstall )

OPTION( C++ "Build with C++" OFF )
OPTION( USE_SQLITE "Enable building SQLite" ON )
if( WIN32 )
   set( __WINDOWS__ ON )
else()
	if( UNIX )
          set( __LINUX__ ON )
        endif()
endif()

SET(VLC_PATH "c:/tools/vlc-1.0.5" CACHE STRING "Base Path of VLC Installation
1.0.0 and less than 1.1.0" )

OPTION( BUILD_INTERSHELL "Build InterShell" ON )
OPTION( SACK_ONLY "Build only sack library, and include structure?" OFF )
OPTION( __WINDOWS__ "Enable specific windows features" ${__WINDOWS__} )
OPTION( __LINUX__ "Enable specific linux features (disable __WINDOWS__)" OFF )
OPTION( __ARM__ "Enable specific arm-linux features (disable __WINDOWS__)" OFF )
OPTION( USE_ODBC "Use Odbc" 1 )
OPTION( USE_OPTIONS "Use Option Code all internal option usage (and don't include option code)" 1 )
OPTION( USING_OLD_MINGW "MinGW Has NO UpdateLayeredWindow" 1 )
OPTION( USE_SQLITE "Use SQLITE" 1 )
OPTION( __NO_GUI__ "No GUI or image parts" 0 )
OPTION( NEED_JPEG "Build internal jpeg lib (with suitable exports to link against)" 0 )
OPTION( NEED_PNG "Build internal png lib (with suitable exports to link against)" 0 )
OPTION( NEED_FREETYPE "Build internal freetype lib (with suitable exports to link against)" 0 )
OPTION( NEED_ZLIB "Build internal zlib lib (with suitable exports to link against)" 0 )


if ( UNIX )
        find_package( PNG REQUIRED )
        find_package( JPEG REQUIRED )
        find_package( Freetype REQUIRED )
        find_package( GGI REQUIRED )
        find_package( Threads REQUIRED )
        find_package( Curses REQUIRED )
        find_package( Sndfile REQUIRED )
	find_package( SDL REQUIRED )
        set( SDL_LIBRARIES SDL SDL SDLmain SDL_gfx SDL_image SDL_ttf )
else ( UNIX )
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

add_definitions( -DSACK_CORE_BUILD )


LINK_LIBRARIES( odbc32 )

if( USE_ODBC )

set( ODBC_SOURCE 
  src/SQLlib/sqlparse3.c
  src/SQLlib/sqlstub.c
  src/SQLlib/sqlstub.rc
  src/SQLlib/sqlutil.c
  src/SQLlib/sqlwrap.c
)
add_definitions( 
  -DUSE_ODBC
)

if( USE_OPTIONS )
SET( OPTION_SOURCE
   src/SQLlib/optlib/getoption.c
   src/SQLlib/optlib/getoption_new.c
   src/SQLlib/optlib/optionutil.c
   src/SQLlib/optlib/optionutil_new.c
   )
else()
add_definitions( 
  -D__NO_OPTIONS__
)
endif()

if( USE_SQLITE )
LIST(APPEND ODBC_SOURCE
   src/sqlite/sqlite3.c
   )
add_definitions( 
  -DUSE_SQLITE
)
endif()

else( USE_ODBC )
add_definitions( 
  -D__NO_SQL__ -D__NO_OPTIONS__
)

endif( USE_ODBC )

# this is where external image libraries are added for windows native builds
include( Externals.cmake )


if( NOT C++ )
  if( ${CMAKE_COMPILER_IS_GNUCC} ) 
  SET( FIRST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/deadstart/deadstart_list.c ) 
  SET( FIRST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_list.c ) 
  SET( LAST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/deadstart/deadstart_lib.c ${SACK_BASE}/src/deadstart/deadstart_end.c ) 
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_prog.c ${SACK_BASE}/src/deadstart/deadstart_end.c ) 
  endif()
  if( ${MSVC}${WATCOM} ) 
  message( "Setting program deadstart." )
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_prog.c ) 
  endif()
else()
  if( ${CMAKE_COMPILER_IS_GNUCC} ) 
  # gcc I still need this, otherwise a program needs to InvokeDeadstarts()
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_prog.c ) 
  endif()
endif()


  add_definitions( -D_DEBUG )

set( ASM_SOURCES
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

set ( CONFIGURATION
  src/procreglib/interface.conf
)

set(SOURCES 
  ${FIRST_GCC_LIBRARY_SOURCE}
  all_resources.rc
  src/sysloglib/syslog.c
  src/memlib/sharemem.c
  src/typelib/binarylist.c
  src/typelib/familytree.c
  src/typelib/input.c
  src/typelib/msgqueue.c
  src/typelib/sets.c
  src/typelib/text.c
  src/typelib/typecode.c
  src/timerlib/timers.c
  src/idlelib/idle.c
  src/msgsvr/client/client.c
  src/fractionlib/fractions.c
  src/configlib/configscript.c
  src/procreglib/names.c
  src/netlib/network.c
  src/netlib/net_winsock2.c
  src/netlib/ping.c
  src/netlib/tcpnetwork.c
  src/netlib/udpnetwork.c
  src/netlib/whois.c
  src/filesyslib/filescan.c
  src/filesyslib/pathops.c
  src/filesyslib/filemon/allfiles.c
  src/filesyslib/filemon/windowsfiles.c
  src/systemlib/system.c
  src/systemlib/spawntask.c
  src/systemlib/args.c
  src/msgsvr/summoner/construct.c
  src/deadstart/deadlib.c
  src/deadstart/deadstart_core.c
  src/deadstart/deadstop.c
  src/vectlib/vectlib.c
  src/sha1lib/sha1.c
  src/genx/charProps.c
  src/genx/genx.c
  src/sexpat/xmlparse.c
  src/sexpat/xmlrole.c
  src/sexpat/xmltok.c
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
  src/vidlib/key.c
  src/vidlib/keydefs.c
  src/vidlib/opengl.c
  src/vidlib/vidlib.c
  src/psilib/borders.c
  src/psilib/calctl/calender.c
  src/psilib/calctl/clock.c
  src/psilib/calctl/analog.c
  src/psilib/controls.c
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
  ${OPTION_SOURCE}
  ${ODBC_SOURCE}
  ${JPEG_SOURCE}
  ${ZLIB_SOURCE}
  ${PNG_SOURCE}
  ${FREETYPE_SOURCE}
  ${LAST_GCC_LIBRARY_SOURCE}
)

include_directories( 
  ${SACK_BASE}/include
)


if( USING_OLD_MINGW )
add_definitions( 
  -DMINGW_SUX
)
endif()


if( __WINDOWS__ )
add_definitions( 
  -D__WINDOWS__
)
LIST(APPEND SOURCES
   src/netlib/net_winsock2.c
)
endif(__WINDOWS__)

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library( sack_bag SHARED ${SOURCES} )
add_definitions( -DSACK_CORE_BUILD )

#SET_TARGET_PROPERTIES( sack_bag properties 
SET_TARGET_PROPERTIES(sack_bag PROPERTIES
                  DEFINE_SYMBOL SACK_BAG_EXPORTS
                  COMPILE_FLAGS  -DTARGET_LABEL=${PROJECT_NAME}
)
#SET_TARGET_PROPERTIES(foo-static PROPERTIES PREFIX "lib")
#SET_TARGET_PROPERTIES(foo-static PROPERTIES SUFFIX ".dll")


target_link_libraries( sack_bag wsock32 ws2_32 opengl32 odbc32 winmm iphlpapi)
if( NOT NEED_PNG ) 
  target_link_libraries( sack_bag png )
endif() 
if( NOT NEED_JPEG ) 
target_link_libraries( sack_bag jpeg   )
endif() 
if( NOT NEED_FREETYPE )
target_link_libraries( sack_bag freetype  )
endif()


if( NOT SACK_ONLY OR BUILD_INTERSHELL )
add_subdirectory( src/intershell )
add_subdirectory( src/SQLlib/optlib/editoption )

add_subdirectory( include )
add_subdirectory( data )

endif()

install_default_dest( sack_bag )



