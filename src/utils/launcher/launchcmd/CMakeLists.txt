
cmake_minimum_required(VERSION 2.8)

project( launchcmd )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    launchcmd
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

