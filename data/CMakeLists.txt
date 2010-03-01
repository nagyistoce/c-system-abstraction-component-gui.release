
SET( DEFAULT_FONTS
Crysta.ttf
arial.ttf
arialbd.ttf
comic.ttf
comicbd.ttf
cour.ttf
courbd.ttf
courbi.ttf
couri.ttf
lucon.ttf
)
SET( BASE_IMAGES
frame_border.png
glare.png
)



INSTALL( 
  FILES
    ${DEFAULT_FONTS}
  DESTINATION
    ${DATA_INSTALL_PREFIX}/fonts
)


INSTALL( 
  FILES
    ${BASE_IMAGES}
  DESTINATION
    ${DATA_INSTALL_PREFIX}/images
)


