


cmake_minimum_required(VERSION 2.8)

project( InterShell_window )


set(SOURCES 
  ${FIRST_GCC_PROGRAM_SOURCE}
   ../../utils/runner/runwin.c 
  ${LAST_GCC_PROGRAM_SOURCE}
)

add_definitions( -DLOAD_LIBNAME="InterShell.core" -DMODE=0 )

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
target_link_libraries( ${PROJECT_NAME}  ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )

