

cmake_minimum_required(VERSION 2.8)

project( network_graph.isp )

set( SOURCES
${FIRST_GCC_LIBRARY_SOURCE} 
global.c linegraph.c connect_stat.c packet_stat.c
${LAST_GCC_LIBRARY_SOURCE}
)

if( C++ )
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES} )



string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  PREFIX ""
                  SUFFIX ""
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARY} )
install_default_dest_binary( ${PROJECT_NAME} )

add_subdirectory( frames )

