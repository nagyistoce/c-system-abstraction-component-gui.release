

cmake_minimum_required(VERSION 2.8)

project( vlc_button.isp )

set( SOURCES
${FIRST_GCC_LIBRARY_SOURCE} 
vlc_button
${LAST_GCC_LIBRARY_SOURCE} 
)

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES} )



string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} ${VLC_INTERFACE} sack_widgets)
install_default_dest_binary( ${PROJECT_NAME} )

