PROJECT( network_tcp_proxy )
add_executable( ${PROJECT_NAME} proxy )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
