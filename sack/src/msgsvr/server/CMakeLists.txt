

cmake_minimum_required(VERSION 2.8)

set( core_service bag.msg.core.service )

project( ${core_service} )


set(SOURCES 
${FIRST_GCC_LIBRARY_SOURCE}
server2
${LAST_GCC_LIBRARY_SOURCE}
)


add_library( ${PROJECT_NAME} SHARED ${SOURCES} )

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  DEFINE_SYMBOL INTERSHELL_SOURCE
                  COMPILE_FLAGS  "-DTARGET_LABEL=bag_msg_core_service"
                  PREFIX ""
                  SUFFIX ""
                  FOLDER "Message Service"
)

target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
install_default_dest_binary( ${PROJECT_NAME} )


project( msgsvr )

set(SERVER_SOURCES 
${FIRST_GCC_PROGRAM_SOURCE}
server3
${LAST_GCC_PROGRAM_SOURCE}
)

add_executable( ${PROJECT_NAME} WIN32 ${SERVER_SOURCES} )
string( REGEX REPLACE "(.*)\\.(.*)" "\\1_\\2" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  DEFINE_SYMBOL INTERSHELL_SOURCE
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -DSERVERMSG_SOURCE -DBAG"
                  FOLDER "Message Service"
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )

install_default_dest_binary( ${PROJECT_NAME} )


