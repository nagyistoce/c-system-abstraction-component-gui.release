
cmake_minimum_required(VERSION 2.8)

PROJECT( sack_deploy )

configure_file( deploy.c.cm ${CMAKE_BINARY_DIR}/deploy.c )
configure_file( cmake_environment.h.in ${CMAKE_BINARY_DIR}/cmake_environment.h )
ADD_EXECUTABLE( ${PROJECT_NAME} ${CMAKE_BINARY_DIR}/deploy.c )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES FOLDER install )

#TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} )

if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION .
)
else( WIN32 )
# should deploy differently for linux.
#	install( TARGETS ${PROJECT_NAME}
#	        RUNTIME DESTINATION bin 
#        	LIBRARY DESTINATION lib
#	        ARCHIVE DESTINATION lib )
endif( WIN32 )


