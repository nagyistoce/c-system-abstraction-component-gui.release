
if( NOT __NO_GUI__ )

if( NEED_JPEG )
SET( JBASEDIR src/jpeg-8b )
SET( SYSDEPMEM jmemnobs.c )

# library object files common to compression and decompression
SET( COMSRCS  jaricom.c jcomapi.c jutils.c jerror.c jmemmgr.c ${SYSDEPMEM} )

# compression library object files
SET( CLIBSRCS jcarith.c jcapimin.c jcapistd.c jctrans.c jcparam.c jdatadst.c 
        jcinit.c jcmaster.c jcmarker.c jcmainct jcprepct.c 
        jccoefct.c jccolor.c jcsample.c jchuff.c 
        jcdctmgr.c jfdctfst.c jfdctflt.c jfdctint.c )

# decompression library object files
SET( DLIBSRCS jdarith.c jdapimin.c jdapistd.c jdtrans.c jdatasrc.c 
        jdmaster.c jdinput.c jdmarker.c jdhuff.c 
        jdmainct.c jdcoefct.c jdpostct.c jddctmgr.c jidctfst.c 
        jidctflt.c jidctint.c jdsample.c jdcolor.c 
        jquant1.c jquant2.c jdmerge.c )
# These objectfiles are included in libjpeg.lib
FOREACH( SRC ${CLIBSRCS} ${COMSRCS} ${DLIBSRCS} )
  LIST( APPEND JPEG_SOURCE ${JBASEDIR}/${SRC} )
ENDFOREACH( SRC )
add_definitions( -DJPEG_SOURCE )
# ya, this is sorta redundant... should fix that someday.
include_directories( ${SACK_BASE}/${JBASEDIR} )

#message( adding ${SACK_BASE}/${JPEG_SOURCE} )

source_group("Source Files\\Jpeg-8b Library" FILES ${JPEG_SOURCE})


endif( NEED_JPEG )


if( NEED_PNG )

 if( NEED_ZLIB )
  SET( ZBASEDIR src/zlib-1.2.5 )
  include( ${ZBASEDIR}/CMakeLists.part )
source_group("Source Files\\zlib-1.2.5 Library" FILES ${ZLIB_SOURCE})
 endif( NEED_ZLIB )


 SET( PBASEDIR src/libpng-1.4.3 )
 include( ${PBASEDIR}/CMakeLists.part )

source_group("Source Files\\libpng-1.4.3 Library" FILES ${PNG_SOURCE})
endif( NEED_PNG )


if( NEED_FREETYPE )
SET( FBASEDIR src/freetype-2.4.1/src )

add_definitions( -DFREETYPE_SOURCE -DFT2_BUILD_LIBRARY )

SET( FT_SRCS autofit/autofit.c 
     base/ftbase.c 
     bdf/bdf.c 
     cache/ftcache.c 
     cff/cff.c 
     cid/type1cid.c 
     lzw/ftlzw.c 
     gzip/ftgzip.c 
     otvalid/otvalid.c 
     pcf/pcf.c 
     pfr/pfr.c 
     psnames/psmodule.c 
     psaux/psaux.c 
     pshinter/pshinter.c 
     raster/raster.c 
     sfnt/sfnt.c 
     smooth/smooth.c 
     truetype/truetype.c 
     type1/type1.c 
     type42/type42.c 
     winfonts/winfnt.c 
     base/ftbitmap.c 
     base/ftgasp.c 
     base/ftglyph.c 
     base/ftgxval.c 
     base/ftinit.c 
     base/ftmm.c 
     base/ftotval.c 
     base/ftpfr.c 
     base/ftstroke.c 
     base/ftsynth.c 
     base/ftsystem.c 
     base/fttype1.c 
     base/ftwinfnt.c 
     base/ftxf86.c  )

include_directories( ${FBASEDIR}/../include )
FOREACH( SRC ${FT_SRCS} )
  LIST( APPEND FREETYPE_SOURCE ${FBASEDIR}/${SRC} )
ENDFOREACH()
endif()

source_group("Source Files\\Freetype-2.4.1 Library" FILES ${FREETYPE_SOURCE})

endif( NOT __NO_GUI__ )

