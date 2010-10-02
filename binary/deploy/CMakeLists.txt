
cmake_minimum_required(VERSION 2.8)

PROJECT( "Sack Binaries" )
add_definitions( -D_X86_ )

configure_file( deploy.cm.c ${CMAKE_BINARY_DIR}/deploy.c )
ADD_EXECUTABLE( ${PROJECT_NAME} ${CMAKE_BINARY_DIR}/deploy.c )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES FOLDER install )

if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION .
)
else( WIN32 )
# should deploy differently for linux. otherwise we'll end up with bin in /
#	install( TARGETS ${PROJECT_NAME}
#	        RUNTIME DESTINATION bin 
#        	LIBRARY DESTINATION lib
#	        ARCHIVE DESTINATION lib )
endif( WIN32 )


