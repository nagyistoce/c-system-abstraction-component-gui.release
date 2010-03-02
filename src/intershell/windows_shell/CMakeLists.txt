

cmake_minimum_required(VERSION 2.8)

project( windows_shell.isp )


add_library(${PROJECT_NAME} SHARED 
${FIRST_GCC_LIBRARY_SOURCE} 
   shell
${LAST_GCC_LIBRARY_SOURCE} )


set( CMAKE_SHARED_LIBRARY_SUFFIX "" )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )
string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
message("${TARGET_LABEL} is from ${PROJECT_NAME}" )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME} sack_bag )
install_default_dest_binary( ${PROJECT_NAME} )

if( C++ )
  set( CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" )
  set( CMAKE_C_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}" )
endif()

