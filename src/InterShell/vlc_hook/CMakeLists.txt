

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

SET(VLC_PATH "c:/tools/vlc-1.1.4" CACHE STRING "Base Path of VLC Installation 1.1.0 and more" )

include_directories( ${VLC_PATH}/sdk/include )

add_library(${PROJECT_NAME} SHARED ${SOURCES} )

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES 
        COMPILE_FLAGS "-DTARGET_LABEL=${TARGET_LABEL} ${ExtraFlags}" 
        FOLDER "Video Streaming" 
)

target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
if( WIN32 )
target_link_libraries( ${PROJECT_NAME} winmm )
endif( WIN32 )

install_default_dest( ${PROJECT_NAME} )
install_literal_product( ${PROJECT_NAME} plugins ) 


add_subdirectory( test )
add_subdirectory( intershell )

