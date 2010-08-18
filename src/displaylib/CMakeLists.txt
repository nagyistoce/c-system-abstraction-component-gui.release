
cmake_minimum_required(VERSION 2.8)

PROJECT( bag.display.plugin )

  include_directories( /home/d3x0r/build/SDL/include )
LINK_DIRECTORIES( /home/d3x0r/build/SDL/lib )


set( SOURCES 
${FIRST_GCC_LIBRARY_SOURCE} 
display
display_image
display_video
keybrd
calibrate
#fb_sd_lmouse
${LAST_GCC_LIBRARY_SOURCE}

)

ADD_LIBRARY( ${PROJECT_NAME} SHARED ${SOURCES} )
target_link_libraries( ${PROJECT_NAME} ${SACK_LIBRARIES} SDL )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
PREFIX ""
SUFFIX ""
COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} -DDISPLAY_SOURCE -DRENDER_LIBRARY_SOURCE -DIMAGE_LIBRARY_SOURCE -DBAG"
)

INSTALL_DEFAULT_DEST_BINARY( ${PROJECT_NAME} )


#APP_DEFAULT_DATA1=Display.Config fingerdimple.png ELO.Config
