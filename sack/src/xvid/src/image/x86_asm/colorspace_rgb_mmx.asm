;/*****************************************************************************
; *
; *  XVID MPEG-4 VIDEO CODEC
; *  - RGB colorspace conversions -
; *
; *  Copyright(C) 2002-2003 Michael Militzer <isibaar@xvid.org>
; *               2002-2003 Peter Ross <pross@xvid.org>
; *
; *  This program is free software ; you can redistribute it and/or modify
; *  it under the terms of the GNU General Public License as published by
; *  the Free Software Foundation ; either version 2 of the License, or
; *  (at your option) any later version.
; *
; *  This program is distributed in the hope that it will be useful,
; *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
; *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; *  GNU General Public License for more details.
; *
; *  You should have received a copy of the GNU General Public License
; *  along with this program ; if not, write to the Free Software
; *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
; *
; ****************************************************************************/

BITS 32

%macro cglobal 1
	%ifdef PREFIX
		global _%1
		%define %1 _%1
	%else
		global %1
	%endif
%endmacro

;=============================================================================
; Some constants
;=============================================================================

;-----------------------------------------------------------------------------
; RGB->YV12 yuv constants
;-----------------------------------------------------------------------------

%define Y_R		0.257
%define Y_G		0.504
%define Y_B		0.098
%define Y_ADD	16

%define U_R		0.148
%define U_G		0.291
%define U_B		0.439
%define U_ADD	128

%define V_R		0.439
%define V_G		0.368
%define V_B		0.071
%define V_ADD	128

; Scaling used during conversion
%define SCALEBITS 6

;=============================================================================
; Read only data
;=============================================================================

%ifdef FORMAT_COFF
SECTION .rodata data
%else
SECTION .rodata data align=16
%endif
ALIGN 16

;-----------------------------------------------------------------------------
; RGB->YV12 multiplication matrices
;-----------------------------------------------------------------------------
;         FIX(Y_B)	FIX(Y_G)	FIX(Y_R) Ignored

y_mul: dw    25,      129,        66,      0
u_mul: dw   112,      -74,       -38,      0
v_mul: dw   -18,      -94,       112,      0


;-----------------------------------------------------------------------------
; YV12->RGB data
;-----------------------------------------------------------------------------

Y_SUB: dw  16,  16,  16,  16
U_SUB: dw 128, 128, 128, 128
V_SUB: dw 128, 128, 128, 128

Y_MUL: dw  74,  74,  74,  74

UG_MUL: dw  25,  25,  25,  25
VG_MUL: dw  52,  52,  52,  52

UB_MUL: dw 129, 129, 129, 129
VR_MUL: dw 102, 102, 102, 102

BRIGHT: db 128, 128, 128, 128, 128, 128, 128, 128

;=============================================================================
; Helper macros used with the colorspace_mmx.inc file
;=============================================================================

;------------------------------------------------------------------------------
; BGR_TO_YV12( BYTES )
;
; BYTES		3=bgr(24bit), 4=bgra(32-bit)
;
; bytes=3/4, pixels = 2, vpixels=2
;------------------------------------------------------------------------------

%macro BGR_TO_YV12_INIT		2
  movq mm7, [y_mul]
%endmacro


