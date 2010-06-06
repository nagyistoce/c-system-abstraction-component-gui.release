
cmake_minimum_required(VERSION 2.8)
project( sack_widgets ) 

set(SOURCES 
   banner/banner.c 
   keypad/buttons.c 
   keypad/keypad.c 
   keypad/accum.c 
)

add_definitions( 
	-DBANNER_SOURCE  -DKEYPAD_SOURCE
)

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library( ${PROJECT_NAME} SHARED 
   ${FIRST_GCC_LIBRARY_SOURCE}
      ${SOURCES} 
   ${LAST_GCC_LIBRARY_SOURCE}
)

string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
)


target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} ${SACK_PSI_LIBRARY})
install_default_dest( ${PROJECT_NAME} )


SET( WIDGET_SDK
  include/banner.h include/buttons.h include/keypad.h
)

INSTALL( 
  FILES
    ${WIDGET_SDK}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/widgets
)



