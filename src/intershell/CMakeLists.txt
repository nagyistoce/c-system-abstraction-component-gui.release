

cmake_minimum_required(VERSION 2.8)
add_subdirectory( widgets )
add_subdirectory( frames )
add_subdirectory( images )

project( InterShell.core )


set(SOURCES 
${FIRST_GCC_LIBRARY_SOURCE}
banner_button.c
fileopen.c     
fonts.c        
loadsave.c     
macros.c       
main.c         
InterShell_plugins.c 
page_dialog.c  
pages.c        
resname.c      
security.c     
sprites.c      
text_label.c
${LAST_GCC_LIBRARY_SOURCE}
)



add_library( ${PROJECT_NAME} SHARED ${SOURCES} )

string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  DEFINE_SYMBOL INTERSHELL_SOURCE
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -D__NO_ANIMATION__"
)


target_link_libraries( ${PROJECT_NAME} sack_widgets sack_bag )
install_default_dest_binary( ${PROJECT_NAME} )

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

install_default_dest_binary( ${PROJECT_NAME} )



SET( INTERSHELL_SDK
  intershell_export.h
  intershell_registry.h
  intershell_button.h
)


INSTALL( 
  FILES
    ${INTERSHELL_SDK}
  DESTINATION
    include/InterShell
)


add_subdirectory( vlc_hook )
add_subdirectory( windows_shell )
add_subdirectory( tasks )


# this has to be set before the cmakelists.txt ends... but not before other cmakelists are loaded
set( CMAKE_SHARED_LIBRARY_SUFFIX "" )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )
add_subdirectory( shell )
add_subdirectory( wshell )









