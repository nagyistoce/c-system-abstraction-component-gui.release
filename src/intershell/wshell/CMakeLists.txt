


cmake_minimum_required(VERSION 2.8)

set( PROJECT InterShell_window )
project( ${PROJECT} )


set(SOURCES 
   ../../utils/runner/runwin.c 
)

include_directories( 
  M:/sack/include
)


add_definitions( 
	-DLOAD_LIBNAME="${CMAKE_SHARED_LIBRARY_PREFIX}InterShell.core${CMAKE_SHARED_LIBRARY_SUFFIX}" -DMODE=0
)

add_executable(${PROJECT} WIN32 ${SOURCES})
target_link_libraries( ${PROJECT} sack_bag )
install_default_dest( ${PROJECT} )

