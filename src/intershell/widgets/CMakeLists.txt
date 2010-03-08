
cmake_minimum_required(VERSION 2.8)
project( sack_widgets ) 

set(SOURCES 
   banner/banner 
   keypad/buttons 
   keypad/keypad 
   keypad/accum 
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
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "")
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SUFFIX "")


target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} ${SACK_PSI_LIBRARY})
install_default_dest( ${PROJECT_NAME} )




