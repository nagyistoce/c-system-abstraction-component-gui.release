

cmake_minimum_required(VERSION 2.8)

project( windows_shell.isp )

set( SOURCES 
${FIRST_GCC_LIBRARY_SOURCE} 
   shell
${LAST_GCC_LIBRARY_SOURCE} )


add_library(${PROJECT_NAME} SHARED ${SOURCES} )


string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  SUFFIX ""
                  PREFIX ""
                  FOLDER InterShell/plugins
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
install_literal_product( ${PROJECT_NAME} InterShell/plugins ) 

