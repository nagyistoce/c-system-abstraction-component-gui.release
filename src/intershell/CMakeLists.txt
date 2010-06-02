

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
intershell_plugins.c 
page_dialog.c  
pages.c        
resname.c      
security.c     
sprites.c      
text_label.c
${LAST_GCC_LIBRARY_SOURCE}
)


if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library( ${PROJECT_NAME} SHARED ${SOURCES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  DEFINE_SYMBOL INTERSHELL_SOURCE
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -D__NO_ANIMATION__"
                  SUFFIX ""
                  PREFIX ""
)

target_link_libraries( ${PROJECT_NAME} sack_widgets ${SACK_LIBRARY} )
install_literal_product( ${PROJECT_NAME} InterShell ) 


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
    include/${CMAKE_PROJECT_NAME}/InterShell
)

if( NOT __ARM__ )
add_subdirectory( vlc_hook )
endif()
add_subdirectory( windows_shell )
add_subdirectory( tasks )
add_subdirectory( clock )
add_subdirectory( graph )


# this has to be set before the cmakelists.txt ends... but not before other cmakelists are loaded

if( NOT __ARM__ )
# arm target has no console.
add_subdirectory( shell )
endif( NOT __ARM__ )
add_subdirectory( wshell )