%macro BGR_TO_YV12			2
    ; y_out
  pxor mm4, mm4
  pxor mm5, mm5
  movd mm0, [edi]               ; x_ptr[0...]
  movd mm2, [edi+edx]           ; x_ptr[x_stride...]
  punpcklbw mm0, mm4            ; [  |b |g |r ]
  punpcklbw mm2, mm5            ; [  |b |g |r ]
  movq mm6, mm0                 ; = [  |b4|g4|r4]
  paddw mm6, mm2                ; +[  |b4|g4|r4]
  pmaddwd mm0, mm7              ; *= Y_MUL
  pmaddwd mm2, mm7              ; *= Y_MUL
  movq mm4, mm0                 ; [r]
  movq mm5, mm2                 ; [r]
  psrlq mm4, 32                 ; +[g]
  psrlq mm5, 32                 ; +[g]
  paddd mm0, mm4                ; +[b]
  paddd mm2, mm5                ; +[b]

  pxor mm4, mm4
  pxor mm5, mm5
  movd mm1, [edi+%1]            ; src[%1...]
  movd mm3, [edi+edx+%1]        ; src[x_stride+%1...]
  punpcklbw mm1, mm4            ; [  |b |g |r ]
  punpcklbw mm3, mm5            ; [  |b |g |r ]
  paddw mm6, mm1                ; +[  |b4|g4|r4]
  paddw mm6, mm3                ; +[  |b4|g4|r4]
  pmaddwd mm1, mm7              ; *= Y_MUL
  pmaddwd mm3, mm7              ; *= Y_MUL
  movq mm4, mm1                 ; [r]
  movq mm5, mm3                 ; [r]
  psrlq mm4, 32                 ; +[g]
  psrlq mm5, 32                 ; +[g]
  paddd mm1, mm4                ; +[b]
  paddd mm3, mm5                ; +[b]

  push edx

  movd edx, mm0
  shr edx, 8
  add edx, Y_ADD
  mov [esi], dl                 ; y_ptr[0]

  movd edx, mm1
  shr edx, 8
  add edx, Y_ADD
  mov [esi + 1], dl             ; y_ptr[1]

  movd edx, mm2
  shr edx, 8
  add edx, Y_ADD
  mov [esi + eax + 0], dl       ; y_ptr[y_stride + 0]

  movd edx, mm3
  shr edx, 8
  add edx, Y_ADD
  mov [esi + eax + 1], dl       ; y_ptr[y_stride + 1]

  ; u_ptr, v_ptr
  movq mm0, mm6                 ; = [  |b4|g4|r4]
  pmaddwd mm6, [v_mul]          ; *= V_MUL
  pmaddwd mm0, [u_mul]          ; *= U_MUL
  movq mm1, mm0
  movq mm2, mm6
  psrlq mm1, 32
  psrlq mm2, 32
  paddd mm0, mm1
  paddd mm2, mm6

  movd edx, mm0
  shr edx, 10
  add edx, U_ADD
  mov [ebx], dl

  movd edx, mm2
  shr edx, 10
  add edx, V_ADD
  mov [ecx], dl

  pop edx
%endmacro

;------------------------------------------------------------------------------
; YV12_TO_BGR( BYTES )
;
; BYTES		3=bgr(24-bit), 4=bgra(32-bit)
;
; bytes=3/4, pixels = 8, vpixels=2
;------------------------------------------------------------------------------

%macro YV12_TO_BGR_INIT		2
  pxor mm7, mm7			; clear mm7
%endmacro

