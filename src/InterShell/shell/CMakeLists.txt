


cmake_minimum_required(VERSION 2.8)

project( InterShell_console  )


set(SOURCES
  ${FIRST_GCC_PROGRAM_SOURCE}
   ../../utils/runner/runwin.c 
  ${LAST_GCC_PROGRAM_SOURCE}
)


add_definitions( 
	-DLOAD_LIBNAME="InterShell.core" -DMODE=1
)

add_executable(${PROJECT_NAME} ${SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}"
                  FOLDER InterShell
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell ) 

