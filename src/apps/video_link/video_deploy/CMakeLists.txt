
PROJECT( video_link_deploy )

ADD_EXECUTABLE( ${PROJECT_NAME} WIN32
${FIRST_GCC_PROGRAM_SOURCE} 
  deploy.c regacces.c
${LAST_GCC_PROGRAM_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} winmm.lib )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "WINDOWS_MODE;TARGET_LABEL=${TARGET_LABEL};TARGETNAME=\"${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\""
                  FOLDER "Video Link"
)
install_default_dest( ${PROJECT_NAME} )
install( FILES ClearShell.exe setcomputer.ps1
  DESTINATION bin )
install( FILES configurations/Stations.Map 
  DESTINATION resources )

