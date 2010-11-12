
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

macro( install_mode_dest )
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION bin/\${CMAKE_INSTALL_CONFIG_NAME}
        	LIBRARY DESTINATION bin/\${CMAKE_INSTALL_CONFIG_NAME}
	        ARCHIVE DESTINATION lib/\${CMAKE_INSTALL_CONFIG_NAME} )
else( WIN32 )
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION bin 
        	LIBRARY DESTINATION lib
	        ARCHIVE DESTINATION lib )
endif( WIN32 )
endmacro( install_mode_dest )

macro( install_sack_sdk_dest )
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION ${SACK_BASE}/bin/\${CMAKE_INSTALL_CONFIG_NAME}                     
        	LIBRARY DESTINATION ${SACK_BASE}/bin/\${CMAKE_INSTALL_CONFIG_NAME}
	        ARCHIVE DESTINATION ${SACK_BASE}/lib/\${CMAKE_INSTALL_CONFIG_NAME} )
else( WIN32 )
	install( TARGETS ${ARGV}
	        RUNTIME DESTINATION ${SACK_BASE}/bin 
        	LIBRARY DESTINATION ${SACK_BASE}/lib
	        ARCHIVE DESTINATION ${SACK_BASE}/lib )
endif( WIN32 )
endmacro( install_sack_sdk_dest )


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

include( GetPrerequisites )

macro( install_product proj )
  get_prerequisites( ${${PROJECT}_BINARY_DIR}/${PROJECT}${CMAKE_EXECUTABLE_SUFFIX} filelist 1 0 "" "" )
  #exclude_system recurse dirs
  message( "pre-req: ${filelist} ${proj} (${${proj}_BINARY_DIR}/${PROJECT}${CMAKE_EXECUTABLE_SUFFIX})" )
endmacro( install_product )

macro( install_literal_product proj project_target )
  #get_prerequisites( ${${proj}_BINARY_DIR}/${proj} filelist 1 0 "" "" )
  #exclude_system recurse dirs
  
 # GET_TARGET_PROPERTY(LIBS2 ${proj} IMPORTED_LINK_INTERFACE_LIBRARIES )
 # message( "${proj} libs: ${LIBS2}" )
 # GET_TARGET_PROPERTY(LIBS ${proj} IMPORTED_LINK_DEPENDENT_LIBRARIES )
 # message( "libs: ${LIBS}" )
 # #set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )

 # #message( "pre-req: ${proj} (${${proj}_BINARY_DIR}/${proj}) [${filelist}]" )
 # FOREACH( req ${filelist} )
 #   install( FILES ${req} DESTINATION bin/${project_target}  )
 # ENDFOREACH()
if( WIN32 )
  install( TARGETS ${proj} RUNTIME DESTINATION bin/${project_target} 
  	)
else( WIN32 )
  install( TARGETS ${proj} LIBRARY DESTINATION bin/${project_target} 
  	)
endif()
endmacro( install_literal_product )

