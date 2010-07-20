


cmake_minimum_required(VERSION 2.8)

project( InterShell_window )


set(SOURCES 
  ${FIRST_GCC_PROGRAM_SOURCE}
   ../../utils/runner/runwin.c 
  ${LAST_GCC_PROGRAM_SOURCE}
)

add_definitions( -DLOAD_LIBNAME="InterShell.core" -DMODE=0 )

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}"
)
target_link_libraries( ${PROJECT_NAME}  ${SACK_LIBRARY} )
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell ) 

