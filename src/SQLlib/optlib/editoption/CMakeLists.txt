
cmake_minimum_required(VERSION 2.8)

project( EditOptions )


set(SOURCES
    editopt
)


add_executable(${PROJECT_NAME} ${FIRST_GCC_PROGRAM_SOURCE} ${SOURCES} ${LAST_GCC_PROGRAM_SOURCE} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${PROJECT_NAME}"
)
target_link_libraries( ${PROJECT_NAME} sack_bag )
install_default_dest( ${PROJECT_NAME} )


if( C++ )
  set( CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" )
  set( CMAKE_C_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}" )
endif()
