
if( NOT __NO_GUI__ )

if( NEED_JPEG )
SET( JBASEDIR src/jpeg-8b )
SET( SYSDEPMEM jmemnobs )

# library object files common to compression and decompression
SET( COMSRCS  jaricom jcomapi jutils jerror jmemmgr ${SYSDEPMEM} )

# compression library object files
SET( CLIBSRCS jcarith jcapimin jcapistd jctrans jcparam jdatadst 
        jcinit jcmaster jcmarker jcmainct jcprepct 
        jccoefct jccolor jcsample jchuff 
        jcdctmgr jfdctfst jfdctflt jfdctint )

# decompression library object files
SET( DLIBSRCS jdarith jdapimin jdapistd jdtrans jdatasrc 
        jdmaster jdinput jdmarker jdhuff 
        jdmainct jdcoefct jdpostct jddctmgr jidctfst 
        jidctflt jidctint jdsample jdcolor 
        jquant1 jquant2 jdmerge )
# These objectfiles are included in libjpeg.lib
FOREACH( SRC ${CLIBSRCS} ${COMSRCS} ${DLIBSRCS} )
  LIST( APPEND JPEG_SOURCE ${JBASEDIR}/${SRC} )
ENDFOREACH( SRC )
add_definitions( -DJPEG_SOURCE )
# ya, this is sorta redundant... should fix that someday.
include_directories( ${SACK_BASE}/${JBASEDIR} )

#message( adding ${SACK_BASE}/${JPEG_SOURCE} )
endif()


if( NEED_PNG )

 if( NEED_ZLIB )
  SET( ZBASEDIR src/zlib-1.2.5 )
  include( ${ZBASEDIR}/CMakeLists.part )
 endif( NEED_ZLIB )


 SET( PBASEDIR src/libpng-1.4.3 )
 include( ${PBASEDIR}/CMakeLists.part )
endif( NEED_PNG )


if( NEED_FREETYPE )
SET( FBASEDIR src/freetype-2.4.1/src )

add_definitions( -DFREETYPE_SOURCE -DFT2_BUILD_LIBRARY )

SET( FT_SRCS autofit/autofit 
     base/ftbase 
     bdf/bdf 
     cache/ftcache 
     cff/cff 
     cid/type1cid 
     lzw/ftlzw 
     gzip/ftgzip 
     otvalid/otvalid 
     pcf/pcf 
     pfr/pfr 
     psnames/psmodule 
     psaux/psaux 
     pshinter/pshinter 
     raster/raster 
     sfnt/sfnt 
     smooth/smooth 
     truetype/truetype 
     type1/type1 
     type42/type42 
     winfonts/winfnt 
     base/ftbitmap 
     base/ftgasp 
     base/ftglyph 
     base/ftgxval 
     base/ftinit 
     base/ftmm 
     base/ftotval 
     base/ftpfr 
     base/ftstroke 
     base/ftsynth 
     base/ftsystem 
     base/fttype1 
     base/ftwinfnt 
     base/ftxf86  )

include_directories( ${FBASEDIR}/../include )
FOREACH( SRC ${FT_SRCS} )
  LIST( APPEND FREETYPE_SOURCE ${FBASEDIR}/${SRC} )
ENDFOREACH()
endif()

endif( NOT __NO_GUI__ )

