


cmake_minimum_required(VERSION 2.8)

project( InterShell_window.stop )


set(SOURCES 
  ${FIRST_GCC_PROGRAM_SOURCE}
   ../stop.c 
  ${LAST_GCC_PROGRAM_SOURCE}
)

add_executable(${PROJECT_NAME} ${SOURCES})
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME}  ${SACK_LIBRARY} )
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell ) 

project( InterShell_console.stop )


set(SOURCES 
  ${FIRST_GCC_PROGRAM_SOURCE}
   ../stop.c 
  ${LAST_GCC_PROGRAM_SOURCE}
)

add_executable(${PROJECT_NAME} ${SOURCES})
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
)
target_link_libraries( ${PROJECT_NAME}  ${SACK_LIBRARY} )
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell ) 

