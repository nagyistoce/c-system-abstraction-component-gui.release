cmake_minimum_required(VERSION 2.8)

if( NOT SACK_BASE )
SET( ENABLE_CPACK 1 )
endif( NOT SACK_BASE )

GET_FILENAME_COMPONENT(SACK_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\SACK;Install_Dir]" ABSOLUTE CACHE)
GET_FILENAME_COMPONENT(SACK_BINARY_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\Sack.Binaries;Install_Dir]" ABSOLUTE CACHE)

GET_FILENAME_COMPONENT(INTERSHELL_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\InterShell;Install_Dir]" ABSOLUTE CACHE)

PROJECT( video_link )

include( ${SACK_SDK_ROOT_PATH}/CMakePackage )
include( ${SACK_BINARY_SDK_ROOT_PATH}/CMakePackage )
include( ${INTERSHELL_SDK_ROOT_PATH}/CMakePackage )

INSTALL_INTERSHELL( . )
INSTALL_SACK_BINARY( proxy.service bin )
INSTALL_SACK_BINARY( SetOption bin )

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

set( PROJECT_OUTPUT video_link_service )

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX
    ${SACK_BASE} CACHE PATH "Project install prefix; defaults to sack installation" FORCE
    )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


include_directories( ${SACK_INCLUDE_DIR} )
LINK_DIRECTORIES( ${SACK_LIBRARY_DIR} )
include_directories( ${INTERSHELL_INCLUDE_DIR} )
LINK_DIRECTORIES( ${INTERSHELL_LIBRARY_DIR} )


########################################################################################
########################################################################################

#PROJECT( video_link_server )
set( PROJECT_OUTPUT video_link_server )

ADD_EXECUTABLE( ${PROJECT_OUTPUT} WIN32
${FIRST_GCC_PROGRAM_SOURCE} 
  stateserver.c db.c
${LAST_GCC_PROGRAM_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} winmm.lib )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "WINDOWS_MODE;TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}${CMAKE_EXECUTABLE_SUFFIX}\""
                  FOLDER "Video Link"
)
if( WIN32 )
	# On Windows platforms, the dynamic libs should
	# go in the same dir as the executables.
	install( TARGETS ${PROJECT_OUTPUT}
	        RUNTIME DESTINATION bin                     
        	LIBRARY DESTINATION bin
	        ARCHIVE DESTINATION lib )
else( WIN32 )
	install( TARGETS ${PROJECT_OUTPUT}
	        RUNTIME DESTINATION bin 
        	LIBRARY DESTINATION lib
	        ARCHIVE DESTINATION lib )
endif( WIN32 )

########################################################################################
########################################################################################

#PROJECT( video_link_server.isp )
set( PROJECT_OUTPUT video_link_server.isp )

ADD_LIBRARY( ${PROJECT_OUTPUT} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  stateserver.c db.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} winmm.lib )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/InterShell/plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin/plugins 
  	)
endif()

########################################################################################
########################################################################################

#PROJECT( video_link_control.isp )
set( PROJECT_OUTPUT video_link_control.isp )

ADD_LIBRARY( ${PROJECT_OUTPUT} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  statebuttons.c db.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} sack_widgets )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/InterShell/plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin/plugins 
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


#PROJECT( vlc_launcher.vplug )
set( PROJECT_OUTPUT vlc_launcher.vplug )
ADD_LIBRARY( ${PROJECT_OUTPUT} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  vlc_launcher.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/Plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin
  	)
endif()

########################################################################################
########################################################################################

#PROJECT( video_event_notices.vplug )
set( PROJECT_OUTPUT video_event_notices.vplug )
ADD_LIBRARY( ${PROJECT_OUTPUT} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  network_events.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/Plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin
  	)
endif()

########################################################################################
########################################################################################

#PROJECT( video_mux.vplug )
set( PROJECT_OUTPUT video_mux.vplug )
ADD_LIBRARY( ${PROJECT_OUTPUT} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  video_mux.c ianson/video_ctrl.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Video Link/Plugins"
)
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin
  	)
endif()

########################################################################################
########################################################################################


#PROJECT( bdata.proxy.vplug )
set( PROJECT_OUTPUT bdata.proxy.vplug )
ADD_LIBRARY( ${PROJECT_OUTPUT} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
  bdata_proxy_task.c
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_OUTPUT} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_OUTPUT}\""
                  PREFIX ""
                  SUFFIX ""
                  FOLDER ""
)
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin/plugins 
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin/plugins 
  	)
endif()
if( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} RUNTIME DESTINATION bin
  	)
else( WIN32 )
  install( TARGETS ${PROJECT_OUTPUT} LIBRARY DESTINATION bin
  	)
endif()


add_subdirectory( video_deploy )
add_subdirectory( video_map_editor )

#install( FILES install/CMakeLists.txt DESTINATION install/video_link )
#install( FILES vlc/vlc-1.1.4-win32.7z DESTINATION install/video_link )
#install( FILES mysql/mysql-5.4-minimal.zip DESTINATION install/video_link )
#install( FILES tools/pkzip25.exe DESTINATION install/video_link )
#install( FILES tools/7za.exe DESTINATION install/video_link )

if( ENABLE_CPACK )

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video Link 1.0")
SET(CPACK_PACKAGE_VENDOR "Fortunet")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/release.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/liscense.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

SET(CPACK_NSIS_CREATE_ICONS "CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Video Map Editor.lnk' '\$INSTDIR\\\\bin\\\\video_map_editor.exe'") 
SET(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Video Link Deploy.lnk' '\$INSTDIR\\\\bin\\\\video_link_deploy.exe'
               CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Start Video Link Server.lnk' '\$INSTDIR\\\\bin\\\\video_link_server.exe' 
               CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Video Link Controller.lnk' '\$INSTDIR\\\\bin\\\\intershell_window.exe' video_control.config") 

IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resource\\\\junk.ico")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\sack_bag.dll")
  SET(CPACK_NSIS_DISPLAY_NAME "Video Link ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
  SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/MyExecutable")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

#SET(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")

include(CPack)
endif( ENABLE_CPACK )


