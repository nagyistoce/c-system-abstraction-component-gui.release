cmake_minimum_required(VERSION 2.8)

#set( CMAKE_VERBOSE_MAKEFILE  1 )
#message( ${CMAKE_GENERATOR} )

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Set build type")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
               "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

if( ${CMAKE_GENERATOR} MATCHES "Watcom WMake" )

#ENABLE_LANGUAGE(RC)
##<FLAGS> flags is not OK.
#SET(CMAKE_COMPILE_RESOURCE "rc")
SET( CMAKE_RC_COMPILER "rc" )
#SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> /fo<OBJECT> <SOURCE>  -i${SACK_SOURCE_DIR}")

 set( ExtraFlags "${ExtraFlags} -DSEXPAT_SOURCE -DXML_MIN_SIZE /fp5 /fpi87" )
 # this flag is for build dll?  emit DllRuntime startup link 
 #set( LibraryExtraFlags "-bd" )
 set( ConsoleProgramExtraFlags "-bc" )
 set( WindowsProgramExtraFlags "-bg" )
 
if( CMAKE_BUILD_TYPE MATCHES "Debug" )
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
include( DefaultInstall )
#include( FindSqlite )

OPTION( C++ "Build with C++" OFF )
if( WIN32 )
   set( __WINDOWS__ ON )
else()
   if( UNIX )
      set( __LINUX__ ON )
   endif()
endif()

SET(VLC_PATH "c:/tools/vlc-1.0.5" CACHE STRING "Base Path of VLC Installation 1.0.0 and less than 1.1.0" )

SET(BUILD_ELTANIN_ROOT "../eltanin2/BingoUnified" CACHE STRING "If Specified, hook into eltanin build CmakeLists" )
SET(BUILD_ALTANIK_ROOT "../altanik" CACHE STRING "If Specified, hook into eltanin build CmakeLists" )

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

OPTION( BUILD_INTERSHELL "Build InterShell" ON )
OPTION( SACK_ONLY "Build only sack library, and include structure?" OFF )
OPTION( __WINDOWS__ "Enable specific windows features" ${__WINDOWS__} )
OPTION( __LINUX__ "Enable specific linux features (disable __WINDOWS__)" ${UNIX} )
OPTION( __LINUX64__ "Enable specific linux features (disable __WINDOWS__)" OFF )
OPTION( __ARM__ "Enable specific arm-linux features (disable __WINDOWS__)" OFF )
OPTION( USE_ODBC "Use Odbc" 1 )
OPTION( USE_OPTIONS "Use Option Code all internal option usage (and don't include option code)" 1 )
OPTION( USING_OLD_MINGW "MinGW Has NO UpdateLayeredWindow" ${MINGW} )
OPTION( USE_SQLITE "Use SQLITE (also links static internal)" 1 )
OPTION( USE_SQLITE_EXTERNAL "Use SQLITE (build as external module)" 0 )

