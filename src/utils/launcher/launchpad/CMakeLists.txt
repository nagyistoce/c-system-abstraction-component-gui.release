
cmake_minimum_required(VERSION 2.8)

project( launchpad )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad launchpad.rc
${LAST_GCC_PROGRAM_SOURCE}
)

if( MINGW )

SET(CMAKE_RC_COMPILER_INIT "windres")
SET(CMAKE_RC_COMPILER "windres")
SET(CMAKE_RC_COMPILE_OBJECT "windres <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")
ENABLE_LANGUAGE(RC)

endif()


add_executable(${PROJECT_NAME} WIN32 ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest_binary( ${PROJECT_NAME} )


project( launchpad.isp ) 

set(BASE_SOURCES2
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad ishell_plugin
${LAST_GCC_PROGRAM_SOURCE}
)

include_directories( ${SACK_BASE}/src/InterShell )
add_library(${PROJECT_NAME} SHARED ${BASE_SOURCES2} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  PREFIX ""
                  SUFFIX ""
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_literal_product( ${PROJECT_NAME} InterShell ) 


if( altanik_is_built )
project( launchpad.isom ) 

set(BASE_SOURCES3
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad milk_plugin
${LAST_GCC_PROGRAM_SOURCE}
)

add_library(${PROJECT_NAME} SHARED ${BASE_SOURCES3} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags} -DISHELL_PLUGIN"
                  PREFIX ""
                  SUFFIX ""
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )
                      endif()

add_subdirectory( frames )

