
cmake_minimum_required(VERSION 2.8)
project( sack_widgets ) 

message( project:${PROJECT_NAME} )


set(SOURCES 
   banner/banner 
   keypad/buttons 
   keypad/keypad 
   keypad/accum 
)



include_directories( 
  M:/sack/include
)

add_definitions( 
	-DBANNER_SOURCE  -DKEYPAD_SOURCE
)

add_library( ${PROJECT_NAME} SHARED 
   ${FIRST_GCC_LIBRARY_SOURCE}
      ${SOURCES} 
   ${LAST_GCC_LIBRARY_SOURCE}
)

string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "")
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SUFFIX "")


target_link_libraries( ${PROJECT_NAME} sack_bag )
install_default_dest( ${PROJECT_NAME} )


if( C++ )
  set( CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" )
  set( CMAKE_C_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}" )
endif()



