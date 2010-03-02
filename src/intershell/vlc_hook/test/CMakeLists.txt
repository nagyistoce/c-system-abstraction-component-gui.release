

cmake_minimum_required(VERSION 2.8)

project( vlc_test )


add_executable(${PROJECT_NAME} 
${FIRST_GCC_PROGRAM_SOURCE} 
../vlc_test
${LAST_GCC_PROGRAM_SOURCE} )


string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME} sack_bag ${VLC_INTERFACE} )
install_default_dest( ${PROJECT_NAME} )

if( C++ )
  set( CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" )
  set( CMAKE_C_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}" )
endif()