OPTION( __NO_GUI__ "No GUI or image parts" 0 )
OPTION( NEED_JPEG "Build internal jpeg lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( NEED_PNG "Build internal png lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( NEED_FREETYPE "Build internal freetype lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( NEED_ZLIB "Build internal zlib lib (with suitable exports to link against)" ${USE_INTERNAL_STANDARDLIBS} )
OPTION( BUILD_ELTANIN "Hook into building eltanin tree" OFF )
OPTION( BUILD_ALTANIK "Hook into building altanik tree" OFF )
OPTION( __WINDOWS_ARM_CE__ "Build for windows CE 5.0 ARM" 0 )
OPTION( USE_ASSEMBLY "Use Assembly files as available.  NASM required for this." OFF )
OPTION( BUILD_MONOLITHIC "Build one giant library" 1 )


if( __ARM__ OR __BUILD_ARM_CE__ )
set( CMAKE_C_COMPILER_WORKS 1 )
set( CMAKE_CXX_COMPILER_WORKS 1 )
add_definitions( -D_WIN32_WCE=0x501 -DUNDER_CE -DWINCE -D_WINDOWS -DARM -D_ARM_ -DWIN32_PLATFORM_WFSP -D_UNICODE -DUNICODE -DPOCKETPC2003_UI_MODEL -DWIN32 )
else( )
  if( MSVC )
    add_definitions( -D_X86_ )
  endif( MSVC )
endif()


add_definitions( -DSACK_CORE_BUILD )


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

 if( USE_ODBC )
add_definitions( -DUSE_ODBC )
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
add_definitions( -D__NO_OPTIONS__ )
 endif( USE_OPTIONS )

 if( USE_SQLITE )
  SET(SQLITE_SOURCES   src/sqlite/sqlite_interface.c )
#if( !SQLITE3_FOUND )
  SET( SQLITE_SOURCES ${SQLITE_SOURCES} src/sqlite/sqlite3.c  )
  if( NOT USE_SQLITE_EXTERNAL )
   # setup to include sqlite in the core libraries...
   set( BAG_SQLITE_SOURCES ${SQLITE_SOURCES} )
   set( ExtraFlags "${ExtraFlags} -DSQLITE_SOURCE" )
  else()
   set( ExtraFlags "${ExtraFlags} -DUSE_SQLITE_INTERFACE" )
  endif()
#else( !SQLITE3_FOUND )
#   set( ExtraFlags "${ExtraFlags} -DSYSTEM_SQLITE" )
#endif( !SQLITE3_FOUND )

  add_definitions( -DUSE_SQLITE )
    
 endif(USE_SQLITE)

else( USE_ODBC OR USE_SQLITE )
message( "no options too..." )
add_definitions( 
  -D__NO_SQL__ -D__NO_OPTIONS__
)

endif( USE_ODBC OR USE_SQLITE )

# this is where external image libraries are added for windows native builds
if( NEED_JPEG OR NEED_PNG OR NEED_FREETYPE )
include( Externals.cmake )
endif()

if( NOT C++ )
  if( ${CMAKE_COMPILER_IS_GNUCC} ) 
  SET( FIRST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/deadstart/deadstart_list.c ) 
  SET( FIRST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_list.c ) 
  SET( LAST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/deadstart/deadstart_lib.c ${SACK_BASE}/src/deadstart/deadstart_end.c ) 
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_lib.c ${SACK_BASE}/src/deadstart/deadstart_prog.c ${SACK_BASE}/src/deadstart/deadstart_end.c ) 
  endif()
  if( ${MSVC}${WATCOM} ) 
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_prog.c ) 
  endif()
else()
  if( ${CMAKE_COMPILER_IS_GNUCC} ) 
  # gcc I still need this, otherwise a program needs to InvokeDeadstarts()
  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/deadstart/deadstart_prog.c ) 
  endif()
endif()


  add_definitions( -D_DEBUG )

