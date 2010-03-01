

cmake_minimum_required(VERSION 2.8)

PROJECT( vlc_interface )
set( VLC_INTERFACE ${PROJECT_NAME} )


include_directories( ${VLC_PATH}/sdk/include )

add_library(${PROJECT_NAME} SHARED 
${FIRST_GCC_LIBRARY_SOURCE} 
	vlc_interface
${LAST_GCC_LIBRARY_SOURCE} )


string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME} sack_bag )

install_default_dest( ${PROJECT_NAME} )



add_subdirectory( test )
add_subdirectory( intershell )

