
cmake_minimum_required(VERSION 2.8)

project( launchpad )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad
${LAST_GCC_PROGRAM_SOURCE}
)


if( C++ )
set_source_files_properties( ${BASE_SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_executable(${PROJECT_NAME} WIN32 ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )


project( launchpad.isp ) 

set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad ishell_plugin
${LAST_GCC_PROGRAM_SOURCE}
)

if( C++ )
set_source_files_properties( ${BASE_SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()
include_directories( ${SACK_BASE}/src/InterShell )
add_library(${PROJECT_NAME} SHARED ${BASE_SOURCES} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags} -DISHELL_PLUGIN"
                  PREFIX ""
                  SUFFIX ""
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_literal_product( ${PROJECT_NAME} InterShell ) 


if( altanik_is_built )
project( launchpad.isom ) 

set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchpad milk_plugin
${LAST_GCC_PROGRAM_SOURCE}
)

if( C++ )
set_source_files_properties( ${BASE_SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library(${PROJECT_NAME} SHARED ${BASE_SOURCES} )
string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  PREFIX ""
                  SUFFIX ""
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest( ${PROJECT_NAME} )
                      endif()

add_subdirectory( frames )