%macro YV12_TO_BGR			2
%define TEMP_Y1  esp
%define TEMP_Y2  esp + 8
%define TEMP_G1  esp + 16
%define TEMP_G2  esp + 24
%define TEMP_B1  esp + 32
%define TEMP_B2  esp + 40

  movd mm2, [ebx]           ; u_ptr[0]
  movd mm3, [ecx]           ; v_ptr[0]
  punpcklbw mm2, mm7        ; u3u2u1u0 -> mm2
  punpcklbw mm3, mm7        ; v3v2v1v0 -> mm3
  psubsw mm2, [U_SUB]       ; U - 128
  psubsw mm3, [V_SUB]       ; V - 128
  movq mm4, mm2
  movq mm5, mm3
  pmullw mm2, [UG_MUL]
  pmullw mm3, [VG_MUL]
  movq mm6, mm2             ; u3u2u1u0 -> mm6
  punpckhwd mm2, mm2        ; u3u3u2u2 -> mm2
  punpcklwd mm6, mm6        ; u1u1u0u0 -> mm6
  pmullw mm4, [UB_MUL]      ; B_ADD -> mm4
  movq mm0, mm3
  punpckhwd mm3, mm3        ; v3v3v2v2 -> mm2
  punpcklwd mm0, mm0        ; v1v1v0v0 -> mm6
  paddsw mm2, mm3
  paddsw mm6, mm0
  pmullw mm5, [VR_MUL]      ; R_ADD -> mm5
  movq mm0, [esi]           ; y7y6y5y4y3y2y1y0 -> mm0
  movq mm1, mm0
  punpckhbw mm1, mm7        ; y7y6y5y4 -> mm1
  punpcklbw mm0, mm7        ; y3y2y1y0 -> mm0
  psubsw mm0, [Y_SUB]       ; Y - Y_SUB
  psubsw mm1, [Y_SUB]       ; Y - Y_SUB
  pmullw mm1, [Y_MUL]
  pmullw mm0, [Y_MUL]
  movq [TEMP_Y2], mm1       ; y7y6y5y4 -> mm3
  movq [TEMP_Y1], mm0       ; y3y2y1y0 -> mm7
  psubsw mm1, mm2           ; g7g6g5g4 -> mm1
  psubsw mm0, mm6           ; g3g2g1g0 -> mm0
  psraw mm1, SCALEBITS
  psraw mm0, SCALEBITS
  packuswb mm0, mm1         ;g7g6g5g4g3g2g1g0 -> mm0
  movq [TEMP_G1], mm0
  movq mm0, [esi+eax]       ; y7y6y5y4y3y2y1y0 -> mm0
  movq mm1, mm0
  punpckhbw mm1, mm7        ; y7y6y5y4 -> mm1
  punpcklbw mm0, mm7        ; y3y2y1y0 -> mm0
  psubsw mm0, [Y_SUB]       ; Y - Y_SUB
  psubsw mm1, [Y_SUB]       ; Y - Y_SUB
  pmullw mm1, [Y_MUL]
  pmullw mm0, [Y_MUL]
  movq mm3, mm1
  psubsw mm1, mm2           ; g7g6g5g4 -> mm1
  movq mm2, mm0
  psubsw mm0, mm6           ; g3g2g1g0 -> mm0
  psraw mm1, SCALEBITS
  psraw mm0, SCALEBITS
  packuswb mm0, mm1         ; g7g6g5g4g3g2g1g0 -> mm0
  movq [TEMP_G2], mm0
  movq mm0, mm4
  punpckhwd mm4, mm4        ; u3u3u2u2 -> mm2
  punpcklwd mm0, mm0        ; u1u1u0u0 -> mm6
  movq mm1, mm3             ; y7y6y5y4 -> mm1
  paddsw mm3, mm4           ; b7b6b5b4 -> mm3
  movq mm7, mm2             ; y3y2y1y0 -> mm7
  paddsw mm2, mm0           ; b3b2b1b0 -> mm2
  psraw mm3, SCALEBITS
  psraw mm2, SCALEBITS
  packuswb mm2, mm3         ; b7b6b5b4b3b2b1b0 -> mm2
  movq [TEMP_B2], mm2
  movq mm3, [TEMP_Y2]
  movq mm2, [TEMP_Y1]
  movq mm6, mm3             ; TEMP_Y2 -> mm6
  paddsw mm3, mm4           ; b7b6b5b4 -> mm3
  movq mm4, mm2             ; TEMP_Y1 -> mm4
  paddsw mm2, mm0           ; b3b2b1b0 -> mm2
  psraw mm3, SCALEBITS
  psraw mm2, SCALEBITS
  packuswb mm2, mm3         ; b7b6b5b4b3b2b1b0 -> mm2
  movq [TEMP_B1], mm2
  movq mm0, mm5
  punpckhwd mm5, mm5        ; v3v3v2v2 -> mm5
  punpcklwd mm0, mm0        ; v1v1v0v0 -> mm0
  paddsw mm1, mm5           ; r7r6r5r4 -> mm1
  paddsw mm7, mm0           ; r3r2r1r0 -> mm7
  psraw mm1, SCALEBITS
  psraw mm7, SCALEBITS
  packuswb mm7, mm1         ; r7r6r5r4r3r2r1r0 -> mm7 (TEMP_R2)
  paddsw mm6, mm5           ; r7r6r5r4 -> mm6
  paddsw mm4, mm0           ; r3r2r1r0 -> mm4
  psraw mm6, SCALEBITS
  psraw mm4, SCALEBITS
  packuswb mm4, mm6         ; r7r6r5r4r3r2r1r0 -> mm4 (TEMP_R1)
  movq mm0, [TEMP_B1]
  movq mm1, [TEMP_G1]
  movq mm6, mm7
  movq mm2, mm0
  punpcklbw mm2, mm4        ; r3b3r2b2r1b1r0b0 -> mm2
  punpckhbw mm0, mm4        ; r7b7r6b6r5b5r4b4 -> mm0
  pxor mm7, mm7
  movq mm3, mm1
  punpcklbw mm1, mm7        ; 0g30g20g10g0 -> mm1
  punpckhbw mm3, mm7        ; 0g70g60g50g4 -> mm3
  movq mm4, mm2
  punpcklbw mm2, mm1        ; 0r1g1b10r0g0b0 -> mm2
  punpckhbw mm4, mm1        ; 0r3g3b30r2g2b2 -> mm4
  movq mm5, mm0
  punpcklbw mm0, mm3        ; 0r5g5b50r4g4b4 -> mm0
  punpckhbw mm5, mm3        ; 0r7g7b70r6g6b6 -> mm5
