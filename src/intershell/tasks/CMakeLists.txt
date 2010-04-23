

cmake_minimum_required(VERSION 2.8)

project( tasks.isp )

set( SOURCES
${FIRST_GCC_LIBRARY_SOURCE} 
tasks
${LAST_GCC_LIBRARY_SOURCE}
)

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES} )



string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
message("${TARGET_LABEL} is from ${PROJECT_NAME}" )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  SUFFIX ""
                  PREFIX ""
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} sack_widgets )
install_default_dest_binary( ${PROJECT_NAME} )

add_subdirectory( frames )

if( C++ )
  set( CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" )
  set( CMAKE_C_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}" )
endif()