if( USE_ASSEMBLY )
set( ExtraFlags "${ExtraFlags} -DHAS_ASSEMBLY )
set( ImageExtraFlags "${ExtraFlags} -DHAS_ASSEMBLY )
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
set( MEM_ASSEMBLY_SOURCES src/memlib/memoryasm.asm )
endif()

# expat is a 'standard' library.  ripped a copy for ease, it's also incredibly simple.
set ( EXPAT_SOURCES 
  src/sexpat/xmlparse.c
  src/sexpat/xmlrole.c
  src/sexpat/xmltok.c
)
set( ExtraFlags "${ExtraFlags} -DSEXPAT_SOURCE -DXML_MIN_SIZE" )

# genx is searchable by looking for 'genx xml'
set( GENX_SOURCES
  src/genx/charProps.c
  src/genx/genx.c
)

if( __WINDOWS__ )
set( VIDLIB_SOURCES
  src/vidlib/key.c
  src/vidlib/keydefs.c
  src/vidlib/vidlib.c
)

if( NOT __ARM__ )
set( VIDLIB_SOURCES ${VIDLIB_SOURCES} src/vidlib/opengl.c )
set( ExtraRenderFlags "${ExtraRenderFlags} -D_OPENGL_ENABLED" )
endif( NOT __ARM__ )

else( __WINDOWS__ )

endif( __WINDOWS__ )

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

set( PSI_SOURCES
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
  src/psilib/option_frame.c
)

if( __WINDOWS__ )
  SET( PSI_SOURCES ${PSI_SOURCES}  src/systraylib/systray.c )
else( __WINDOWS__ )
  SET( PSI_SOURCES ${PSI_SOURCES}  src/systraylib/systray_client.c )
  
endif( __WINDOWS__ )


if( NOT BUILD_MONOLITHIC )
set( PSI_SOURCES ${PSI_SOURCES} 
  src/psilib/fntcache.c
)
endif()

set( ExtraFlags "${ExtraFlags} -DGENX_SOURCE" )

set( SACK_MSG_SERVICES 
  src/msgsvr/client/client.c
  src/msgsvr/summoner/construct.c
  )

if( NOT __ARM__ )
set(BASE_SOURCES ${BASE_SOURCES} ${SACK_MSG_SERVICES}
  src/netlib/ping.c
  src/netlib/whois.c
  src/systemlib/spawntask.c
  src/filesyslib/filemon/allfiles.c
  src/filesyslib/winfiles.c
)
if( __LINUX__ )
set(BASE_SOURCES ${BASE_SOURCES}
  src/filesyslib/filemon/linuxfiles.c
)
else( __LINUX__ )
set(BASE_SOURCES ${BASE_SOURCES}
  src/filesyslib/filemon/windowsfiles.c
)
endif( __LINUX__ )

else()
set( Extraflags "${ExtraFlags} -D__NO_MSGSVR__" )
set( CMAKE_C_STANDARD_LIBRARIES_INIT "" )
endif()

if( __WINDOWS__ )
set( MSG_QUEUE_SOURCES
  src/typelib/msgqueue.c
)
endif( __WINDOWS__ )

set(BASE_SOURCES  ${BASE_SOURCES}
  src/sysloglib/syslog.c
  src/memlib/sharemem.c
  ${MEM_ASSEMBLY_SOURCES}
  src/typelib/binarylist.c
  src/typelib/familytree.c
  src/typelib/input.c
  src/typelib/sets.c
  src/typelib/text.c
  src/typelib/typecode.c
  ${MSG_QUEUE_SOURCES}
  src/timerlib/timers.c
  src/idlelib/idle.c
  src/fractionlib/fractions.c
  src/configlib/configscript.c
  src/procreglib/names.c
  src/netlib/network.c
  src/netlib/tcpnetwork.c
  src/netlib/udpnetwork.c
  src/systemlib/args.c
  src/deadstart/deadstart_core.c
  src/vectlib/vectlib.c
  src/sha1lib/sha1.c
  src/filesyslib/pathops.c
  src/filesyslib/filescan.c
  src/systemlib/system.c
  ${OPTION_SOURCE}
  ${GENX_SOURCES}
  ${EXPAT_SOURCES}
)


if( __WINDOWS__ )
set(BASE_SOURCES ${BASE_SOURCES}
  src/netlib/net_winsock2.c
)
endif( __WINDOWS__ )


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
LIST(APPEND BASE_SOURCES
   src/netlib/net_winsock2.c
)
endif(__WINDOWS__)

if( C++ )
set_source_files_properties( ${FIRST_GCC_LIBRARY_SOURCE} ${LAST_GCC_LIBRARY_SOURCE} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${FIRST_GCC_PROGRAM_SOURCE} ${LAST_GCC_PROGRAM_SOURCE} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${BASE_SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${IMAGE_SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${VIDLIB_SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${PSI_SOURCES} PROPERTIES LANGUAGE "CXX" )
#set_source_files_properties( ${JPEG_SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${ODBC_SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${OPTION_SOURCES} PROPERTIES LANGUAGE "CXX" )

#set_source_files_properties( ${PNG_SOURCES} PROPERTIES LANGUAGE "CXX" )
#set_source_files_properties( ${ZLIB_SOURCES} PROPERTIES LANGUAGE "CXX" )
#set_source_files_properties( ${FREETYPE_SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

if( NOT WATCOM )
  LIST(APPEND BASE_SOURCES
     all_resources.rc )
else()
add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/all_resources.res
                     DEPENDS ${SACK_BASE}/all_resources.rc
                     COMMAND ${CMAKE_RC_COMPILER} -i${SACK_BASE} -fo${PROJECT_BINARY_DIR}/all_resources.res ${SACK_BASE}/all_resources.rc
 )
 ADD_CUSTOM_TARGET( generate_foo DEPENDS ${PROJECT_BINARY_DIR}/all_resources.res)
endif( NOT WATCOM ) 

if( BUILD_MONOLITHIC )

if( NOT __NO_GUI__ )
set( BASE_SOURCES ${BASE_SOURCES} 
    ${IMAGE_SOURCES} 
    ${VIDLIB_SOURCES} 
    ${PSI_SOURCES} 
)
endif( NOT __NO_GUI__ )

set( SACK_SOURCES 
  ${FIRST_GCC_LIBRARY_SOURCE} 
    ${BASE_SOURCES} 
    ${ODBC_SOURCES}
    ${JPEG_SOURCE}
    ${ZLIB_SOURCE}
    ${PNG_SOURCE}
    ${FREETYPE_SOURCE}
  ${LAST_GCC_LIBRARY_SOURCE} 
)

add_library( sack_bag SHARED 
  ${SACK_SOURCES} 
  )

set( cplusplus_sources )

FOREACH(SOURCE ${SACK_SOURCES})
   if( ${SOURCE} MATCHES "(.*)\\.c" )
     set( BASENAME ${CMAKE_MATCH_1} )
     if( ${SOURCE} MATCHES "${PROJECT_SOURCE_DIR}.*" )
       if( ${SOURCE} IS_NEWER_THAN ${BASENAME}.cpp )
          EXECUTE_PROCESS(COMMAND cmake -E copy_if_different ${SOURCE} ${BASENAME}.cpp )
          #message( "LIB2_SOURCE1 : ${BASENAME}  ${SOURCE}" )
        endif()
      else()
        if( ${PROJECT_SOURCE_DIR}/${SOURCE} IS_NEWER_THAN ${PROJECT_SOURCE_DIR}/${BASENAME}.cpp )
          EXECUTE_PROCESS(COMMAND cmake -E copy_if_different ${PROJECT_SOURCE_DIR}/${SOURCE} ${PROJECT_SOURCE_DIR}/${BASENAME}.cpp )
          #message( "LIB2_SOURCE2 : ${BASENAME}  ${SOURCE}" )
        endif()
      endif()
      set( cplusplus_sources ${cplusplus_sources} ${BASENAME}.cpp )
   else()
      message( "NOT THING ${SOURCE}" )
   endif()
ENDFOREACH(SOURCE)

set( SACK_SOURCES ${SACK_SOURCES}
    ${BAG_SQLITE_SOURCES}
)


set( SACK_SOURCES ${SACK_SOURCES}
  ${LAST_GCC_LIBRARY_SOURCE} 
)

message( SOURCES:${SACK_SOURCES} )
message( C++SOURCE:${cplusplus_sources} )

#set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )


SET_TARGET_PROPERTIES(sack_bag PROPERTIES
                  DEFINE_SYMBOL SACK_BAG_EXPORTS
                  COMPILE_FLAGS  "-DTARGET_LABEL=sack_bag ${ExtraFlags} ${ExtraRenderFlags}"
		  SOLUTION_FOLDER "MyFolder" 
)


add_library( sack_bag++ SHARED 
  ${cplusplus_sources} 
)
SET_TARGET_PROPERTIES(sack_bag++ PROPERTIES
                  DEFINE_SYMBOL SACK_BAG_EXPORTS
                  COMPILE_FLAGS  "-DTARGET_LABEL=sack_bag_plusplus ${ExtraFlags} ${ExtraRenderFlags}"
		  SOLUTION_FOLDER "sack_bag++" 
)

if( __WINDOWS_ARM_CE__ )
SET_TARGET_PROPERTIES( sack_bag PROPERTIES LINK_LIBRARIES "" )
endif()

if( WATCOM )
ADD_DEPENDENCIES( sack_bag generate_foo )
SET_TARGET_PROPERTIES(sack_bag PROPERTIES LINK_FLAGS "option resource=all_resources.res")

endif()

set( SACK_LIBRARY sack_bag )
set( SACK_LIBRARIES sack_bag )

if( __WINDOWS__ )
  if( __ARM__ )
    target_link_libraries( sack_bag ws2 mmtimer iphlpapi coredll corelibc.lib)
    target_link_libraries( sack_bag++ ws2 mmtimer iphlpapi coredll corelibc.lib)
  else()
    target_link_libraries( sack_bag wsock32 ws2_32 opengl32 winmm iphlpapi)
    target_link_libraries( sack_bag++ wsock32 ws2_32 opengl32 winmm iphlpapi)
  endif()
else( __WINDOWS__ )
  target_link_libraries( sack_bag odbc ${SQLITE3_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} )
  target_link_libraries( sack_bag++ odbc ${SQLITE3_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} )
endif( __WINDOWS__ )

install_default_dest( sack_bag )
install_default_dest( sack_bag++ )

if( NOT NEED_PNG ) 
  target_link_libraries( sack_bag png )
  target_link_libraries( sack_bag++ png )
endif() 
if( NOT NEED_JPEG ) 
  target_link_libraries( sack_bag jpeg   )
  target_link_libraries( sack_bag++ jpeg   )
endif() 
if( NOT NEED_FREETYPE )
  target_link_libraries( sack_bag  freetype  )
  target_link_libraries( sack_bag++  freetype  )
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
set( SACK_LIBRARY bag bag.psi bag.image bag.video )

###########

if( NEED_FREETYPE OR NEED_JPEG OR NEED_PNG )
set( SACK_LIBRARY ${SACK_LIBRARY} bag.externals )
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
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_psi ${ExtraFlags} -DSOURCE_PSI2 -DSYSTRAY_LIBRARAY -DFORCE_NO_INTERFACE"
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
add_definitions( -DSACK_CORE_BUILD )

add_definitions( -D__OBSCURE_STREAM_INTERNALS )


  if( USE_SQLITE_EXTERNAL )
   set( ExtraFlags "${ExtraFlags} -DUSE_SQLITE_INTERFACE" )
   add_library( bag.sqlite3.external SHARED 
      ${FIRST_GCC_LIBRARY_SOURCE} 
      ${SQLITE_SOURCES}
      ${LAST_GCC_LIBRARY_SOURCE} )
   # required link back to bag for register interface
   target_link_libraries( bag.sqlite3.external ${SACK_LIBRARY} )
   install_default_dest( bag.sqlite3.external )
   SET_TARGET_PROPERTIES( bag.sqlite3.external PROPERTIES 
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_sqlite3 -DSQLITE_SOURCE"
                  SUFFIX ""
                  PREFIX ""
   )
   
  endif()

#add_subdirectory( src/displaylib )

if( NOT __NO_GUI__ )
add_subdirectory( src/msgsvr/server )
endif( NOT __NO_GUI__ )

if( NOT SACK_ONLY )
 if( NOT __NO_GUI__ )
  if( BUILD_INTERSHELL )
   add_subdirectory( src/intershell )
  endif( BUILD_INTERSHELL )
  
  add_subdirectory( src/psilib/paltst )

 endif( NOT __NO_GUI__ )

# parts of SDK ...
add_subdirectory( include )
add_subdirectory( src/utils/runner )
add_subdirectory( src/deadstart )

add_subdirectory( data )
# includes some testdata in tstdisplay...
# the test programs do not have install targets, but they will end up building...
 if( NOT __NO_GUI__ )
  add_subdirectory( src/msgsvr/testdisplay )
 endif( NOT __NO_GUI__ )
#add_subdirectory( src/bench/mem )

endif( NOT SACK_ONLY )

 if( NOT __NO_GUI__ )
if( USE_OPTIONS )
 add_subdirectory( src/SQLlib/optlib/editoption )
endif( USE_OPTIONS )
 endif( NOT __NO_GUI__ )


if( BUILD_ELTANIN )
add_subdirectory( ${BUILD_ELTANIN_ROOT} eltanin )
endif( BUILD_ELTANIN )
if( BUILD_ALTANIK )
add_subdirectory( ${BUILD_ALTANIK_ROOT} altanik )
endif( BUILD_ALTANIK )

add_subdirectory( src/netlib/user )
add_subdirectory( src/netlib/echo )
add_subdirectory( src/utils/deploy )

set( CMAKE_C_STANDARD_LIBRARIES_INIT "" )

