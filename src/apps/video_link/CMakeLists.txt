cmake_minimum_required(VERSION 2.8)

GET_FILENAME_COMPONENT(SACK_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SACK;Install_Dir]" ABSOLUTE CACHE)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
"MinSizeRel" "RelWithDebInfo")


PROJECT( video_link_service )

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX
    ${SACK_BASE} CACHE PATH "Project install prefix; defaults to sack installation" FORCE
    )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include( ${SACK_SDK_ROOT_PATH}/CMakePackage )

include_directories( ${SACK_INCLUDE_DIR} )
LINK_DIRECTORIES( ${SACK_LIBRARY_DIR} )


if( CMAKE_BUILD_TYPE MATCHES "Debug" )
  add_definitions( -D_DEBUG )
endif()
if( CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" )
  add_definitions( -D_DEBUG )
endif()


########################################################################################
########################################################################################

PROJECT( video_link_server )

ADD_EXECUTABLE( ${PROJECT_NAME} WIN32
${FIRST_GCC_PROGRAM_SOURCE} 
  stateserver.c db.c
${LAST_GCC_PROGRAM_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} winmm.lib )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "WINDOWS_MODE;TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\""
                  FOLDER "Video Link"
)
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION bin                     
        	LIBRARY DESTINATION bin
	        ARCHIVE DESTINATION lib )
else( WIN32 )
	install( TARGETS ${PROJECT_NAME}
	        RUNTIME DESTINATION bin 
        	LIBRARY DESTINATION lib
	        ARCHIVE DESTINATION lib )
endif( WIN32 )

########################################################################################
########################################################################################

PROJECT( video_link_server.isp )

ADD_LIBRARY( ${PROJECT_NAME} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  stateserver.c db.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} winmm.lib )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/InterShell/plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin/InterShell/plugins 
  	)
endif()

########################################################################################
########################################################################################

PROJECT( video_link_control.isp )

ADD_LIBRARY( ${PROJECT_NAME} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  statebuttons.c db.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} sack_widgets )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/InterShell/plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin/InterShell/plugins 
  	)
endif()
INSTALL( 
  FILES
    frames/Edit_Link_Button.isFrame
  DESTINATION
    ${DATA_INSTALL_PREFIX}/frames
)



########################################################################################
########################################################################################


PROJECT( vlc_launcher.vplug )
ADD_LIBRARY( ${PROJECT_NAME} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  vlc_launcher.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/Plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin/InterShell/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin
  	)
endif()

########################################################################################
########################################################################################

PROJECT( video_mux.vplug )
ADD_LIBRARY( ${PROJECT_NAME} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  video_mux.c ianson/video_ctrl.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/Plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin/InterShell/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin
  	)
endif()


PROJECT( bdata.proxy.vplug )
ADD_LIBRARY( ${PROJECT_NAME} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  bdata_proxy_task.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER ""
)
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/InterShell/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin/InterShell/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin
  	)
endif()






IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
message( "default once." )
  SET(CMAKE_INSTALL_PREFIX
    ${SACK_BASE} CACHE PATH "Project install prefix; defaults to sack installation" FORCE
    )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


