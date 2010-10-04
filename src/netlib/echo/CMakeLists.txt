PROJECT( test_echo_server )
add_executable( ${PROJECT_NAME} echo )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  FOLDER tests
)

target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
