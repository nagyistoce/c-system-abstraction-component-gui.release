
cmake_minimum_required(VERSION 2.8)

project( service_list )


set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    servicelist
${LAST_GCC_PROGRAM_SOURCE}
)

add_executable(${PROJECT_NAME} ${BASE_SOURCES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}" 
                  FOLDER "Message Service"
)
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )

install_default_dest( ${PROJECT_NAME} )

