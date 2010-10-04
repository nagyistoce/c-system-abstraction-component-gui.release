
cmake_minimum_required(VERSION 2.8)

project( task.frames )


SET( INTERSHELL_FRAMES
CommonTaskProperties.isFrame
menu.task.isframe
task.isframe
task.resolution.isframe
)




INSTALL( 
  FILES
    ${INTERSHELL_FRAMES}
  DESTINATION
    ${DATA_INSTALL_PREFIX}/frames
)




