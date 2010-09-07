


cmake_minimum_required(VERSION 2.8)

if(MSVC)
project( InterShell_window_stop )
else(MSVC)
project( InterShell_window.stop )
endif(MSVC)


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


if(MSVC)
project( InterShell_console_stop )
else(MSVC)
project( InterShell_console.stop )
endif(MSVC)


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

