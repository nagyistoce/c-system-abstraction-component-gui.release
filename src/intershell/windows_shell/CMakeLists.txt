

cmake_minimum_required(VERSION 2.8)

project( windows_shell.isp )

set( SOURCES 
${FIRST_GCC_LIBRARY_SOURCE} 
   shell
${LAST_GCC_LIBRARY_SOURCE} )


if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES} )


set( CMAKE_SHARED_LIBRARY_SUFFIX "" )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )
string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
message("${TARGET_LABEL} is from ${PROJECT_NAME}" )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest_binary( ${PROJECT_NAME} )

