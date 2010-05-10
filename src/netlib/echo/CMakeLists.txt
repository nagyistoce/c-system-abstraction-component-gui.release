PROJECT( test_echo_server )
add_executable( ${PROJECT_NAME} echo )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
