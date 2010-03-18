
SET( HEADER_INSTALL_PREFIX include )
SET( DATA_INSTALL_PREFIX resources )

macro( install_default_dest )
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION bin                     
        	LIBRARY DESTINATION bin
	        ARCHIVE DESTINATION lib )
else( WIN32 )
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION bin 
        	LIBRARY DESTINATION lib
	        ARCHIVE DESTINATION lib )
endif( WIN32 )
endmacro( install_default_dest )

macro( install_default_dest_binary )
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION bin 
        	LIBRARY DESTINATION bin )
else( WIN32 )
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION bin 
        	LIBRARY DESTINATION lib )
endif( WIN32 )
endmacro( install_default_dest_binary )