%if %1 == 3     ; BGR (24-bit)
  movd [edi], mm2
  psrlq mm2, 32
  movd [edi + 3], mm2
  movd [edi + 6], mm4
  psrlq mm4, 32
  movd [edi + 9], mm4
  movd [edi + 12], mm0
  psrlq mm0, 32
  movd [edi + 15], mm0
  movq mm2, mm5
  psrlq mm0, 8              ; 000000r5g5 -> mm0
  psllq mm2, 32             ; 0r6g6b60000 -> mm2
  psrlq mm5, 32             ; 00000r7g7b7 -> mm5
  psrlq mm2, 16             ; 000r6g6b600 -> mm2
  por mm0, mm2              ; 000r6g6b6r5g5 -> mm0
  psllq mm5, 40             ; r7g7b700000 -> mm5
  por mm5, mm0              ; r7g7b7r6g6b6r5g5 -> mm5
  movq [edi + 16], mm5
  movq mm0, [TEMP_B2]
  movq mm1, [TEMP_G2]
  movq mm2, mm0
  punpcklbw mm2, mm6        ; r3b3r2b2r1b1r0b0 -> mm2
  punpckhbw mm0, mm6        ; r7b7r6b6r5b5r4b4 -> mm0
  movq mm3, mm1
  punpcklbw mm1, mm7        ; 0g30g20g10g0 -> mm1
  punpckhbw mm3, mm7        ; 0g70g60g50g4 -> mm3
  movq mm4, mm2
  punpcklbw mm2, mm1        ; 0r1g1b10r0g0b0 -> mm2
  punpckhbw mm4, mm1        ; 0r3g3b30r2g2b2 -> mm4
  movq mm5, mm0
  punpcklbw mm0, mm3        ; 0r5g5b50r4g4b4 -> mm0
  punpckhbw mm5, mm3        ; 0r7g7b70r6g6b6 -> mm5
  movd [edi+edx], mm2
  psrlq mm2, 32
  movd [edi+edx + 3], mm2
  movd [edi+edx + 6], mm4
  psrlq mm4, 32
  movd [edi+edx + 9], mm4
  movd [edi+edx + 12], mm0
  psrlq mm0, 32
  movd [edi+edx + 15], mm0
  movq mm2, mm5
  psrlq mm0, 8              ; 000000r5g5 -> mm0
  psllq mm2, 32             ; 0r6g6b60000 -> mm2
  psrlq mm5, 32             ; 00000r7g7b7 -> mm5
  psrlq mm2, 16             ; 000r6g6b600 -> mm2
  por mm0, mm2              ; 000r6g6b6r5g5 -> mm0
  psllq mm5, 40             ; r7g7b700000 -> mm5
  por mm5, mm0              ; r7g7b7r6g6b6r5g5 -> mm5
  movq [edi + edx + 16], mm5

%else       ; BGRA (32-bit)
  movq [edi], mm2
  movq [edi + 8], mm4
  movq [edi + 16], mm0
  movq [edi + 24], mm5
  movq mm0, [TEMP_B2]
  movq mm1, [TEMP_G2]
  movq mm2, mm0
  punpcklbw mm2, mm6        ; r3b3r2b2r1b1r0b0 -> mm2
  punpckhbw mm0, mm6        ; r7b7r6b6r5b5r4b4 -> mm0
  movq mm3, mm1
  punpcklbw mm1, mm7        ; 0g30g20g10g0 -> mm1
  punpckhbw mm3, mm7        ; 0g70g60g50g4 -> mm3
  movq mm4, mm2
  punpcklbw mm2, mm1        ; 0r1g1b10r0g0b0 -> mm2
  punpckhbw mm4, mm1        ; 0r3g3b30r2g2b2 -> mm4
  movq mm5, mm0
  punpcklbw mm0, mm3        ; 0r5g5b50r4g4b4 -> mm0
  punpckhbw mm5, mm3        ; 0r7g7b70r6g6b6 -> mm5
  movq [edi + edx], mm2
  movq [edi + edx + 8], mm4
  movq [edi + edx + 16], mm0
  movq [edi + edx + 24], mm5
%endif

%undef TEMP_Y1
%undef TEMP_Y2
%undef TEMP_G1
%undef TEMP_G2
%undef TEMP_B1
%undef TEMP_B2
%endmacro

;=============================================================================
; Code
;=============================================================================

SECTION .text

%include "colorspace_mmx.inc"

; input
MAKE_COLORSPACE  bgr_to_yv12_mmx,0,    3,2,2,  BGR_TO_YV12,  3, -1
MAKE_COLORSPACE  bgra_to_yv12_mmx,0,   4,2,2,  BGR_TO_YV12,  4, -1

; output
MAKE_COLORSPACE  yv12_to_bgr_mmx,48,   3,8,2,  YV12_TO_BGR,  3, -1
MAKE_COLORSPACE  yv12_to_bgra_mmx,48,  4,8,2,  YV12_TO_BGR,  4, -1

