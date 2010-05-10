
PROJECT( scan_ports )
add_executable( ${PROJECT_NAME} scan )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )

PROJECT( scan_addresses )
add_executable( ${PROJECT_NAME} pinger )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )

PROJECT( nameof )
add_executable( ${PROJECT_NAME} nameof )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )

PROJECT( ipof )
add_executable( ${PROJECT_NAME} ipof )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} )
