

cmake_minimum_required(VERSION 2.8)

PROJECT( vlc_interface )
set( VLC_INTERFACE ${PROJECT_NAME} )

if( ${CMAKE_GENERATOR} MATCHES "Watcom WMake" )
 set( ExtraFlags "${ExtraFlags} -bd" )
endif()


set( SOURCES
${FIRST_GCC_LIBRARY_SOURCE} 
	vlc_interface
${LAST_GCC_LIBRARY_SOURCE} )

include_directories( ${VLC_PATH}/sdk/include )

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()


add_library(${PROJECT_NAME} SHARED ${SOURCES} )


string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${ExtraFlags}"
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} ${SACK_PSI_LIBRARY} )

install_default_dest( ${PROJECT_NAME} )


add_subdirectory( test )
add_subdirectory( intershell )

