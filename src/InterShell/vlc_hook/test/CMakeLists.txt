

cmake_minimum_required(VERSION 2.8)

project( vlc_test )

set( SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
../vlc_test
${LAST_GCC_PROGRAM_SOURCE} )

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_executable(${PROJECT_NAME} ${SOURCES} )


string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} ${VLC_INTERFACE} )
install_default_dest( ${PROJECT_NAME} )

