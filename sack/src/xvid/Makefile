
LIBNAME=xvidcore
DEFNAME=src/xvidcore.def
SRCS=src/bitstream/bitstream \
     src/bitstream/cbp \
     src/bitstream/mbcoding \
     src/bitstream/x86_asm/cbp_3dne \
       src/bitstream/x86_asm/cbp_mmx \
         src/bitstream/x86_asm/cbp_sse2 \
     src/xvid src/decoder src/encoder \
     src/dct/fdct src/dct/idct src/dct/simple_idct \
     src/dct/x86_asm/fdct_mmx_ffmpeg  \
     src/dct/x86_asm/fdct_mmx_skal    \
     src/dct/x86_asm/fdct_sse2_skal   \
     src/dct/x86_asm/idct_3dne        \
     src/dct/x86_asm/idct_mmx         \
     src/dct/x86_asm/idct_sse2_dmitry \
     src/dct/x86_asm/simple_idct_mmx  \
     src/image/colorspace src/image/font src/image/image \
     src/image/interpolate8x8 src/image/postprocessing \
     src/image/qpel src/image/reduced \
     src/image/x86_asm/colorspace_rgb_mmx    \
     src/image/x86_asm/colorspace_yuv_mmx    \
     src/image/x86_asm/colorspace_yuyv_mmx   \
     src/image/x86_asm/interpolate8x8_3dn \
     src/image/x86_asm/interpolate8x8_3dne  \
     src/image/x86_asm/interpolate8x8_mmx   \
     src/image/x86_asm/interpolate8x8_xmm   \
     src/image/x86_asm/qpel_mmx             \
     src/image/x86_asm/reduced_mmx          \
     src/motion/estimation_bvop \
     src/motion/estimation_common \
     src/motion/estimation_gmc \
     src/motion/estimation_pvop \
     src/motion/estimation_rd_based \
     src/motion/gmc \
     src/motion/motion_comp \
     src/motion/sad            \
     src/motion/x86_asm/sad_3dn \
     src/motion/x86_asm/sad_3dne \
     src/motion/x86_asm/sad_mmx \
     src/motion/x86_asm/sad_sse2 \
     src/motion/x86_asm/sad_xmm \
     src/motion/vop_type_decision      \
     src/quant/x86_asm/quantize_h263_3dne \
     src/quant/x86_asm/quantize_h263_mmx \
     src/quant/x86_asm/quantize_mpeg_mmx \
     src/quant/x86_asm/quantize_mpeg_xmm \
     src/quant/quant_h263 \
     src/quant/quant_matrix \
     src/quant/quant_mpeg \
     src/utils/emms \
     src/utils/mbtransquant \
     src/utils/mem_align \
     src/utils/mem_transfer \
     src/utils/timer \
     src/utils/x86_asm/cpuid \
     src/utils/x86_asm/interlacing_mmx \
     src/utils/x86_asm/mem_transfer_3dne \
     src/utils/x86_asm/mem_transfer_mmx  \
     src/prediction/mbprediction               	
CXFLAGS= -DXVID_SOURCE -DARCH_IS_32BIT -DARCH_IS_GENERIC -DARCH_IS_IA32
ifdef __WINDOWS__
AFLAGS=-DPREFIX
endif
#PROJECTS=src


include $(SACK_BASE)/makefiles/makefile.single
