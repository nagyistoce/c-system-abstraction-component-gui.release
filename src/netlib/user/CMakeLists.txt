PROJECT( test_user_connect )
add_executable( ${PROJECT_NAME} user )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
