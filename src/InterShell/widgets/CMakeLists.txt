
cmake_minimum_required(VERSION 2.8)
project( sack_widgets ) 

set(SOURCES 
   banner/banner.c 
   keypad/buttons.c 
   keypad/keypad_control.c 
   keypad/accum.c 
)

add_definitions( 
	-DBANNER_SOURCE  -DKEYPAD_SOURCE
)

add_library( ${PROJECT_NAME} SHARED 
   ${FIRST_GCC_LIBRARY_SOURCE}
      ${SOURCES} 
   ${LAST_GCC_LIBRARY_SOURCE}
)

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  FOLDER InterShell
)


target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES})
install_default_dest( ${PROJECT_NAME} )

if( WIN32 )
target_link_libraries( ${PROJECT_NAME} winmm )
endif( WIN32 )

SET( WIDGET_SDK
  include/banner.h include/buttons.h include/keypad.h
)

INSTALL( 
  FILES
    ${WIDGET_SDK}
  DESTINATION
    include/widgets
)

INSTALL( 
  FILES
    ${WIDGET_SDK}
  DESTINATION
    ${SACK_BASE}/include/SACK/widgets )

install_default_dest( ${PROJECT_NAME} )
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION ${SACK_BASE}/bin                     
        	LIBRARY DESTINATION ${SACK_BASE}/bin
	        ARCHIVE DESTINATION ${SACK_BASE}/lib )
else( WIN32 )
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION ${SACK_BASE}/bin 
        	LIBRARY DESTINATION ${SACK_BASE}/lib
	        ARCHIVE DESTINATION ${SACK_BASE}/lib )
endif( WIN32 )

