
cmake_minimum_required(VERSION 2.8)

PROJECT( sack_deploy )

configure_file( deploy.cm.c ${CMAKE_BINARY_DIR}/deploy.c )
ADD_EXECUTABLE( ${PROJECT_NAME} ${CMAKE_BINARY_DIR}/deploy.c )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES FOLDER install )

if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
        if(MSVC) 
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION bin/\${CMAKE_INSTALL_CONFIG_NAME} )
        else(MSVC)
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION bin/${CMAKE_BUILD_TYPE} )
        endif(MSVC)
else( WIN32 )
# should deploy differently for linux.
#	install( TARGETS ${PROJECT_NAME}
#	        RUNTIME DESTINATION bin 
#        	LIBRARY DESTINATION lib
#	        ARCHIVE DESTINATION lib )
endif( WIN32 )


