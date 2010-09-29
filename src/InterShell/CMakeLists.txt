cmake_minimum_required(VERSION 2.8)

PROJECT( InterShell )

GET_FILENAME_COMPONENT(SACK_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\SACK;Install_Dir]" ABSOLUTE CACHE)

include( ${SACK_SDK_ROOT_PATH}/CMakePackage )

INSTALL_SACK( bin )

include_directories( ${SACK_INCLUDE_DIR} )
LINK_DIRECTORIES( ${SACK_LIBRARY_DIR} )

SET(VLC_PATH "c:/tools/vlc-1.1.4" CACHE STRING "Base Path of VLC Installation 1.1.0 and more" )

add_subdirectory( ../../data __/__/data )
add_subdirectory( widgets )
add_subdirectory( frames )
add_subdirectory( images )
add_subdirectory( calendar )

#project( InterShell.core )
set( PROJECT_OUTPUT InterShell.core )

set(SOURCES 
${FIRST_GCC_LIBRARY_SOURCE}
banner_button.c
fileopen.c     
fonts.c        
loadsave.c     
macros.c       
main.c         
intershell_plugins.c 
page_dialog.c  
pages.c        
resname.c      
security.c     
sprites.c      
text_label.c
${LAST_GCC_LIBRARY_SOURCE}
)


add_library( ${PROJECT_OUTPUT} SHARED ${SOURCES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_OUTPUT} )
SET_TARGET_PROPERTIES(${PROJECT_OUTPUT} PROPERTIES
                  DEFINE_SYMBOL INTERSHELL_SOURCE
                  COMPILE_FLAGS  ""
                  COMPILE_DEFINITIONS "TARGET_LABEL=${TARGET_LABEL};_DEFINE_INTERFACE;__NO_ANIMATION__"
                  SUFFIX ""
                  PREFIX ""
                  FOLDER InterShell

)

target_link_libraries( ${PROJECT_OUTPUT} sack_widgets ${SACK_LIBRARIES} )
install_literal_product( ${PROJECT_OUTPUT} InterShell ) 


include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}
)


SET( INTERSHELL_SDK
  intershell_export.h
  intershell_registry.h
  intershell_button.h
)

INSTALL( 
  FILES
    ${INTERSHELL_SDK}
  DESTINATION
    include/${CMAKE_PROJECT_OUTPUT}/InterShell
)

INSTALL( 
  FILES
    ${INTERSHELL_SDK}
  DESTINATION
    ${SACK_BASE}/include/${CMAKE_PROJECT_OUTPUT}/InterShell
)

if( NOT __ARM__ )
add_subdirectory( vlc_hook )
endif()
add_subdirectory( windows_shell )
add_subdirectory( stop )
add_subdirectory( tasks )
add_subdirectory( clock )
add_subdirectory( graph )


# this has to be set before the cmakelists.txt ends... but not before other cmakelists are loaded

if( NOT __ARM__ )
# arm target has no console.
add_subdirectory( shell )
endif( NOT __ARM__ )
add_subdirectory( wshell )




