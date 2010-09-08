
cmake_minimum_required(VERSION 2.8)

PROJECT( sack_deploy )

ADD_EXECUTABLE( ${PROJECT_NAME} deploy )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  FOLDER install
)

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


