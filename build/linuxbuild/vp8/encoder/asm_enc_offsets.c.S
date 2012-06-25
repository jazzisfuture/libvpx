	.file	"asm_enc_offsets.c"
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.text
.Ltext0:
	.type	vpx_internal_error, @function
vpx_internal_error:
.LFB0:
	.file 1 "../../vpx/internal/vpx_codec_internal.h"
	.loc 1 438 0
	.cfi_startproc
	pushq	%rbp
.LCFI0:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI1:
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$248, %rsp
	movq	%rdi, -232(%rbp)
	movl	%esi, -236(%rbp)
	movq	%rcx, -168(%rbp)
	movq	%r8, -160(%rbp)
	movq	%r9, -152(%rbp)
	movzbl	%al, %eax
	leaq	0(,%rax,4), %rcx
	movl	$.L2, %eax
	subq	%rcx, %rax
	leaq	-17(%rbp), %rcx
	jmp	*%rax
	movaps	%xmm7, -15(%rcx)
	movaps	%xmm6, -31(%rcx)
	movaps	%xmm5, -47(%rcx)
	movaps	%xmm4, -63(%rcx)
	movaps	%xmm3, -79(%rcx)
	movaps	%xmm2, -95(%rcx)
	movaps	%xmm1, -111(%rcx)
	movaps	%xmm0, -127(%rcx)
.L2:
	movq	%rdx, -248(%rbp)
	.loc 1 441 0
	movq	-232(%rbp), %rax
	movl	-236(%rbp), %edx
	movl	%edx, (%rax)
	.loc 1 442 0
	movq	-232(%rbp), %rax
	movl	$0, 4(%rax)
	.loc 1 444 0
	cmpq	$0, -248(%rbp)
	je	.L3
	.cfi_offset 3, -24
.LBB2:
	.loc 1 445 0
	movq	$80, -200(%rbp)
	.loc 1 447 0
	movq	-232(%rbp), %rax
	movl	$1, 4(%rax)
	.loc 1 448 0
	leaq	-224(%rbp), %rax
	movl	$24, (%rax)
	leaq	-224(%rbp), %rax
	movl	$48, 4(%rax)
	leaq	-224(%rbp), %rax
	leaq	16(%rbp), %rdx
	movq	%rdx, 8(%rax)
	leaq	-224(%rbp), %rax
	leaq	-192(%rbp), %rdx
	movq	%rdx, 16(%rax)
	.loc 1 449 0
	movq	-200(%rbp), %rax
	leaq	-1(%rax), %rsi
	movq	-232(%rbp), %rax
	leaq	8(%rax), %rbx
	leaq	-224(%rbp), %rdx
	movq	-248(%rbp), %rax
	movq	%rdx, %rcx
	movq	%rax, %rdx
	movq	%rbx, %rdi
	call	vsnprintf
	.loc 1 451 0
	movq	-200(%rbp), %rax
	leaq	-1(%rax), %rdx
	movq	-232(%rbp), %rax
	movb	$0, 8(%rax,%rdx)
.L3:
.LBE2:
	.loc 1 454 0
	movq	-232(%rbp), %rax
	movl	88(%rax), %eax
	testl	%eax, %eax
	je	.L5
	.loc 1 455 0
	movq	-232(%rbp), %rax
	movl	(%rax), %eax
	movq	-232(%rbp), %rdx
	addq	$96, %rdx
	movl	%eax, %esi
	movq	%rdx, %rdi
	call	longjmp
.L5:
	.loc 1 456 0
	addq	$248, %rsp
	popq	%rbx
	leave
	ret
	.cfi_endproc
.LFE0:
	.size	vpx_internal_error, .-vpx_internal_error
	.type	VP8_SET_REFERENCE__value, @function
VP8_SET_REFERENCE__value:
.LFB1:
	.file 2 "../../vpx/vp8.h"
	.loc 2 105 0
	.cfi_startproc
	pushq	%rbp
.LCFI2:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI3:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 105 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L7
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L8
.L7:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L8:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE1:
	.size	VP8_SET_REFERENCE__value, .-VP8_SET_REFERENCE__value
	.type	VP8_SET_REFERENCE__convert, @function
VP8_SET_REFERENCE__convert:
.LFB2:
	.loc 2 105 0
	.cfi_startproc
	pushq	%rbp
.LCFI4:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI5:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 105 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE2:
	.size	VP8_SET_REFERENCE__convert, .-VP8_SET_REFERENCE__convert
	.type	VP8_COPY_REFERENCE__value, @function
VP8_COPY_REFERENCE__value:
.LFB3:
	.loc 2 106 0
	.cfi_startproc
	pushq	%rbp
.LCFI6:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI7:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 106 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L13
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L14
.L13:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L14:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE3:
	.size	VP8_COPY_REFERENCE__value, .-VP8_COPY_REFERENCE__value
	.type	VP8_COPY_REFERENCE__convert, @function
VP8_COPY_REFERENCE__convert:
.LFB4:
	.loc 2 106 0
	.cfi_startproc
	pushq	%rbp
.LCFI8:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI9:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 106 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE4:
	.size	VP8_COPY_REFERENCE__convert, .-VP8_COPY_REFERENCE__convert
	.type	VP8_SET_POSTPROC__value, @function
VP8_SET_POSTPROC__value:
.LFB5:
	.loc 2 107 0
	.cfi_startproc
	pushq	%rbp
.LCFI10:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI11:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 107 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L19
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L20
.L19:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L20:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE5:
	.size	VP8_SET_POSTPROC__value, .-VP8_SET_POSTPROC__value
	.type	VP8_SET_POSTPROC__convert, @function
VP8_SET_POSTPROC__convert:
.LFB6:
	.loc 2 107 0
	.cfi_startproc
	pushq	%rbp
.LCFI12:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI13:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 107 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE6:
	.size	VP8_SET_POSTPROC__convert, .-VP8_SET_POSTPROC__convert
	.type	VP8_SET_DBG_COLOR_REF_FRAME__value, @function
VP8_SET_DBG_COLOR_REF_FRAME__value:
.LFB7:
	.loc 2 108 0
	.cfi_startproc
	pushq	%rbp
.LCFI14:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI15:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 108 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L25
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L26
.L25:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L26:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE7:
	.size	VP8_SET_DBG_COLOR_REF_FRAME__value, .-VP8_SET_DBG_COLOR_REF_FRAME__value
	.type	VP8_SET_DBG_COLOR_REF_FRAME__convert, @function
VP8_SET_DBG_COLOR_REF_FRAME__convert:
.LFB8:
	.loc 2 108 0
	.cfi_startproc
	pushq	%rbp
.LCFI16:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI17:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 108 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE8:
	.size	VP8_SET_DBG_COLOR_REF_FRAME__convert, .-VP8_SET_DBG_COLOR_REF_FRAME__convert
	.type	VP8_SET_DBG_COLOR_MB_MODES__value, @function
VP8_SET_DBG_COLOR_MB_MODES__value:
.LFB9:
	.loc 2 109 0
	.cfi_startproc
	pushq	%rbp
.LCFI18:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI19:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 109 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L31
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L32
.L31:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L32:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE9:
	.size	VP8_SET_DBG_COLOR_MB_MODES__value, .-VP8_SET_DBG_COLOR_MB_MODES__value
	.type	VP8_SET_DBG_COLOR_MB_MODES__convert, @function
VP8_SET_DBG_COLOR_MB_MODES__convert:
.LFB10:
	.loc 2 109 0
	.cfi_startproc
	pushq	%rbp
.LCFI20:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI21:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 109 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE10:
	.size	VP8_SET_DBG_COLOR_MB_MODES__convert, .-VP8_SET_DBG_COLOR_MB_MODES__convert
	.type	VP8_SET_DBG_COLOR_B_MODES__value, @function
VP8_SET_DBG_COLOR_B_MODES__value:
.LFB11:
	.loc 2 110 0
	.cfi_startproc
	pushq	%rbp
.LCFI22:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI23:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 110 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L37
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L38
.L37:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L38:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE11:
	.size	VP8_SET_DBG_COLOR_B_MODES__value, .-VP8_SET_DBG_COLOR_B_MODES__value
	.type	VP8_SET_DBG_COLOR_B_MODES__convert, @function
VP8_SET_DBG_COLOR_B_MODES__convert:
.LFB12:
	.loc 2 110 0
	.cfi_startproc
	pushq	%rbp
.LCFI24:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI25:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 110 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE12:
	.size	VP8_SET_DBG_COLOR_B_MODES__convert, .-VP8_SET_DBG_COLOR_B_MODES__convert
	.type	VP8_SET_DBG_DISPLAY_MV__value, @function
VP8_SET_DBG_DISPLAY_MV__value:
.LFB13:
	.loc 2 111 0
	.cfi_startproc
	pushq	%rbp
.LCFI26:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI27:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 111 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L43
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L44
.L43:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L44:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE13:
	.size	VP8_SET_DBG_DISPLAY_MV__value, .-VP8_SET_DBG_DISPLAY_MV__value
	.type	VP8_SET_DBG_DISPLAY_MV__convert, @function
VP8_SET_DBG_DISPLAY_MV__convert:
.LFB14:
	.loc 2 111 0
	.cfi_startproc
	pushq	%rbp
.LCFI28:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI29:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 2 111 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE14:
	.size	VP8_SET_DBG_DISPLAY_MV__convert, .-VP8_SET_DBG_DISPLAY_MV__convert
	.type	VP8E_UPD_ENTROPY__value, @function
VP8E_UPD_ENTROPY__value:
.LFB15:
	.file 3 "../../vpx/vp8cx.h"
	.loc 3 290 0
	.cfi_startproc
	pushq	%rbp
.LCFI30:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI31:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 290 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L49
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L50
.L49:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L50:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE15:
	.size	VP8E_UPD_ENTROPY__value, .-VP8E_UPD_ENTROPY__value
	.type	VP8E_UPD_ENTROPY__convert, @function
VP8E_UPD_ENTROPY__convert:
.LFB16:
	.loc 3 290 0
	.cfi_startproc
	pushq	%rbp
.LCFI32:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI33:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 290 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE16:
	.size	VP8E_UPD_ENTROPY__convert, .-VP8E_UPD_ENTROPY__convert
	.type	VP8E_UPD_REFERENCE__value, @function
VP8E_UPD_REFERENCE__value:
.LFB17:
	.loc 3 291 0
	.cfi_startproc
	pushq	%rbp
.LCFI34:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI35:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 291 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L55
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L56
.L55:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L56:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE17:
	.size	VP8E_UPD_REFERENCE__value, .-VP8E_UPD_REFERENCE__value
	.type	VP8E_UPD_REFERENCE__convert, @function
VP8E_UPD_REFERENCE__convert:
.LFB18:
	.loc 3 291 0
	.cfi_startproc
	pushq	%rbp
.LCFI36:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI37:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 291 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE18:
	.size	VP8E_UPD_REFERENCE__convert, .-VP8E_UPD_REFERENCE__convert
	.type	VP8E_USE_REFERENCE__value, @function
VP8E_USE_REFERENCE__value:
.LFB19:
	.loc 3 292 0
	.cfi_startproc
	pushq	%rbp
.LCFI38:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI39:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 292 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L61
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L62
.L61:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L62:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE19:
	.size	VP8E_USE_REFERENCE__value, .-VP8E_USE_REFERENCE__value
	.type	VP8E_USE_REFERENCE__convert, @function
VP8E_USE_REFERENCE__convert:
.LFB20:
	.loc 3 292 0
	.cfi_startproc
	pushq	%rbp
.LCFI40:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI41:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 292 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE20:
	.size	VP8E_USE_REFERENCE__convert, .-VP8E_USE_REFERENCE__convert
	.type	VP8E_SET_ROI_MAP__value, @function
VP8E_SET_ROI_MAP__value:
.LFB21:
	.loc 3 294 0
	.cfi_startproc
	pushq	%rbp
.LCFI42:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI43:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 294 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L67
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L68
.L67:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L68:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE21:
	.size	VP8E_SET_ROI_MAP__value, .-VP8E_SET_ROI_MAP__value
	.type	VP8E_SET_ROI_MAP__convert, @function
VP8E_SET_ROI_MAP__convert:
.LFB22:
	.loc 3 294 0
	.cfi_startproc
	pushq	%rbp
.LCFI44:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI45:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 294 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE22:
	.size	VP8E_SET_ROI_MAP__convert, .-VP8E_SET_ROI_MAP__convert
	.type	VP8E_SET_ACTIVEMAP__value, @function
VP8E_SET_ACTIVEMAP__value:
.LFB23:
	.loc 3 295 0
	.cfi_startproc
	pushq	%rbp
.LCFI46:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI47:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 295 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L73
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L74
.L73:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L74:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE23:
	.size	VP8E_SET_ACTIVEMAP__value, .-VP8E_SET_ACTIVEMAP__value
	.type	VP8E_SET_ACTIVEMAP__convert, @function
VP8E_SET_ACTIVEMAP__convert:
.LFB24:
	.loc 3 295 0
	.cfi_startproc
	pushq	%rbp
.LCFI48:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI49:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 295 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE24:
	.size	VP8E_SET_ACTIVEMAP__convert, .-VP8E_SET_ACTIVEMAP__convert
	.type	VP8E_SET_SCALEMODE__value, @function
VP8E_SET_SCALEMODE__value:
.LFB25:
	.loc 3 296 0
	.cfi_startproc
	pushq	%rbp
.LCFI50:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI51:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 296 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L79
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L80
.L79:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L80:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE25:
	.size	VP8E_SET_SCALEMODE__value, .-VP8E_SET_SCALEMODE__value
	.type	VP8E_SET_SCALEMODE__convert, @function
VP8E_SET_SCALEMODE__convert:
.LFB26:
	.loc 3 296 0
	.cfi_startproc
	pushq	%rbp
.LCFI52:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI53:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 296 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE26:
	.size	VP8E_SET_SCALEMODE__convert, .-VP8E_SET_SCALEMODE__convert
	.type	VP8E_SET_CPUUSED__value, @function
VP8E_SET_CPUUSED__value:
.LFB27:
	.loc 3 298 0
	.cfi_startproc
	pushq	%rbp
.LCFI54:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI55:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 298 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L85
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L86
.L85:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L86:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE27:
	.size	VP8E_SET_CPUUSED__value, .-VP8E_SET_CPUUSED__value
	.type	VP8E_SET_CPUUSED__convert, @function
VP8E_SET_CPUUSED__convert:
.LFB28:
	.loc 3 298 0
	.cfi_startproc
	pushq	%rbp
.LCFI56:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI57:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 298 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE28:
	.size	VP8E_SET_CPUUSED__convert, .-VP8E_SET_CPUUSED__convert
	.type	VP8E_SET_ENABLEAUTOALTREF__value, @function
VP8E_SET_ENABLEAUTOALTREF__value:
.LFB29:
	.loc 3 299 0
	.cfi_startproc
	pushq	%rbp
.LCFI58:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI59:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 299 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L91
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L92
.L91:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L92:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE29:
	.size	VP8E_SET_ENABLEAUTOALTREF__value, .-VP8E_SET_ENABLEAUTOALTREF__value
	.type	VP8E_SET_ENABLEAUTOALTREF__convert, @function
VP8E_SET_ENABLEAUTOALTREF__convert:
.LFB30:
	.loc 3 299 0
	.cfi_startproc
	pushq	%rbp
.LCFI60:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI61:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 299 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE30:
	.size	VP8E_SET_ENABLEAUTOALTREF__convert, .-VP8E_SET_ENABLEAUTOALTREF__convert
	.type	VP8E_SET_NOISE_SENSITIVITY__value, @function
VP8E_SET_NOISE_SENSITIVITY__value:
.LFB31:
	.loc 3 300 0
	.cfi_startproc
	pushq	%rbp
.LCFI62:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI63:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 300 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L97
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L98
.L97:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L98:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE31:
	.size	VP8E_SET_NOISE_SENSITIVITY__value, .-VP8E_SET_NOISE_SENSITIVITY__value
	.type	VP8E_SET_NOISE_SENSITIVITY__convert, @function
VP8E_SET_NOISE_SENSITIVITY__convert:
.LFB32:
	.loc 3 300 0
	.cfi_startproc
	pushq	%rbp
.LCFI64:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI65:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 300 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE32:
	.size	VP8E_SET_NOISE_SENSITIVITY__convert, .-VP8E_SET_NOISE_SENSITIVITY__convert
	.type	VP8E_SET_SHARPNESS__value, @function
VP8E_SET_SHARPNESS__value:
.LFB33:
	.loc 3 301 0
	.cfi_startproc
	pushq	%rbp
.LCFI66:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI67:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 301 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L103
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L104
.L103:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L104:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE33:
	.size	VP8E_SET_SHARPNESS__value, .-VP8E_SET_SHARPNESS__value
	.type	VP8E_SET_SHARPNESS__convert, @function
VP8E_SET_SHARPNESS__convert:
.LFB34:
	.loc 3 301 0
	.cfi_startproc
	pushq	%rbp
.LCFI68:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI69:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 301 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE34:
	.size	VP8E_SET_SHARPNESS__convert, .-VP8E_SET_SHARPNESS__convert
	.type	VP8E_SET_STATIC_THRESHOLD__value, @function
VP8E_SET_STATIC_THRESHOLD__value:
.LFB35:
	.loc 3 302 0
	.cfi_startproc
	pushq	%rbp
.LCFI70:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI71:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 302 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L109
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L110
.L109:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L110:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE35:
	.size	VP8E_SET_STATIC_THRESHOLD__value, .-VP8E_SET_STATIC_THRESHOLD__value
	.type	VP8E_SET_STATIC_THRESHOLD__convert, @function
VP8E_SET_STATIC_THRESHOLD__convert:
.LFB36:
	.loc 3 302 0
	.cfi_startproc
	pushq	%rbp
.LCFI72:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI73:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 302 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE36:
	.size	VP8E_SET_STATIC_THRESHOLD__convert, .-VP8E_SET_STATIC_THRESHOLD__convert
	.type	VP8E_SET_TOKEN_PARTITIONS__value, @function
VP8E_SET_TOKEN_PARTITIONS__value:
.LFB37:
	.loc 3 303 0
	.cfi_startproc
	pushq	%rbp
.LCFI74:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI75:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 303 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L115
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L116
.L115:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L116:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE37:
	.size	VP8E_SET_TOKEN_PARTITIONS__value, .-VP8E_SET_TOKEN_PARTITIONS__value
	.type	VP8E_SET_TOKEN_PARTITIONS__convert, @function
VP8E_SET_TOKEN_PARTITIONS__convert:
.LFB38:
	.loc 3 303 0
	.cfi_startproc
	pushq	%rbp
.LCFI76:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI77:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 303 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE38:
	.size	VP8E_SET_TOKEN_PARTITIONS__convert, .-VP8E_SET_TOKEN_PARTITIONS__convert
	.type	VP8E_SET_ARNR_MAXFRAMES__value, @function
VP8E_SET_ARNR_MAXFRAMES__value:
.LFB39:
	.loc 3 305 0
	.cfi_startproc
	pushq	%rbp
.LCFI78:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI79:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 305 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L121
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L122
.L121:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L122:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE39:
	.size	VP8E_SET_ARNR_MAXFRAMES__value, .-VP8E_SET_ARNR_MAXFRAMES__value
	.type	VP8E_SET_ARNR_MAXFRAMES__convert, @function
VP8E_SET_ARNR_MAXFRAMES__convert:
.LFB40:
	.loc 3 305 0
	.cfi_startproc
	pushq	%rbp
.LCFI80:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI81:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 305 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE40:
	.size	VP8E_SET_ARNR_MAXFRAMES__convert, .-VP8E_SET_ARNR_MAXFRAMES__convert
	.type	VP8E_SET_ARNR_STRENGTH__value, @function
VP8E_SET_ARNR_STRENGTH__value:
.LFB41:
	.loc 3 306 0
	.cfi_startproc
	pushq	%rbp
.LCFI82:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI83:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 306 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L127
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L128
.L127:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L128:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE41:
	.size	VP8E_SET_ARNR_STRENGTH__value, .-VP8E_SET_ARNR_STRENGTH__value
	.type	VP8E_SET_ARNR_STRENGTH__convert, @function
VP8E_SET_ARNR_STRENGTH__convert:
.LFB42:
	.loc 3 306 0
	.cfi_startproc
	pushq	%rbp
.LCFI84:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI85:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 306 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE42:
	.size	VP8E_SET_ARNR_STRENGTH__convert, .-VP8E_SET_ARNR_STRENGTH__convert
	.type	VP8E_SET_ARNR_TYPE__value, @function
VP8E_SET_ARNR_TYPE__value:
.LFB43:
	.loc 3 307 0
	.cfi_startproc
	pushq	%rbp
.LCFI86:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI87:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 307 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L133
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L134
.L133:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L134:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE43:
	.size	VP8E_SET_ARNR_TYPE__value, .-VP8E_SET_ARNR_TYPE__value
	.type	VP8E_SET_ARNR_TYPE__convert, @function
VP8E_SET_ARNR_TYPE__convert:
.LFB44:
	.loc 3 307 0
	.cfi_startproc
	pushq	%rbp
.LCFI88:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI89:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 307 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE44:
	.size	VP8E_SET_ARNR_TYPE__convert, .-VP8E_SET_ARNR_TYPE__convert
	.type	VP8E_SET_TUNING__value, @function
VP8E_SET_TUNING__value:
.LFB45:
	.loc 3 308 0
	.cfi_startproc
	pushq	%rbp
.LCFI90:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI91:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 308 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L139
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L140
.L139:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L140:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE45:
	.size	VP8E_SET_TUNING__value, .-VP8E_SET_TUNING__value
	.type	VP8E_SET_TUNING__convert, @function
VP8E_SET_TUNING__convert:
.LFB46:
	.loc 3 308 0
	.cfi_startproc
	pushq	%rbp
.LCFI92:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI93:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 308 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE46:
	.size	VP8E_SET_TUNING__convert, .-VP8E_SET_TUNING__convert
	.type	VP8E_SET_CQ_LEVEL__value, @function
VP8E_SET_CQ_LEVEL__value:
.LFB47:
	.loc 3 309 0
	.cfi_startproc
	pushq	%rbp
.LCFI94:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI95:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 309 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L145
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L146
.L145:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L146:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE47:
	.size	VP8E_SET_CQ_LEVEL__value, .-VP8E_SET_CQ_LEVEL__value
	.type	VP8E_SET_CQ_LEVEL__convert, @function
VP8E_SET_CQ_LEVEL__convert:
.LFB48:
	.loc 3 309 0
	.cfi_startproc
	pushq	%rbp
.LCFI96:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI97:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 309 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE48:
	.size	VP8E_SET_CQ_LEVEL__convert, .-VP8E_SET_CQ_LEVEL__convert
	.type	VP8E_GET_LAST_QUANTIZER__value, @function
VP8E_GET_LAST_QUANTIZER__value:
.LFB49:
	.loc 3 311 0
	.cfi_startproc
	pushq	%rbp
.LCFI98:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI99:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 311 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L151
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L152
.L151:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L152:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE49:
	.size	VP8E_GET_LAST_QUANTIZER__value, .-VP8E_GET_LAST_QUANTIZER__value
	.type	VP8E_GET_LAST_QUANTIZER__convert, @function
VP8E_GET_LAST_QUANTIZER__convert:
.LFB50:
	.loc 3 311 0
	.cfi_startproc
	pushq	%rbp
.LCFI100:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI101:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 311 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE50:
	.size	VP8E_GET_LAST_QUANTIZER__convert, .-VP8E_GET_LAST_QUANTIZER__convert
	.type	VP8E_GET_LAST_QUANTIZER_64__value, @function
VP8E_GET_LAST_QUANTIZER_64__value:
.LFB51:
	.loc 3 312 0
	.cfi_startproc
	pushq	%rbp
.LCFI102:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI103:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 312 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L157
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L158
.L157:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L158:
	movq	(%rax), %rax
	leave
	ret
	.cfi_endproc
.LFE51:
	.size	VP8E_GET_LAST_QUANTIZER_64__value, .-VP8E_GET_LAST_QUANTIZER_64__value
	.type	VP8E_GET_LAST_QUANTIZER_64__convert, @function
VP8E_GET_LAST_QUANTIZER_64__convert:
.LFB52:
	.loc 3 312 0
	.cfi_startproc
	pushq	%rbp
.LCFI104:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI105:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 312 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	leave
	ret
	.cfi_endproc
.LFE52:
	.size	VP8E_GET_LAST_QUANTIZER_64__convert, .-VP8E_GET_LAST_QUANTIZER_64__convert
	.type	VP8E_SET_MAX_INTRA_BITRATE_PCT__value, @function
VP8E_SET_MAX_INTRA_BITRATE_PCT__value:
.LFB53:
	.loc 3 314 0
	.cfi_startproc
	pushq	%rbp
.LCFI106:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI107:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 3 314 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$47, %eax
	ja	.L163
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	mov	%eax, %eax
	leaq	(%rdx,%rax), %rax
	movq	-8(%rbp), %rdx
	movl	(%rdx), %edx
	leal	8(%rdx), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, (%rdx)
	jmp	.L164
.L163:
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	%rdx, %rax
	leaq	8(%rdx), %rcx
	movq	-8(%rbp), %rdx
	movq	%rcx, 8(%rdx)
.L164:
	movl	(%rax), %eax
	leave
	ret
	.cfi_endproc
.LFE53:
	.size	VP8E_SET_MAX_INTRA_BITRATE_PCT__value, .-VP8E_SET_MAX_INTRA_BITRATE_PCT__value
	.type	VP8E_SET_MAX_INTRA_BITRATE_PCT__convert, @function
VP8E_SET_MAX_INTRA_BITRATE_PCT__convert:
.LFB54:
	.loc 3 314 0
	.cfi_startproc
	pushq	%rbp
.LCFI108:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI109:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	.loc 3 314 0
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
	movl	-16(%rbp), %eax
	leave
	ret
	.cfi_endproc
.LFE54:
	.size	VP8E_SET_MAX_INTRA_BITRATE_PCT__convert, .-VP8E_SET_MAX_INTRA_BITRATE_PCT__convert
	.type	update_blockd_bmi, @function
update_blockd_bmi:
.LFB56:
	.file 4 "../../vp8/common/blockd.h"
	.loc 4 381 0
	.cfi_startproc
	pushq	%rbp
.LCFI110:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI111:
	.cfi_def_cfa_register 6
	pushq	%rbx
	movq	%rdi, -32(%rbp)
	.loc 4 384 0
	movq	-32(%rbp), %rax
	movq	5416(%rax), %rax
	movl	(%rax), %eax
	cmpl	$16, %eax
	je	.L169
	.cfi_offset 3, -24
	.loc 4 385 0
	movq	-32(%rbp), %rax
	movq	5416(%rax), %rax
	movl	(%rax), %eax
	.loc 4 384 0
	cmpl	$10, %eax
	je	.L169
	.loc 4 386 0
	movq	-32(%rbp), %rax
	movq	5416(%rax), %rax
	movl	(%rax), %eax
	.loc 4 384 0
	cmpl	$11, %eax
	jne	.L170
.L169:
	movl	$1, %eax
	jmp	.L171
.L170:
	movl	$0, %eax
.L171:
	movl	%eax, -16(%rbp)
	.loc 4 388 0
	cmpl	$0, -16(%rbp)
	je	.L175
	.loc 4 389 0
	movl	$0, -12(%rbp)
	jmp	.L173
.L174:
	.loc 4 390 0
	movl	-12(%rbp), %edx
	movq	-32(%rbp), %rax
	movq	5416(%rax), %rbx
	movl	-12(%rbp), %ecx
	movq	-32(%rbp), %rsi
	movslq	%edx,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$5, %rax
	leaq	(%rsi,%rax), %rax
	leaq	2896(%rax), %rdx
	movslq	%ecx,%rcx
	movq	%rcx, %rax
	addq	%rax, %rax
	addq	%rcx, %rax
	salq	$2, %rax
	leaq	(%rbx,%rax), %rax
	addq	$32, %rax
	movq	12(%rax), %rcx
	movq	%rcx, 4(%rdx)
	movl	20(%rax), %eax
	movl	%eax, 12(%rdx)
	.loc 4 389 0
	addl	$1, -12(%rbp)
.L173:
	cmpl	$15, -12(%rbp)
	jle	.L174
.L175:
	.loc 4 393 0
	popq	%rbx
	leave
	ret
	.cfi_endproc
.LFE56:
	.size	update_blockd_bmi, .-update_blockd_bmi
	.type	vp8_encode_bool, @function
vp8_encode_bool:
.LFB57:
	.file 5 "../../vp8/encoder/boolhuff.h"
	.loc 5 54 0
	.cfi_startproc
	pushq	%rbp
.LCFI112:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI113:
	.cfi_def_cfa_register 6
	pushq	%rbx
	movq	%rdi, -48(%rbp)
	movl	%esi, -52(%rbp)
	movl	%edx, -56(%rbp)
	.loc 5 56 0
	movq	-48(%rbp), %rax
	movl	12(%rax), %eax
	movl	%eax, -16(%rbp)
	.loc 5 57 0
	movq	-48(%rbp), %rax
	movl	4(%rax), %eax
	movl	%eax, -20(%rbp)
	.loc 5 58 0
	movq	-48(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, -24(%rbp)
	.loc 5 72 0
	movl	-20(%rbp), %eax
	leal	-1(%rax), %edx
	movl	-56(%rbp), %eax
	imull	%edx, %eax
	shrl	$8, %eax
	addl	$1, %eax
	movl	%eax, -12(%rbp)
	.loc 5 74 0
	movl	-12(%rbp), %eax
	movl	%eax, -20(%rbp)
	.loc 5 76 0
	cmpl	$0, -52(%rbp)
	je	.L177
	.cfi_offset 3, -24
	.loc 5 77 0
	movl	-12(%rbp), %eax
	addl	%eax, -24(%rbp)
	.loc 5 78 0
	movq	-48(%rbp), %rax
	movl	4(%rax), %eax
	subl	-12(%rbp), %eax
	movl	%eax, -20(%rbp)
.L177:
	.loc 5 81 0
	movl	-20(%rbp), %eax
	mov	%eax, %eax
	movzbl	vp8_norm(%rax), %eax
	movzbl	%al, %ebx
	.loc 5 83 0
	movl	%ebx, %eax
	movl	%eax, %ecx
	sall	%cl, -20(%rbp)
	.loc 5 84 0
	movl	-16(%rbp), %eax
	addl	%ebx, %eax
	movl	%eax, -16(%rbp)
	.loc 5 86 0
	cmpl	$0, -16(%rbp)
	js	.L178
.LBB3:
	.loc 5 87 0
	movl	-16(%rbp), %eax
	movl	%ebx, %edi
	subl	%eax, %edi
	movl	%edi, %eax
	movl	%eax, -28(%rbp)
	.loc 5 89 0
	movl	-28(%rbp), %eax
	subl	$1, %eax
	movl	-24(%rbp), %edx
	movl	%edx, %ebx
	movl	%eax, %ecx
	sall	%cl, %ebx
	movl	%ebx, %eax
	testl	%eax, %eax
	jns	.L179
.LBB4:
	.loc 5 90 0
	movq	-48(%rbp), %rax
	movl	16(%rax), %eax
	subl	$1, %eax
	movl	%eax, -32(%rbp)
	.loc 5 92 0
	jmp	.L180
.L182:
	.loc 5 93 0
	movq	-48(%rbp), %rax
	movq	24(%rax), %rdx
	movl	-32(%rbp), %eax
	cltq
	leaq	(%rdx,%rax), %rax
	movb	$0, (%rax)
	.loc 5 94 0
	subl	$1, -32(%rbp)
.L180:
	.loc 5 92 0
	cmpl	$0, -32(%rbp)
	js	.L181
	movq	-48(%rbp), %rax
	movq	24(%rax), %rdx
	movl	-32(%rbp), %eax
	cltq
	leaq	(%rdx,%rax), %rax
	movzbl	(%rax), %eax
	cmpb	$-1, %al
	je	.L182
.L181:
	.loc 5 97 0
	movq	-48(%rbp), %rax
	movq	24(%rax), %rdx
	movl	-32(%rbp), %eax
	cltq
	addq	%rax, %rdx
	movq	-48(%rbp), %rax
	movq	24(%rax), %rcx
	movl	-32(%rbp), %eax
	cltq
	leaq	(%rcx,%rax), %rax
	movzbl	(%rax), %eax
	addl	$1, %eax
	movb	%al, (%rdx)
.L179:
.LBE4:
	.loc 5 100 0
	movq	-48(%rbp), %rax
	movq	24(%rax), %rcx
	movq	-48(%rbp), %rax
	movl	16(%rax), %eax
	mov	%eax, %edx
	leaq	(%rcx,%rdx), %rbx
	movl	$24, %edx
	subl	-28(%rbp), %edx
	movl	-24(%rbp), %esi
	movl	%esi, %edi
	movl	%edx, %ecx
	shrl	%cl, %edi
	movl	%edi, %edx
	movb	%dl, (%rbx)
	leal	1(%rax), %edx
	movq	-48(%rbp), %rax
	movl	%edx, 16(%rax)
	.loc 5 101 0
	movl	-28(%rbp), %eax
	movl	%eax, %ecx
	sall	%cl, -24(%rbp)
	.loc 5 102 0
	movl	-16(%rbp), %ebx
	.loc 5 103 0
	andl	$16777215, -24(%rbp)
	.loc 5 104 0
	subl	$8, -16(%rbp)
.L178:
.LBE3:
	.loc 5 107 0
	movl	%ebx, %eax
	movl	%eax, %ecx
	sall	%cl, -24(%rbp)
	.loc 5 108 0
	movq	-48(%rbp), %rax
	movl	-16(%rbp), %edx
	movl	%edx, 12(%rax)
	.loc 5 109 0
	movq	-48(%rbp), %rax
	movl	-24(%rbp), %edx
	movl	%edx, (%rax)
	.loc 5 110 0
	movq	-48(%rbp), %rax
	movl	-20(%rbp), %edx
	movl	%edx, 4(%rax)
	.loc 5 111 0
	popq	%rbx
	leave
	ret
	.cfi_endproc
.LFE57:
	.size	vp8_encode_bool, .-vp8_encode_bool
	.type	mv_bias, @function
mv_bias:
.LFB64:
	.file 6 "../../vp8/common/findnearmv.h"
	.loc 6 22 0
	.cfi_startproc
	pushq	%rbp
.LCFI114:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI115:
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movq	%rdx, -32(%rbp)
	movq	%rcx, -40(%rbp)
	.loc 6 24 0
	movq	-32(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, -16(%rbp)
	.loc 6 26 0
	movl	-24(%rbp), %eax
	cltq
	salq	$2, %rax
	addq	-40(%rbp), %rax
	movl	(%rax), %eax
	cmpl	-20(%rbp), %eax
	je	.L185
	.loc 6 27 0
	movzwl	-16(%rbp), %eax
	negl	%eax
	movw	%ax, -16(%rbp)
	.loc 6 28 0
	movzwl	-14(%rbp), %eax
	negl	%eax
	movw	%ax, -14(%rbp)
.L185:
	.loc 6 31 0
	movq	-32(%rbp), %rax
	movl	-16(%rbp), %edx
	movl	%edx, (%rax)
	.loc 6 32 0
	leave
	ret
	.cfi_endproc
.LFE64:
	.size	mv_bias, .-mv_bias
	.type	vp8_clamp_mv2, @function
vp8_clamp_mv2:
.LFB65:
	.loc 6 36 0
	.cfi_startproc
	pushq	%rbp
.LCFI116:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI117:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 6 37 0
	movq	-8(%rbp), %rax
	movzwl	2(%rax), %eax
	movswl	%ax,%edx
	movq	-16(%rbp), %rax
	movl	5524(%rax), %eax
	addl	$-128, %eax
	cmpl	%eax, %edx
	jge	.L188
	.loc 6 38 0
	movq	-16(%rbp), %rax
	movl	5524(%rax), %eax
	addl	$-128, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, 2(%rax)
	jmp	.L189
.L188:
	.loc 6 39 0
	movq	-8(%rbp), %rax
	movzwl	2(%rax), %eax
	movswl	%ax,%edx
	movq	-16(%rbp), %rax
	movl	5528(%rax), %eax
	subl	$-128, %eax
	cmpl	%eax, %edx
	jle	.L189
	.loc 6 40 0
	movq	-16(%rbp), %rax
	movl	5528(%rax), %eax
	subw	$-128, %ax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, 2(%rax)
.L189:
	.loc 6 42 0
	movq	-8(%rbp), %rax
	movzwl	(%rax), %eax
	movswl	%ax,%edx
	movq	-16(%rbp), %rax
	movl	5532(%rax), %eax
	addl	$-128, %eax
	cmpl	%eax, %edx
	jge	.L190
	.loc 6 43 0
	movq	-16(%rbp), %rax
	movl	5532(%rax), %eax
	addl	$-128, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, (%rax)
	jmp	.L192
.L190:
	.loc 6 44 0
	movq	-8(%rbp), %rax
	movzwl	(%rax), %eax
	movswl	%ax,%edx
	movq	-16(%rbp), %rax
	movl	5536(%rax), %eax
	subl	$-128, %eax
	cmpl	%eax, %edx
	jle	.L192
	.loc 6 45 0
	movq	-16(%rbp), %rax
	movl	5536(%rax), %eax
	subw	$-128, %ax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, (%rax)
.L192:
	.loc 6 46 0
	leave
	ret
	.cfi_endproc
.LFE65:
	.size	vp8_clamp_mv2, .-vp8_clamp_mv2
	.type	vp8_clamp_mv, @function
vp8_clamp_mv:
.LFB66:
	.loc 6 49 0
	.cfi_startproc
	pushq	%rbp
.LCFI118:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI119:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	movl	%ecx, -20(%rbp)
	movl	%r8d, -24(%rbp)
	.loc 6 51 0
	movq	-8(%rbp), %rax
	movzwl	2(%rax), %eax
	.loc 6 50 0
	movswl	%ax,%edx
	movl	-12(%rbp), %eax
	cmpl	%eax, %edx
	cmovge	%edx, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, 2(%rax)
	.loc 6 53 0
	movq	-8(%rbp), %rax
	movzwl	2(%rax), %eax
	.loc 6 52 0
	movswl	%ax,%edx
	movl	-16(%rbp), %eax
	cmpl	%eax, %edx
	cmovle	%edx, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, 2(%rax)
	.loc 6 55 0
	movq	-8(%rbp), %rax
	movzwl	(%rax), %eax
	.loc 6 54 0
	movswl	%ax,%edx
	movl	-20(%rbp), %eax
	cmpl	%eax, %edx
	cmovge	%edx, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, (%rax)
	.loc 6 57 0
	movq	-8(%rbp), %rax
	movzwl	(%rax), %eax
	.loc 6 56 0
	movswl	%ax,%edx
	movl	-24(%rbp), %eax
	cmpl	%eax, %edx
	cmovle	%edx, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, (%rax)
	.loc 6 58 0
	leave
	ret
	.cfi_endproc
.LFE66:
	.size	vp8_clamp_mv, .-vp8_clamp_mv
	.type	vp8_check_mv_bounds, @function
vp8_check_mv_bounds:
.LFB67:
	.loc 6 61 0
	.cfi_startproc
	pushq	%rbp
.LCFI120:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI121:
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movl	%edx, -32(%rbp)
	movl	%ecx, -36(%rbp)
	movl	%r8d, -40(%rbp)
	.loc 6 63 0
	movq	-24(%rbp), %rax
	movzwl	2(%rax), %eax
	cwtl
	cmpl	-28(%rbp), %eax
	setl	%al
	movzbl	%al, %eax
	movl	%eax, -4(%rbp)
	.loc 6 64 0
	movq	-24(%rbp), %rax
	movzwl	2(%rax), %eax
	cwtl
	cmpl	-32(%rbp), %eax
	setg	%al
	movzbl	%al, %eax
	orl	%eax, -4(%rbp)
	.loc 6 65 0
	movq	-24(%rbp), %rax
	movzwl	(%rax), %eax
	cwtl
	cmpl	-36(%rbp), %eax
	setl	%al
	movzbl	%al, %eax
	orl	%eax, -4(%rbp)
	.loc 6 66 0
	movq	-24(%rbp), %rax
	movzwl	(%rax), %eax
	cwtl
	cmpl	-40(%rbp), %eax
	setg	%al
	movzbl	%al, %eax
	orl	%eax, -4(%rbp)
	.loc 6 67 0
	movl	-4(%rbp), %eax
	.loc 6 68 0
	leave
	ret
	.cfi_endproc
.LFE67:
	.size	vp8_check_mv_bounds, .-vp8_check_mv_bounds
	.type	left_block_mv, @function
left_block_mv:
.LFB68:
	.loc 6 88 0
	.cfi_startproc
	pushq	%rbp
.LCFI122:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI123:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	.loc 6 89 0
	movl	-12(%rbp), %eax
	andl	$3, %eax
	testl	%eax, %eax
	jne	.L198
	.loc 6 91 0
	subq	$236, -8(%rbp)
	.loc 6 93 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$16, %eax
	je	.L199
	.loc 6 94 0
	movq	-8(%rbp), %rax
	movl	24(%rax), %eax
	jmp	.L200
.L199:
	.loc 6 95 0
	addl	$4, -12(%rbp)
.L198:
	.loc 6 98 0
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	cltq
	leaq	-1(%rax), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
.L200:
	.loc 6 99 0
	leave
	ret
	.cfi_endproc
.LFE68:
	.size	left_block_mv, .-left_block_mv
	.type	left_block_second_mv, @function
left_block_second_mv:
.LFB69:
	.loc 6 101 0
	.cfi_startproc
	pushq	%rbp
.LCFI124:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI125:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	.loc 6 102 0
	movl	-12(%rbp), %eax
	andl	$3, %eax
	testl	%eax, %eax
	jne	.L203
	.loc 6 104 0
	subq	$236, -8(%rbp)
	.loc 6 106 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$16, %eax
	je	.L204
	.loc 6 107 0
	movq	-8(%rbp), %rax
	movl	16(%rax), %eax
	testl	%eax, %eax
	je	.L205
	movq	-8(%rbp), %rax
	movl	28(%rax), %eax
	jmp	.L206
.L205:
	movq	-8(%rbp), %rax
	movl	24(%rax), %eax
.L206:
	jmp	.L207
.L204:
	.loc 6 108 0
	addl	$4, -12(%rbp)
.L203:
	.loc 6 111 0
	movq	-8(%rbp), %rax
	movl	16(%rax), %eax
	testl	%eax, %eax
	je	.L208
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	cltq
	leaq	-1(%rax), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	leaq	(%rcx,%rax), %rax
	movl	4(%rax), %eax
	jmp	.L209
.L208:
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	cltq
	leaq	-1(%rax), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
.L209:
.L207:
	.loc 6 112 0
	leave
	ret
	.cfi_endproc
.LFE69:
	.size	left_block_second_mv, .-left_block_second_mv
	.type	above_block_mv, @function
above_block_mv:
.LFB70:
	.loc 6 114 0
	.cfi_startproc
	pushq	%rbp
.LCFI126:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI127:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	.loc 6 115 0
	movl	-12(%rbp), %eax
	sarl	$2, %eax
	testl	%eax, %eax
	jne	.L212
	.loc 6 117 0
	movl	-16(%rbp), %eax
	cltq
	imulq	$236, %rax, %rax
	negq	%rax
	addq	%rax, -8(%rbp)
	.loc 6 119 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$16, %eax
	je	.L213
	.loc 6 120 0
	movq	-8(%rbp), %rax
	movl	24(%rax), %eax
	jmp	.L214
.L213:
	.loc 6 121 0
	addl	$16, -12(%rbp)
.L212:
	.loc 6 124 0
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	movslq	%eax,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	subq	$48, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
.L214:
	.loc 6 125 0
	leave
	ret
	.cfi_endproc
.LFE70:
	.size	above_block_mv, .-above_block_mv
	.type	above_block_second_mv, @function
above_block_second_mv:
.LFB71:
	.loc 6 127 0
	.cfi_startproc
	pushq	%rbp
.LCFI128:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI129:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	.loc 6 128 0
	movl	-12(%rbp), %eax
	sarl	$2, %eax
	testl	%eax, %eax
	jne	.L217
	.loc 6 130 0
	movl	-16(%rbp), %eax
	cltq
	imulq	$236, %rax, %rax
	negq	%rax
	addq	%rax, -8(%rbp)
	.loc 6 132 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$16, %eax
	je	.L218
	.loc 6 133 0
	movq	-8(%rbp), %rax
	movl	16(%rax), %eax
	testl	%eax, %eax
	je	.L219
	movq	-8(%rbp), %rax
	movl	28(%rax), %eax
	jmp	.L220
.L219:
	movq	-8(%rbp), %rax
	movl	24(%rax), %eax
.L220:
	jmp	.L221
.L218:
	.loc 6 134 0
	addl	$16, -12(%rbp)
.L217:
	.loc 6 137 0
	movq	-8(%rbp), %rax
	movl	16(%rax), %eax
	testl	%eax, %eax
	je	.L222
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	movslq	%eax,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	subq	$48, %rax
	leaq	(%rcx,%rax), %rax
	movl	4(%rax), %eax
	jmp	.L223
.L222:
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	movslq	%eax,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	subq	$48, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
.L223:
.L221:
	.loc 6 138 0
	leave
	ret
	.cfi_endproc
.LFE71:
	.size	above_block_second_mv, .-above_block_second_mv
	.type	left_block_mode, @function
left_block_mode:
.LFB72:
	.loc 6 140 0
	.cfi_startproc
	pushq	%rbp
.LCFI130:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI131:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	.loc 6 141 0
	movl	-12(%rbp), %eax
	andl	$3, %eax
	testl	%eax, %eax
	jne	.L226
	.loc 6 143 0
	subq	$236, -8(%rbp)
	.loc 6 144 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$11, %eax
	ja	.L227
	mov	%eax, %eax
	movq	.L233(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L233:
	.quad	.L228
	.quad	.L229
	.quad	.L230
	.quad	.L227
	.quad	.L227
	.quad	.L227
	.quad	.L227
	.quad	.L227
	.quad	.L227
	.quad	.L231
	.quad	.L232
	.quad	.L232
	.text
.L228:
	.loc 6 146 0
	movl	$0, %eax
	jmp	.L234
.L229:
	.loc 6 148 0
	movl	$2, %eax
	jmp	.L234
.L230:
	.loc 6 150 0
	movl	$3, %eax
	jmp	.L234
.L231:
	.loc 6 152 0
	movl	$1, %eax
	jmp	.L234
.L232:
	.loc 6 155 0
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	movslq	%eax,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	addq	$36, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
	jmp	.L234
.L227:
	.loc 6 157 0
	movl	$0, %eax
	jmp	.L234
.L226:
	.loc 6 160 0
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	cltq
	leaq	-1(%rax), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
.L234:
	.loc 6 161 0
	leave
	ret
	.cfi_endproc
.LFE72:
	.size	left_block_mode, .-left_block_mode
	.type	above_block_mode, @function
above_block_mode:
.LFB73:
	.loc 6 164 0
	.cfi_startproc
	pushq	%rbp
.LCFI132:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI133:
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	.loc 6 165 0
	movl	-12(%rbp), %eax
	sarl	$2, %eax
	testl	%eax, %eax
	jne	.L237
	.loc 6 167 0
	movl	-16(%rbp), %eax
	cltq
	imulq	$236, %rax, %rax
	negq	%rax
	addq	%rax, -8(%rbp)
	.loc 6 169 0
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$11, %eax
	ja	.L238
	mov	%eax, %eax
	movq	.L244(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L244:
	.quad	.L239
	.quad	.L240
	.quad	.L241
	.quad	.L238
	.quad	.L238
	.quad	.L238
	.quad	.L238
	.quad	.L238
	.quad	.L238
	.quad	.L242
	.quad	.L243
	.quad	.L243
	.text
.L239:
	.loc 6 171 0
	movl	$0, %eax
	jmp	.L245
.L240:
	.loc 6 173 0
	movl	$2, %eax
	jmp	.L245
.L241:
	.loc 6 175 0
	movl	$3, %eax
	jmp	.L245
.L242:
	.loc 6 177 0
	movl	$1, %eax
	jmp	.L245
.L243:
	.loc 6 180 0
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	movslq	%eax,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	addq	$144, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
	jmp	.L245
.L238:
	.loc 6 182 0
	movl	$0, %eax
	jmp	.L245
.L237:
	.loc 6 186 0
	movq	-8(%rbp), %rax
	leaq	44(%rax), %rcx
	movl	-12(%rbp), %eax
	movslq	%eax,%rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$2, %rax
	subq	$48, %rax
	leaq	(%rcx,%rax), %rax
	movl	(%rax), %eax
.L245:
	.loc 6 187 0
	leave
	ret
	.cfi_endproc
.LFE73:
	.size	above_block_mode, .-above_block_mode
.globl main
	.type	main, @function
main:
.LFB74:
	.file 7 "../../vp8/encoder/asm_enc_offsets.c"
	.loc 7 20 0
	.cfi_startproc
	pushq	%rbp
.LCFI134:
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
.LCFI135:
	.cfi_def_cfa_register 6
	.loc 7 23 0
#APP
# 23 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_coeff EQU $8
# 0 "" 2
	.loc 7 24 0
# 24 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_zbin EQU $40
# 0 "" 2
	.loc 7 25 0
# 25 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_round EQU $72
# 0 "" 2
	.loc 7 26 0
# 26 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_quant EQU $16
# 0 "" 2
	.loc 7 27 0
# 27 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_quant_fast EQU $24
# 0 "" 2
	.loc 7 28 0
# 28 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_zbin_extra EQU $80
# 0 "" 2
	.loc 7 29 0
# 29 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_zrun_zbin_boost EQU $56
# 0 "" 2
	.loc 7 30 0
# 30 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_quant_shift EQU $32
# 0 "" 2
	.loc 7 32 0
# 32 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_blockd_qcoeff EQU $0
# 0 "" 2
	.loc 7 33 0
# 33 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_blockd_dequant EQU $32
# 0 "" 2
	.loc 7 34 0
# 34 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_blockd_dqcoeff EQU $8
# 0 "" 2
	.loc 7 35 0
# 35 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_blockd_eob EQU $80
# 0 "" 2
	.loc 7 38 0
# 38 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_base_src EQU $88
# 0 "" 2
	.loc 7 39 0
# 39 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_src EQU $104
# 0 "" 2
	.loc 7 40 0
# 40 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_src_diff EQU $0
# 0 "" 2
	.loc 7 41 0
# 41 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_block_src_stride EQU $108
# 0 "" 2
	.loc 7 43 0
# 43 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_blockd_predictor EQU $16
# 0 "" 2
	.loc 7 46 0
# 46 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_writer_lowvalue EQU $0
# 0 "" 2
	.loc 7 47 0
# 47 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_writer_range EQU $4
# 0 "" 2
	.loc 7 48 0
# 48 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_writer_value EQU $8
# 0 "" 2
	.loc 7 49 0
# 49 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_writer_count EQU $12
# 0 "" 2
	.loc 7 50 0
# 50 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_writer_pos EQU $16
# 0 "" 2
	.loc 7 51 0
# 51 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_writer_buffer EQU $24
# 0 "" 2
	.loc 7 53 0
# 53 "../../vp8/encoder/asm_enc_offsets.c" 1
	
tokenextra_token EQU $10
# 0 "" 2
	.loc 7 54 0
# 54 "../../vp8/encoder/asm_enc_offsets.c" 1
	
tokenextra_extra EQU $8
# 0 "" 2
	.loc 7 55 0
# 55 "../../vp8/encoder/asm_enc_offsets.c" 1
	
tokenextra_context_tree EQU $0
# 0 "" 2
	.loc 7 56 0
# 56 "../../vp8/encoder/asm_enc_offsets.c" 1
	
tokenextra_skip_eob_node EQU $11
# 0 "" 2
	.loc 7 57 0
# 57 "../../vp8/encoder/asm_enc_offsets.c" 1
	
TOKENEXTRA_SZ EQU $16
# 0 "" 2
	.loc 7 59 0
# 59 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_extra_bit_struct_sz EQU $24
# 0 "" 2
	.loc 7 61 0
# 61 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_token_value EQU $0
# 0 "" 2
	.loc 7 62 0
# 62 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_token_len EQU $4
# 0 "" 2
	.loc 7 64 0
# 64 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_extra_bit_struct_tree EQU $0
# 0 "" 2
	.loc 7 65 0
# 65 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_extra_bit_struct_prob EQU $8
# 0 "" 2
	.loc 7 66 0
# 66 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_extra_bit_struct_len EQU $16
# 0 "" 2
	.loc 7 67 0
# 67 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_extra_bit_struct_base_val EQU $20
# 0 "" 2
	.loc 7 69 0
# 69 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_comp_tplist EQU $769536
# 0 "" 2
	.loc 7 70 0
# 70 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_comp_common EQU $393216
# 0 "" 2
	.loc 7 71 0
# 71 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_comp_bc2 EQU $627248
# 0 "" 2
	.loc 7 73 0
# 73 "../../vp8/encoder/asm_enc_offsets.c" 1
	
tokenlist_start EQU $0
# 0 "" 2
	.loc 7 74 0
# 74 "../../vp8/encoder/asm_enc_offsets.c" 1
	
tokenlist_stop EQU $8
# 0 "" 2
	.loc 7 75 0
# 75 "../../vp8/encoder/asm_enc_offsets.c" 1
	
TOKENLIST_SZ EQU $16
# 0 "" 2
	.loc 7 77 0
# 77 "../../vp8/encoder/asm_enc_offsets.c" 1
	
vp8_common_mb_rows EQU $25444
# 0 "" 2
	.loc 7 79 0
#NO_APP
	movl	$0, %eax
	leave
	ret
	.cfi_endproc
.LFE74:
	.size	main, .-main
.Letext0:
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.quad	.LFB0-.Ltext0
	.quad	.LCFI0-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI0-.Ltext0
	.quad	.LCFI1-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI1-.Ltext0
	.quad	.LFE0-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST1:
	.quad	.LFB1-.Ltext0
	.quad	.LCFI2-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI2-.Ltext0
	.quad	.LCFI3-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI3-.Ltext0
	.quad	.LFE1-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST2:
	.quad	.LFB2-.Ltext0
	.quad	.LCFI4-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI4-.Ltext0
	.quad	.LCFI5-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI5-.Ltext0
	.quad	.LFE2-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST3:
	.quad	.LFB3-.Ltext0
	.quad	.LCFI6-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI6-.Ltext0
	.quad	.LCFI7-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI7-.Ltext0
	.quad	.LFE3-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST4:
	.quad	.LFB4-.Ltext0
	.quad	.LCFI8-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI8-.Ltext0
	.quad	.LCFI9-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI9-.Ltext0
	.quad	.LFE4-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST5:
	.quad	.LFB5-.Ltext0
	.quad	.LCFI10-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI10-.Ltext0
	.quad	.LCFI11-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI11-.Ltext0
	.quad	.LFE5-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST6:
	.quad	.LFB6-.Ltext0
	.quad	.LCFI12-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI12-.Ltext0
	.quad	.LCFI13-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI13-.Ltext0
	.quad	.LFE6-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST7:
	.quad	.LFB7-.Ltext0
	.quad	.LCFI14-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI14-.Ltext0
	.quad	.LCFI15-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI15-.Ltext0
	.quad	.LFE7-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST8:
	.quad	.LFB8-.Ltext0
	.quad	.LCFI16-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI16-.Ltext0
	.quad	.LCFI17-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI17-.Ltext0
	.quad	.LFE8-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST9:
	.quad	.LFB9-.Ltext0
	.quad	.LCFI18-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI18-.Ltext0
	.quad	.LCFI19-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI19-.Ltext0
	.quad	.LFE9-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST10:
	.quad	.LFB10-.Ltext0
	.quad	.LCFI20-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI20-.Ltext0
	.quad	.LCFI21-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI21-.Ltext0
	.quad	.LFE10-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST11:
	.quad	.LFB11-.Ltext0
	.quad	.LCFI22-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI22-.Ltext0
	.quad	.LCFI23-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI23-.Ltext0
	.quad	.LFE11-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST12:
	.quad	.LFB12-.Ltext0
	.quad	.LCFI24-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI24-.Ltext0
	.quad	.LCFI25-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI25-.Ltext0
	.quad	.LFE12-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST13:
	.quad	.LFB13-.Ltext0
	.quad	.LCFI26-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI26-.Ltext0
	.quad	.LCFI27-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI27-.Ltext0
	.quad	.LFE13-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST14:
	.quad	.LFB14-.Ltext0
	.quad	.LCFI28-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI28-.Ltext0
	.quad	.LCFI29-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI29-.Ltext0
	.quad	.LFE14-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST15:
	.quad	.LFB15-.Ltext0
	.quad	.LCFI30-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI30-.Ltext0
	.quad	.LCFI31-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI31-.Ltext0
	.quad	.LFE15-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST16:
	.quad	.LFB16-.Ltext0
	.quad	.LCFI32-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI32-.Ltext0
	.quad	.LCFI33-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI33-.Ltext0
	.quad	.LFE16-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST17:
	.quad	.LFB17-.Ltext0
	.quad	.LCFI34-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI34-.Ltext0
	.quad	.LCFI35-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI35-.Ltext0
	.quad	.LFE17-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST18:
	.quad	.LFB18-.Ltext0
	.quad	.LCFI36-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI36-.Ltext0
	.quad	.LCFI37-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI37-.Ltext0
	.quad	.LFE18-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST19:
	.quad	.LFB19-.Ltext0
	.quad	.LCFI38-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI38-.Ltext0
	.quad	.LCFI39-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI39-.Ltext0
	.quad	.LFE19-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST20:
	.quad	.LFB20-.Ltext0
	.quad	.LCFI40-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI40-.Ltext0
	.quad	.LCFI41-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI41-.Ltext0
	.quad	.LFE20-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST21:
	.quad	.LFB21-.Ltext0
	.quad	.LCFI42-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI42-.Ltext0
	.quad	.LCFI43-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI43-.Ltext0
	.quad	.LFE21-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST22:
	.quad	.LFB22-.Ltext0
	.quad	.LCFI44-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI44-.Ltext0
	.quad	.LCFI45-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI45-.Ltext0
	.quad	.LFE22-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST23:
	.quad	.LFB23-.Ltext0
	.quad	.LCFI46-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI46-.Ltext0
	.quad	.LCFI47-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI47-.Ltext0
	.quad	.LFE23-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST24:
	.quad	.LFB24-.Ltext0
	.quad	.LCFI48-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI48-.Ltext0
	.quad	.LCFI49-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI49-.Ltext0
	.quad	.LFE24-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST25:
	.quad	.LFB25-.Ltext0
	.quad	.LCFI50-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI50-.Ltext0
	.quad	.LCFI51-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI51-.Ltext0
	.quad	.LFE25-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST26:
	.quad	.LFB26-.Ltext0
	.quad	.LCFI52-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI52-.Ltext0
	.quad	.LCFI53-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI53-.Ltext0
	.quad	.LFE26-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST27:
	.quad	.LFB27-.Ltext0
	.quad	.LCFI54-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI54-.Ltext0
	.quad	.LCFI55-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI55-.Ltext0
	.quad	.LFE27-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST28:
	.quad	.LFB28-.Ltext0
	.quad	.LCFI56-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI56-.Ltext0
	.quad	.LCFI57-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI57-.Ltext0
	.quad	.LFE28-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST29:
	.quad	.LFB29-.Ltext0
	.quad	.LCFI58-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI58-.Ltext0
	.quad	.LCFI59-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI59-.Ltext0
	.quad	.LFE29-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST30:
	.quad	.LFB30-.Ltext0
	.quad	.LCFI60-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI60-.Ltext0
	.quad	.LCFI61-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI61-.Ltext0
	.quad	.LFE30-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST31:
	.quad	.LFB31-.Ltext0
	.quad	.LCFI62-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI62-.Ltext0
	.quad	.LCFI63-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI63-.Ltext0
	.quad	.LFE31-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST32:
	.quad	.LFB32-.Ltext0
	.quad	.LCFI64-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI64-.Ltext0
	.quad	.LCFI65-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI65-.Ltext0
	.quad	.LFE32-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST33:
	.quad	.LFB33-.Ltext0
	.quad	.LCFI66-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI66-.Ltext0
	.quad	.LCFI67-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI67-.Ltext0
	.quad	.LFE33-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST34:
	.quad	.LFB34-.Ltext0
	.quad	.LCFI68-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI68-.Ltext0
	.quad	.LCFI69-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI69-.Ltext0
	.quad	.LFE34-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST35:
	.quad	.LFB35-.Ltext0
	.quad	.LCFI70-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI70-.Ltext0
	.quad	.LCFI71-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI71-.Ltext0
	.quad	.LFE35-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST36:
	.quad	.LFB36-.Ltext0
	.quad	.LCFI72-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI72-.Ltext0
	.quad	.LCFI73-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI73-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST37:
	.quad	.LFB37-.Ltext0
	.quad	.LCFI74-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI74-.Ltext0
	.quad	.LCFI75-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI75-.Ltext0
	.quad	.LFE37-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST38:
	.quad	.LFB38-.Ltext0
	.quad	.LCFI76-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI76-.Ltext0
	.quad	.LCFI77-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI77-.Ltext0
	.quad	.LFE38-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST39:
	.quad	.LFB39-.Ltext0
	.quad	.LCFI78-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI78-.Ltext0
	.quad	.LCFI79-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI79-.Ltext0
	.quad	.LFE39-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST40:
	.quad	.LFB40-.Ltext0
	.quad	.LCFI80-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI80-.Ltext0
	.quad	.LCFI81-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI81-.Ltext0
	.quad	.LFE40-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST41:
	.quad	.LFB41-.Ltext0
	.quad	.LCFI82-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI82-.Ltext0
	.quad	.LCFI83-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI83-.Ltext0
	.quad	.LFE41-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST42:
	.quad	.LFB42-.Ltext0
	.quad	.LCFI84-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI84-.Ltext0
	.quad	.LCFI85-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI85-.Ltext0
	.quad	.LFE42-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST43:
	.quad	.LFB43-.Ltext0
	.quad	.LCFI86-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI86-.Ltext0
	.quad	.LCFI87-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI87-.Ltext0
	.quad	.LFE43-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST44:
	.quad	.LFB44-.Ltext0
	.quad	.LCFI88-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI88-.Ltext0
	.quad	.LCFI89-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI89-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST45:
	.quad	.LFB45-.Ltext0
	.quad	.LCFI90-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI90-.Ltext0
	.quad	.LCFI91-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI91-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST46:
	.quad	.LFB46-.Ltext0
	.quad	.LCFI92-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI92-.Ltext0
	.quad	.LCFI93-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI93-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST47:
	.quad	.LFB47-.Ltext0
	.quad	.LCFI94-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI94-.Ltext0
	.quad	.LCFI95-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI95-.Ltext0
	.quad	.LFE47-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST48:
	.quad	.LFB48-.Ltext0
	.quad	.LCFI96-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI96-.Ltext0
	.quad	.LCFI97-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI97-.Ltext0
	.quad	.LFE48-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST49:
	.quad	.LFB49-.Ltext0
	.quad	.LCFI98-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI98-.Ltext0
	.quad	.LCFI99-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI99-.Ltext0
	.quad	.LFE49-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST50:
	.quad	.LFB50-.Ltext0
	.quad	.LCFI100-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI100-.Ltext0
	.quad	.LCFI101-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI101-.Ltext0
	.quad	.LFE50-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST51:
	.quad	.LFB51-.Ltext0
	.quad	.LCFI102-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI102-.Ltext0
	.quad	.LCFI103-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI103-.Ltext0
	.quad	.LFE51-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST52:
	.quad	.LFB52-.Ltext0
	.quad	.LCFI104-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI104-.Ltext0
	.quad	.LCFI105-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI105-.Ltext0
	.quad	.LFE52-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST53:
	.quad	.LFB53-.Ltext0
	.quad	.LCFI106-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI106-.Ltext0
	.quad	.LCFI107-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI107-.Ltext0
	.quad	.LFE53-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST54:
	.quad	.LFB54-.Ltext0
	.quad	.LCFI108-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI108-.Ltext0
	.quad	.LCFI109-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI109-.Ltext0
	.quad	.LFE54-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST55:
	.quad	.LFB56-.Ltext0
	.quad	.LCFI110-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI110-.Ltext0
	.quad	.LCFI111-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI111-.Ltext0
	.quad	.LFE56-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST56:
	.quad	.LFB57-.Ltext0
	.quad	.LCFI112-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI112-.Ltext0
	.quad	.LCFI113-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI113-.Ltext0
	.quad	.LFE57-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST57:
	.quad	.LFB64-.Ltext0
	.quad	.LCFI114-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI114-.Ltext0
	.quad	.LCFI115-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI115-.Ltext0
	.quad	.LFE64-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST58:
	.quad	.LFB65-.Ltext0
	.quad	.LCFI116-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI116-.Ltext0
	.quad	.LCFI117-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI117-.Ltext0
	.quad	.LFE65-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST59:
	.quad	.LFB66-.Ltext0
	.quad	.LCFI118-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI118-.Ltext0
	.quad	.LCFI119-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI119-.Ltext0
	.quad	.LFE66-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST60:
	.quad	.LFB67-.Ltext0
	.quad	.LCFI120-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI120-.Ltext0
	.quad	.LCFI121-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI121-.Ltext0
	.quad	.LFE67-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST61:
	.quad	.LFB68-.Ltext0
	.quad	.LCFI122-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI122-.Ltext0
	.quad	.LCFI123-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI123-.Ltext0
	.quad	.LFE68-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST62:
	.quad	.LFB69-.Ltext0
	.quad	.LCFI124-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI124-.Ltext0
	.quad	.LCFI125-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI125-.Ltext0
	.quad	.LFE69-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST63:
	.quad	.LFB70-.Ltext0
	.quad	.LCFI126-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI126-.Ltext0
	.quad	.LCFI127-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI127-.Ltext0
	.quad	.LFE70-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST64:
	.quad	.LFB71-.Ltext0
	.quad	.LCFI128-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI128-.Ltext0
	.quad	.LCFI129-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI129-.Ltext0
	.quad	.LFE71-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST65:
	.quad	.LFB72-.Ltext0
	.quad	.LCFI130-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI130-.Ltext0
	.quad	.LCFI131-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI131-.Ltext0
	.quad	.LFE72-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST66:
	.quad	.LFB73-.Ltext0
	.quad	.LCFI132-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI132-.Ltext0
	.quad	.LCFI133-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI133-.Ltext0
	.quad	.LFE73-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST67:
	.quad	.LFB74-.Ltext0
	.quad	.LCFI134-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 8
	.quad	.LCFI134-.Ltext0
	.quad	.LCFI135-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 16
	.quad	.LCFI135-.Ltext0
	.quad	.LFE74-.Ltext0
	.value	0x2
	.byte	0x76
	.sleb128 16
	.quad	0x0
	.quad	0x0
	.file 8 "/usr/lib/gcc/x86_64-linux-gnu/4.4.3/include/stddef.h"
	.file 9 "/usr/include/stdint.h"
	.file 10 "../../vpx/internal/../vpx_image.h"
	.file 11 "../../vpx/internal/../vpx_codec.h"
	.file 12 "/usr/lib/gcc/x86_64-linux-gnu/4.4.3/include/stdarg.h"
	.file 13 "<built-in>"
	.file 14 "/usr/include/bits/setjmp.h"
	.file 15 "/usr/include/bits/sigset.h"
	.file 16 "/usr/include/setjmp.h"
	.file 17 "../../vpx_scale/yv12config.h"
	.file 18 "../../vp8/common/mv.h"
	.file 19 "../../vp8/common/treecoder.h"
	.file 20 "../../vp8/common/subpixel.h"
	.file 21 "../../vp8/common/onyxc_int.h"
	.file 22 "../../vp8/common/loopfilter.h"
	.file 23 "../../vp8/common/idct.h"
	.file 24 "../../vp8/common/recon.h"
	.file 25 "../../vp8/common/postproc.h"
	.section	.debug_info
	.long	0x2db1
	.value	0x2
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF471
	.byte	0x1
	.long	.LASF472
	.long	.LASF473
	.quad	.Ltext0
	.quad	.Letext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF0
	.uleb128 0x3
	.long	.LASF4
	.byte	0x8
	.byte	0xd3
	.long	0x3f
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF1
	.uleb128 0x4
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF2
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF3
	.uleb128 0x3
	.long	.LASF5
	.byte	0x9
	.byte	0x31
	.long	0x66
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF6
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF7
	.uleb128 0x3
	.long	.LASF8
	.byte	0x9
	.byte	0x34
	.long	0x7f
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF9
	.uleb128 0x5
	.long	.LASF77
	.byte	0x4
	.byte	0xa
	.byte	0x28
	.long	0x103
	.uleb128 0x6
	.long	.LASF10
	.sleb128 0
	.uleb128 0x6
	.long	.LASF11
	.sleb128 1
	.uleb128 0x6
	.long	.LASF12
	.sleb128 2
	.uleb128 0x6
	.long	.LASF13
	.sleb128 3
	.uleb128 0x6
	.long	.LASF14
	.sleb128 4
	.uleb128 0x6
	.long	.LASF15
	.sleb128 5
	.uleb128 0x6
	.long	.LASF16
	.sleb128 6
	.uleb128 0x6
	.long	.LASF17
	.sleb128 7
	.uleb128 0x6
	.long	.LASF18
	.sleb128 8
	.uleb128 0x6
	.long	.LASF19
	.sleb128 9
	.uleb128 0x6
	.long	.LASF20
	.sleb128 10
	.uleb128 0x6
	.long	.LASF21
	.sleb128 11
	.uleb128 0x6
	.long	.LASF22
	.sleb128 12
	.uleb128 0x6
	.long	.LASF23
	.sleb128 13
	.uleb128 0x6
	.long	.LASF24
	.sleb128 769
	.uleb128 0x6
	.long	.LASF25
	.sleb128 258
	.uleb128 0x6
	.long	.LASF26
	.sleb128 771
	.uleb128 0x6
	.long	.LASF27
	.sleb128 260
	.byte	0x0
	.uleb128 0x3
	.long	.LASF28
	.byte	0xa
	.byte	0x3c
	.long	0x86
	.uleb128 0x7
	.long	.LASF52
	.byte	0x70
	.byte	0xa
	.byte	0x61
	.long	0x1db
	.uleb128 0x8
	.string	"fmt"
	.byte	0xa
	.byte	0x62
	.long	0x103
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"w"
	.byte	0xa
	.byte	0x65
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x8
	.string	"h"
	.byte	0xa
	.byte	0x66
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x8
	.string	"d_w"
	.byte	0xa
	.byte	0x69
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x8
	.string	"d_h"
	.byte	0xa
	.byte	0x6a
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF29
	.byte	0xa
	.byte	0x6d
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x9
	.long	.LASF30
	.byte	0xa
	.byte	0x6e
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF31
	.byte	0xa
	.byte	0x7d
	.long	0x1db
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF32
	.byte	0xa
	.byte	0x7e
	.long	0x1f4
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x8
	.string	"bps"
	.byte	0xa
	.byte	0x80
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x9
	.long	.LASF33
	.byte	0xa
	.byte	0x85
	.long	0x204
	.byte	0x2
	.byte	0x23
	.uleb128 0x58
	.uleb128 0x9
	.long	.LASF34
	.byte	0xa
	.byte	0x89
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.uleb128 0x9
	.long	.LASF35
	.byte	0xa
	.byte	0x8a
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x68
	.uleb128 0x9
	.long	.LASF36
	.byte	0xa
	.byte	0x8b
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x6c
	.byte	0x0
	.uleb128 0xa
	.long	0x1ee
	.long	0x1eb
	.uleb128 0xb
	.long	0x1eb
	.byte	0x3
	.byte	0x0
	.uleb128 0xc
	.byte	0x8
	.byte	0x7
	.uleb128 0xd
	.byte	0x8
	.long	0x66
	.uleb128 0xa
	.long	0x46
	.long	0x204
	.uleb128 0xb
	.long	0x1eb
	.byte	0x3
	.byte	0x0
	.uleb128 0xe
	.byte	0x8
	.uleb128 0x3
	.long	.LASF37
	.byte	0xa
	.byte	0x8c
	.long	0x10e
	.uleb128 0xf
	.byte	0x4
	.byte	0xb
	.byte	0x51
	.long	0x256
	.uleb128 0x6
	.long	.LASF38
	.sleb128 0
	.uleb128 0x6
	.long	.LASF39
	.sleb128 1
	.uleb128 0x6
	.long	.LASF40
	.sleb128 2
	.uleb128 0x6
	.long	.LASF41
	.sleb128 3
	.uleb128 0x6
	.long	.LASF42
	.sleb128 4
	.uleb128 0x6
	.long	.LASF43
	.sleb128 5
	.uleb128 0x6
	.long	.LASF44
	.sleb128 6
	.uleb128 0x6
	.long	.LASF45
	.sleb128 7
	.uleb128 0x6
	.long	.LASF46
	.sleb128 8
	.uleb128 0x6
	.long	.LASF47
	.sleb128 9
	.byte	0x0
	.uleb128 0x3
	.long	.LASF48
	.byte	0xb
	.byte	0x86
	.long	0x211
	.uleb128 0xd
	.byte	0x8
	.long	0x267
	.uleb128 0x10
	.long	0x26c
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF49
	.uleb128 0xa
	.long	0x7f
	.long	0x283
	.uleb128 0xb
	.long	0x1eb
	.byte	0x3
	.byte	0x0
	.uleb128 0x2
	.byte	0x8
	.byte	0x4
	.long	.LASF50
	.uleb128 0x3
	.long	.LASF51
	.byte	0xc
	.byte	0x28
	.long	0x295
	.uleb128 0xa
	.long	0x2a5
	.long	0x2a5
	.uleb128 0xb
	.long	0x1eb
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.long	.LASF53
	.byte	0x18
	.byte	0xd
	.byte	0x0
	.long	0x2ea
	.uleb128 0x9
	.long	.LASF54
	.byte	0xd
	.byte	0x0
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF55
	.byte	0xd
	.byte	0x0
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.long	.LASF56
	.byte	0xd
	.byte	0x0
	.long	0x204
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF57
	.byte	0xd
	.byte	0x0
	.long	0x204
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x3
	.long	.LASF58
	.byte	0xc
	.byte	0x66
	.long	0x28a
	.uleb128 0xd
	.byte	0x8
	.long	0x2a5
	.uleb128 0xd
	.byte	0x8
	.long	0x26c
	.uleb128 0x3
	.long	.LASF59
	.byte	0xe
	.byte	0x20
	.long	0x30c
	.uleb128 0xa
	.long	0x2d
	.long	0x31c
	.uleb128 0xb
	.long	0x1eb
	.byte	0x7
	.byte	0x0
	.uleb128 0x11
	.byte	0x80
	.byte	0xf
	.byte	0x1e
	.long	0x333
	.uleb128 0x9
	.long	.LASF60
	.byte	0xf
	.byte	0x1f
	.long	0x333
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.byte	0x0
	.uleb128 0xa
	.long	0x3f
	.long	0x343
	.uleb128 0xb
	.long	0x1eb
	.byte	0xf
	.byte	0x0
	.uleb128 0x3
	.long	.LASF61
	.byte	0xf
	.byte	0x20
	.long	0x31c
	.uleb128 0x7
	.long	.LASF62
	.byte	0xc8
	.byte	0x10
	.byte	0x24
	.long	0x385
	.uleb128 0x9
	.long	.LASF63
	.byte	0x10
	.byte	0x29
	.long	0x301
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF64
	.byte	0x10
	.byte	0x2a
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x9
	.long	.LASF65
	.byte	0x10
	.byte	0x2b
	.long	0x343
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.byte	0x0
	.uleb128 0x3
	.long	.LASF66
	.byte	0x10
	.byte	0x31
	.long	0x390
	.uleb128 0xa
	.long	0x34e
	.long	0x3a0
	.uleb128 0xb
	.long	0x1eb
	.byte	0x0
	.byte	0x0
	.uleb128 0x12
	.long	.LASF67
	.value	0x128
	.byte	0x1
	.value	0x1ab
	.long	0x3fa
	.uleb128 0x13
	.long	.LASF68
	.byte	0x1
	.value	0x1ac
	.long	0x256
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.long	.LASF69
	.byte	0x1
	.value	0x1ad
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x13
	.long	.LASF70
	.byte	0x1
	.value	0x1ae
	.long	0x3fa
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x13
	.long	.LASF71
	.byte	0x1
	.value	0x1af
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x58
	.uleb128 0x14
	.string	"jmp"
	.byte	0x1
	.value	0x1b0
	.long	0x385
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.byte	0x0
	.uleb128 0xa
	.long	0x26c
	.long	0x40a
	.uleb128 0xb
	.long	0x1eb
	.byte	0x4f
	.byte	0x0
	.uleb128 0x7
	.long	.LASF72
	.byte	0xc
	.byte	0x2
	.byte	0x49
	.long	0x441
	.uleb128 0x9
	.long	.LASF73
	.byte	0x2
	.byte	0x4a
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF74
	.byte	0x2
	.byte	0x4b
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.long	.LASF75
	.byte	0x2
	.byte	0x4c
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0x3
	.long	.LASF76
	.byte	0x2
	.byte	0x4d
	.long	0x40a
	.uleb128 0x5
	.long	.LASF78
	.byte	0x4
	.byte	0x2
	.byte	0x53
	.long	0x46b
	.uleb128 0x6
	.long	.LASF79
	.sleb128 1
	.uleb128 0x6
	.long	.LASF80
	.sleb128 2
	.uleb128 0x6
	.long	.LASF81
	.sleb128 4
	.byte	0x0
	.uleb128 0x3
	.long	.LASF82
	.byte	0x2
	.byte	0x57
	.long	0x44c
	.uleb128 0x7
	.long	.LASF83
	.byte	0x78
	.byte	0x2
	.byte	0x5e
	.long	0x49f
	.uleb128 0x9
	.long	.LASF84
	.byte	0x2
	.byte	0x5f
	.long	0x46b
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"img"
	.byte	0x2
	.byte	0x60
	.long	0x206
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0x3
	.long	.LASF85
	.byte	0x2
	.byte	0x61
	.long	0x476
	.uleb128 0x5
	.long	.LASF86
	.byte	0x4
	.byte	0x3
	.byte	0xc4
	.long	0x4cf
	.uleb128 0x6
	.long	.LASF87
	.sleb128 0
	.uleb128 0x6
	.long	.LASF88
	.sleb128 1
	.uleb128 0x6
	.long	.LASF89
	.sleb128 2
	.uleb128 0x6
	.long	.LASF90
	.sleb128 3
	.byte	0x0
	.uleb128 0x3
	.long	.LASF91
	.byte	0x3
	.byte	0xc9
	.long	0x4aa
	.uleb128 0x7
	.long	.LASF92
	.byte	0x40
	.byte	0x3
	.byte	0xd2
	.long	0x53b
	.uleb128 0x9
	.long	.LASF93
	.byte	0x3
	.byte	0xd3
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF94
	.byte	0x3
	.byte	0xd4
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF95
	.byte	0x3
	.byte	0xd5
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x9
	.long	.LASF96
	.byte	0x3
	.byte	0xd6
	.long	0x1f4
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF97
	.byte	0x3
	.byte	0xd7
	.long	0x1f4
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF98
	.byte	0x3
	.byte	0xd8
	.long	0x273
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.byte	0x0
	.uleb128 0x3
	.long	.LASF99
	.byte	0x3
	.byte	0xd9
	.long	0x4da
	.uleb128 0x7
	.long	.LASF100
	.byte	0x10
	.byte	0x3
	.byte	0xe2
	.long	0x57d
	.uleb128 0x9
	.long	.LASF101
	.byte	0x3
	.byte	0xe3
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF94
	.byte	0x3
	.byte	0xe4
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF95
	.byte	0x3
	.byte	0xe5
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0x3
	.long	.LASF102
	.byte	0x3
	.byte	0xe6
	.long	0x546
	.uleb128 0x7
	.long	.LASF103
	.byte	0x8
	.byte	0x3
	.byte	0xed
	.long	0x5b1
	.uleb128 0x9
	.long	.LASF104
	.byte	0x3
	.byte	0xee
	.long	0x4cf
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF105
	.byte	0x3
	.byte	0xef
	.long	0x4cf
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x3
	.long	.LASF106
	.byte	0x3
	.byte	0xf0
	.long	0x588
	.uleb128 0x15
	.byte	0x4
	.byte	0x3
	.value	0x104
	.long	0x5de
	.uleb128 0x6
	.long	.LASF107
	.sleb128 0
	.uleb128 0x6
	.long	.LASF108
	.sleb128 1
	.uleb128 0x6
	.long	.LASF109
	.sleb128 2
	.uleb128 0x6
	.long	.LASF110
	.sleb128 3
	.byte	0x0
	.uleb128 0x16
	.long	.LASF111
	.byte	0x3
	.value	0x109
	.long	0x5bc
	.uleb128 0x15
	.byte	0x4
	.byte	0x3
	.value	0x111
	.long	0x600
	.uleb128 0x6
	.long	.LASF112
	.sleb128 0
	.uleb128 0x6
	.long	.LASF113
	.sleb128 1
	.byte	0x0
	.uleb128 0x16
	.long	.LASF114
	.byte	0x3
	.value	0x114
	.long	0x5ea
	.uleb128 0xf
	.byte	0x4
	.byte	0x11
	.byte	0x29
	.long	0x621
	.uleb128 0x6
	.long	.LASF115
	.sleb128 0
	.uleb128 0x6
	.long	.LASF116
	.sleb128 1
	.byte	0x0
	.uleb128 0x3
	.long	.LASF117
	.byte	0x11
	.byte	0x2d
	.long	0x60c
	.uleb128 0x11
	.byte	0x50
	.byte	0x11
	.byte	0x2f
	.long	0x707
	.uleb128 0x9
	.long	.LASF118
	.byte	0x11
	.byte	0x30
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF119
	.byte	0x11
	.byte	0x31
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.long	.LASF120
	.byte	0x11
	.byte	0x32
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF121
	.byte	0x11
	.byte	0x35
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x9
	.long	.LASF122
	.byte	0x11
	.byte	0x36
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF123
	.byte	0x11
	.byte	0x37
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x9
	.long	.LASF124
	.byte	0x11
	.byte	0x3a
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF125
	.byte	0x11
	.byte	0x3b
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF126
	.byte	0x11
	.byte	0x3c
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF127
	.byte	0x11
	.byte	0x3e
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x9
	.long	.LASF128
	.byte	0x11
	.byte	0x3f
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x9
	.long	.LASF129
	.byte	0x11
	.byte	0x40
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x3c
	.uleb128 0x9
	.long	.LASF130
	.byte	0x11
	.byte	0x41
	.long	0x621
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x9
	.long	.LASF131
	.byte	0x11
	.byte	0x43
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x44
	.uleb128 0x9
	.long	.LASF132
	.byte	0x11
	.byte	0x44
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.byte	0x0
	.uleb128 0x3
	.long	.LASF133
	.byte	0x11
	.byte	0x45
	.long	0x62c
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF134
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.long	.LASF135
	.uleb128 0xd
	.byte	0x8
	.long	0x46
	.uleb128 0x11
	.byte	0x4
	.byte	0x12
	.byte	0x10
	.long	0x74b
	.uleb128 0x8
	.string	"row"
	.byte	0x12
	.byte	0x11
	.long	0x54
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"col"
	.byte	0x12
	.byte	0x12
	.long	0x54
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.byte	0x0
	.uleb128 0x17
	.string	"MV"
	.byte	0x12
	.byte	0x13
	.long	0x726
	.uleb128 0x18
	.byte	0x4
	.byte	0x12
	.byte	0x15
	.long	0x774
	.uleb128 0x19
	.long	.LASF136
	.byte	0x12
	.byte	0x16
	.long	0x74
	.uleb128 0x19
	.long	.LASF137
	.byte	0x12
	.byte	0x17
	.long	0x74b
	.byte	0x0
	.uleb128 0x3
	.long	.LASF138
	.byte	0x12
	.byte	0x18
	.long	0x755
	.uleb128 0x3
	.long	.LASF139
	.byte	0x13
	.byte	0x12
	.long	0x66
	.uleb128 0x3
	.long	.LASF140
	.byte	0x14
	.byte	0xa8
	.long	0x795
	.uleb128 0xd
	.byte	0x8
	.long	0x79b
	.uleb128 0x1a
	.byte	0x1
	.long	0x7c0
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x11
	.byte	0xe0
	.byte	0x14
	.byte	0xa9
	.long	0x95d
	.uleb128 0x9
	.long	.LASF141
	.byte	0x14
	.byte	0xab
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF142
	.byte	0x14
	.byte	0xac
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF143
	.byte	0x14
	.byte	0xad
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF144
	.byte	0x14
	.byte	0xae
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF145
	.byte	0x14
	.byte	0xaf
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF146
	.byte	0x14
	.byte	0xb0
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF147
	.byte	0x14
	.byte	0xb1
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x9
	.long	.LASF148
	.byte	0x14
	.byte	0xb2
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x9
	.long	.LASF149
	.byte	0x14
	.byte	0xb3
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x9
	.long	.LASF150
	.byte	0x14
	.byte	0xb4
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x9
	.long	.LASF151
	.byte	0x14
	.byte	0xb5
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x9
	.long	.LASF152
	.byte	0x14
	.byte	0xb6
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x58
	.uleb128 0x9
	.long	.LASF153
	.byte	0x14
	.byte	0xb7
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.uleb128 0x9
	.long	.LASF154
	.byte	0x14
	.byte	0xb8
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x68
	.uleb128 0x9
	.long	.LASF155
	.byte	0x14
	.byte	0xba
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x70
	.uleb128 0x9
	.long	.LASF156
	.byte	0x14
	.byte	0xbb
	.long	0x78a
	.byte	0x2
	.byte	0x23
	.uleb128 0x78
	.uleb128 0x9
	.long	.LASF157
	.byte	0x14
	.byte	0xbc
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x9
	.long	.LASF158
	.byte	0x14
	.byte	0xbd
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x88
	.uleb128 0x9
	.long	.LASF159
	.byte	0x14
	.byte	0xbe
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x90
	.uleb128 0x9
	.long	.LASF160
	.byte	0x14
	.byte	0xbf
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x98
	.uleb128 0x9
	.long	.LASF161
	.byte	0x14
	.byte	0xc0
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xa0
	.uleb128 0x9
	.long	.LASF162
	.byte	0x14
	.byte	0xc1
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xa8
	.uleb128 0x9
	.long	.LASF163
	.byte	0x14
	.byte	0xc2
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xb0
	.uleb128 0x9
	.long	.LASF164
	.byte	0x14
	.byte	0xc3
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xb8
	.uleb128 0x9
	.long	.LASF165
	.byte	0x14
	.byte	0xc4
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xc0
	.uleb128 0x9
	.long	.LASF166
	.byte	0x14
	.byte	0xc5
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xc8
	.uleb128 0x9
	.long	.LASF167
	.byte	0x14
	.byte	0xc6
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xd0
	.uleb128 0x9
	.long	.LASF168
	.byte	0x14
	.byte	0xc7
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0xd8
	.byte	0x0
	.uleb128 0x3
	.long	.LASF169
	.byte	0x14
	.byte	0xc8
	.long	0x7c0
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF170
	.uleb128 0xd
	.byte	0x8
	.long	0x975
	.uleb128 0x10
	.long	0x46
	.uleb128 0x3
	.long	.LASF171
	.byte	0x4
	.byte	0x3c
	.long	0x26c
	.uleb128 0x11
	.byte	0x9
	.byte	0x4
	.byte	0x3d
	.long	0x9c0
	.uleb128 0x8
	.string	"y1"
	.byte	0x4
	.byte	0x3e
	.long	0x9c0
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"u"
	.byte	0x4
	.byte	0x3f
	.long	0x9d0
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x8
	.string	"v"
	.byte	0x4
	.byte	0x40
	.long	0x9d0
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x8
	.string	"y2"
	.byte	0x4
	.byte	0x41
	.long	0x97a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0xa
	.long	0x97a
	.long	0x9d0
	.uleb128 0xb
	.long	0x1eb
	.byte	0x3
	.byte	0x0
	.uleb128 0xa
	.long	0x97a
	.long	0x9e0
	.uleb128 0xb
	.long	0x1eb
	.byte	0x1
	.byte	0x0
	.uleb128 0x3
	.long	.LASF172
	.byte	0x4
	.byte	0x42
	.long	0x985
	.uleb128 0xf
	.byte	0x4
	.byte	0x4
	.byte	0x4c
	.long	0xa00
	.uleb128 0x6
	.long	.LASF173
	.sleb128 0
	.uleb128 0x6
	.long	.LASF174
	.sleb128 1
	.byte	0x0
	.uleb128 0x3
	.long	.LASF175
	.byte	0x4
	.byte	0x4f
	.long	0x9eb
	.uleb128 0xf
	.byte	0x4
	.byte	0x4
	.byte	0x51
	.long	0xa80
	.uleb128 0x6
	.long	.LASF176
	.sleb128 0
	.uleb128 0x6
	.long	.LASF177
	.sleb128 1
	.uleb128 0x6
	.long	.LASF178
	.sleb128 2
	.uleb128 0x6
	.long	.LASF179
	.sleb128 3
	.uleb128 0x6
	.long	.LASF180
	.sleb128 4
	.uleb128 0x6
	.long	.LASF181
	.sleb128 5
	.uleb128 0x6
	.long	.LASF182
	.sleb128 6
	.uleb128 0x6
	.long	.LASF183
	.sleb128 7
	.uleb128 0x6
	.long	.LASF184
	.sleb128 8
	.uleb128 0x6
	.long	.LASF185
	.sleb128 9
	.uleb128 0x6
	.long	.LASF186
	.sleb128 10
	.uleb128 0x6
	.long	.LASF187
	.sleb128 11
	.uleb128 0x6
	.long	.LASF188
	.sleb128 12
	.uleb128 0x6
	.long	.LASF189
	.sleb128 13
	.uleb128 0x6
	.long	.LASF190
	.sleb128 14
	.uleb128 0x6
	.long	.LASF191
	.sleb128 15
	.uleb128 0x6
	.long	.LASF192
	.sleb128 16
	.uleb128 0x6
	.long	.LASF193
	.sleb128 17
	.byte	0x0
	.uleb128 0x3
	.long	.LASF194
	.byte	0x4
	.byte	0x68
	.long	0xa0b
	.uleb128 0xf
	.byte	0x4
	.byte	0x4
	.byte	0x77
	.long	0xaa6
	.uleb128 0x6
	.long	.LASF195
	.sleb128 0
	.uleb128 0x6
	.long	.LASF196
	.sleb128 1
	.uleb128 0x6
	.long	.LASF197
	.sleb128 2
	.byte	0x0
	.uleb128 0x3
	.long	.LASF198
	.byte	0x4
	.byte	0x7d
	.long	0xa8b
	.uleb128 0xf
	.byte	0x4
	.byte	0x4
	.byte	0x81
	.long	0xad2
	.uleb128 0x6
	.long	.LASF199
	.sleb128 0
	.uleb128 0x6
	.long	.LASF200
	.sleb128 1
	.uleb128 0x6
	.long	.LASF201
	.sleb128 2
	.uleb128 0x6
	.long	.LASF202
	.sleb128 3
	.byte	0x0
	.uleb128 0x3
	.long	.LASF203
	.byte	0x4
	.byte	0x86
	.long	0xab1
	.uleb128 0xf
	.byte	0x4
	.byte	0x4
	.byte	0x93
	.long	0xb40
	.uleb128 0x6
	.long	.LASF204
	.sleb128 0
	.uleb128 0x6
	.long	.LASF205
	.sleb128 1
	.uleb128 0x6
	.long	.LASF206
	.sleb128 2
	.uleb128 0x6
	.long	.LASF207
	.sleb128 3
	.uleb128 0x6
	.long	.LASF208
	.sleb128 4
	.uleb128 0x6
	.long	.LASF209
	.sleb128 5
	.uleb128 0x6
	.long	.LASF210
	.sleb128 6
	.uleb128 0x6
	.long	.LASF211
	.sleb128 7
	.uleb128 0x6
	.long	.LASF212
	.sleb128 8
	.uleb128 0x6
	.long	.LASF213
	.sleb128 9
	.uleb128 0x6
	.long	.LASF214
	.sleb128 10
	.uleb128 0x6
	.long	.LASF215
	.sleb128 11
	.uleb128 0x6
	.long	.LASF216
	.sleb128 12
	.uleb128 0x6
	.long	.LASF217
	.sleb128 13
	.uleb128 0x6
	.long	.LASF218
	.sleb128 14
	.byte	0x0
	.uleb128 0x3
	.long	.LASF219
	.byte	0x4
	.byte	0xa8
	.long	0xadd
	.uleb128 0x11
	.byte	0xc
	.byte	0x4
	.byte	0xb2
	.long	0xb7e
	.uleb128 0x9
	.long	.LASF220
	.byte	0x4
	.byte	0xb3
	.long	0xb40
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF221
	.byte	0x4
	.byte	0xb5
	.long	0xb40
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.long	.LASF222
	.byte	0x4
	.byte	0xb6
	.long	0xad2
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0x11
	.byte	0x8
	.byte	0x4
	.byte	0xbd
	.long	0xba3
	.uleb128 0x9
	.long	.LASF220
	.byte	0x4
	.byte	0xbe
	.long	0x774
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF223
	.byte	0x4
	.byte	0xbf
	.long	0x774
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x1c
	.long	.LASF474
	.byte	0xc
	.byte	0x4
	.byte	0xb1
	.long	0xbc6
	.uleb128 0x19
	.long	.LASF224
	.byte	0x4
	.byte	0xbc
	.long	0xb4b
	.uleb128 0x19
	.long	.LASF137
	.byte	0x4
	.byte	0xc0
	.long	0xb7e
	.byte	0x0
	.uleb128 0xf
	.byte	0x4
	.byte	0x4
	.byte	0xc3
	.long	0xbed
	.uleb128 0x6
	.long	.LASF225
	.sleb128 0
	.uleb128 0x6
	.long	.LASF226
	.sleb128 1
	.uleb128 0x6
	.long	.LASF227
	.sleb128 2
	.uleb128 0x6
	.long	.LASF228
	.sleb128 3
	.uleb128 0x6
	.long	.LASF229
	.sleb128 4
	.byte	0x0
	.uleb128 0x3
	.long	.LASF230
	.byte	0x4
	.byte	0xc9
	.long	0xbc6
	.uleb128 0x11
	.byte	0x2c
	.byte	0x4
	.byte	0xcb
	.long	0xcee
	.uleb128 0x9
	.long	.LASF231
	.byte	0x4
	.byte	0xcc
	.long	0xa80
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF232
	.byte	0x4
	.byte	0xcc
	.long	0xa80
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.long	.LASF233
	.byte	0x4
	.byte	0xce
	.long	0xa80
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF234
	.byte	0x4
	.byte	0xd4
	.long	0xbed
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x9
	.long	.LASF235
	.byte	0x4
	.byte	0xd4
	.long	0xbed
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF236
	.byte	0x4
	.byte	0xd5
	.long	0xaa6
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x8
	.string	"mv"
	.byte	0x4
	.byte	0xd6
	.long	0x774
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF237
	.byte	0x4
	.byte	0xd6
	.long	0x774
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x9
	.long	.LASF238
	.byte	0x4
	.byte	0xd7
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF239
	.byte	0x4
	.byte	0xd8
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x21
	.uleb128 0x9
	.long	.LASF240
	.byte	0x4
	.byte	0xd9
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x22
	.uleb128 0x9
	.long	.LASF241
	.byte	0x4
	.byte	0xda
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x23
	.uleb128 0x9
	.long	.LASF242
	.byte	0x4
	.byte	0xdb
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x9
	.long	.LASF243
	.byte	0x4
	.byte	0xde
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x25
	.uleb128 0x9
	.long	.LASF244
	.byte	0x4
	.byte	0xdf
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x26
	.uleb128 0x9
	.long	.LASF245
	.byte	0x4
	.byte	0xe4
	.long	0x66
	.byte	0x2
	.byte	0x23
	.uleb128 0x27
	.uleb128 0x9
	.long	.LASF246
	.byte	0x4
	.byte	0xe8
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x3
	.long	.LASF247
	.byte	0x4
	.byte	0xeb
	.long	0xbf8
	.uleb128 0x11
	.byte	0xec
	.byte	0x4
	.byte	0xed
	.long	0xd1e
	.uleb128 0x9
	.long	.LASF248
	.byte	0x4
	.byte	0xee
	.long	0xcee
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"bmi"
	.byte	0x4
	.byte	0xef
	.long	0xd1e
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.byte	0x0
	.uleb128 0xa
	.long	0xba3
	.long	0xd2e
	.uleb128 0xb
	.long	0x1eb
	.byte	0xf
	.byte	0x0
	.uleb128 0x3
	.long	.LASF249
	.byte	0x4
	.byte	0xf0
	.long	0xcf9
	.uleb128 0x11
	.byte	0x60
	.byte	0x4
	.byte	0xf2
	.long	0xe0a
	.uleb128 0x9
	.long	.LASF250
	.byte	0x4
	.byte	0xf3
	.long	0xe0a
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF251
	.byte	0x4
	.byte	0xf4
	.long	0xe0a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF252
	.byte	0x4
	.byte	0xf5
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF253
	.byte	0x4
	.byte	0xf6
	.long	0xe0a
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF254
	.byte	0x4
	.byte	0xf7
	.long	0xe0a
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF255
	.byte	0x4
	.byte	0xfa
	.long	0xe10
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF256
	.byte	0x4
	.byte	0xfb
	.long	0xe10
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x8
	.string	"pre"
	.byte	0x4
	.byte	0xfc
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x9
	.long	.LASF257
	.byte	0x4
	.byte	0xfd
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x3c
	.uleb128 0x9
	.long	.LASF258
	.byte	0x4
	.byte	0xff
	.long	0xe10
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x14
	.string	"dst"
	.byte	0x4
	.value	0x100
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x13
	.long	.LASF259
	.byte	0x4
	.value	0x101
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x4c
	.uleb128 0x14
	.string	"eob"
	.byte	0x4
	.value	0x103
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x14
	.string	"bmi"
	.byte	0x4
	.value	0x105
	.long	0xba3
	.byte	0x2
	.byte	0x23
	.uleb128 0x54
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x54
	.uleb128 0xd
	.byte	0x8
	.long	0x1ee
	.uleb128 0x16
	.long	.LASF260
	.byte	0x4
	.value	0x106
	.long	0xd39
	.uleb128 0x1d
	.byte	0x18
	.byte	0x4
	.value	0x114
	.long	0xe59
	.uleb128 0x13
	.long	.LASF124
	.byte	0x4
	.value	0x115
	.long	0xe59
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.long	.LASF125
	.byte	0x4
	.value	0x115
	.long	0xe59
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x13
	.long	.LASF126
	.byte	0x4
	.value	0x115
	.long	0xe59
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x5b
	.uleb128 0x12
	.long	.LASF261
	.value	0x1900
	.byte	0x4
	.value	0x108
	.long	0x11ad
	.uleb128 0x13
	.long	.LASF253
	.byte	0x4
	.value	0x109
	.long	0x11ad
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.long	.LASF252
	.byte	0x4
	.value	0x10a
	.long	0x11be
	.byte	0x3
	.byte	0x23
	.uleb128 0x320
	.uleb128 0x13
	.long	.LASF250
	.byte	0x4
	.value	0x10b
	.long	0x11ad
	.byte	0x3
	.byte	0x23
	.uleb128 0x4a0
	.uleb128 0x13
	.long	.LASF251
	.byte	0x4
	.value	0x10c
	.long	0x11ad
	.byte	0x3
	.byte	0x23
	.uleb128 0x7c0
	.uleb128 0x13
	.long	.LASF262
	.byte	0x4
	.value	0x10d
	.long	0x11cf
	.byte	0x3
	.byte	0x23
	.uleb128 0xae0
	.uleb128 0x13
	.long	.LASF263
	.byte	0x4
	.value	0x110
	.long	0x11df
	.byte	0x3
	.byte	0x23
	.uleb128 0xb00
	.uleb128 0x13
	.long	.LASF264
	.byte	0x4
	.value	0x111
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x1460
	.uleb128 0x14
	.string	"pre"
	.byte	0x4
	.value	0x113
	.long	0x707
	.byte	0x3
	.byte	0x23
	.uleb128 0x1468
	.uleb128 0x13
	.long	.LASF265
	.byte	0x4
	.value	0x116
	.long	0xe22
	.byte	0x3
	.byte	0x23
	.uleb128 0x14b8
	.uleb128 0x14
	.string	"dst"
	.byte	0x4
	.value	0x117
	.long	0x707
	.byte	0x3
	.byte	0x23
	.uleb128 0x14d0
	.uleb128 0x13
	.long	.LASF266
	.byte	0x4
	.value	0x119
	.long	0x11ef
	.byte	0x3
	.byte	0x23
	.uleb128 0x1520
	.uleb128 0x13
	.long	.LASF267
	.byte	0x4
	.value	0x11a
	.long	0x11ef
	.byte	0x3
	.byte	0x23
	.uleb128 0x1528
	.uleb128 0x13
	.long	.LASF268
	.byte	0x4
	.value	0x11b
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x1530
	.uleb128 0x13
	.long	.LASF84
	.byte	0x4
	.value	0x11d
	.long	0xa00
	.byte	0x3
	.byte	0x23
	.uleb128 0x1534
	.uleb128 0x13
	.long	.LASF269
	.byte	0x4
	.value	0x11f
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x1538
	.uleb128 0x13
	.long	.LASF270
	.byte	0x4
	.value	0x120
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x153c
	.uleb128 0x13
	.long	.LASF271
	.byte	0x4
	.value	0x123
	.long	0x11f5
	.byte	0x3
	.byte	0x23
	.uleb128 0x1540
	.uleb128 0x13
	.long	.LASF272
	.byte	0x4
	.value	0x124
	.long	0x11f5
	.byte	0x3
	.byte	0x23
	.uleb128 0x1548
	.uleb128 0x13
	.long	.LASF273
	.byte	0x4
	.value	0x127
	.long	0x66
	.byte	0x3
	.byte	0x23
	.uleb128 0x1550
	.uleb128 0x13
	.long	.LASF274
	.byte	0x4
	.value	0x12a
	.long	0x66
	.byte	0x3
	.byte	0x23
	.uleb128 0x1551
	.uleb128 0x13
	.long	.LASF275
	.byte	0x4
	.value	0x12d
	.long	0x66
	.byte	0x3
	.byte	0x23
	.uleb128 0x1552
	.uleb128 0x13
	.long	.LASF276
	.byte	0x4
	.value	0x130
	.long	0x66
	.byte	0x3
	.byte	0x23
	.uleb128 0x1553
	.uleb128 0x13
	.long	.LASF277
	.byte	0x4
	.value	0x136
	.long	0x11fb
	.byte	0x3
	.byte	0x23
	.uleb128 0x1554
	.uleb128 0x13
	.long	.LASF278
	.byte	0x4
	.value	0x13a
	.long	0x120b
	.byte	0x3
	.byte	0x23
	.uleb128 0x1557
	.uleb128 0x13
	.long	.LASF279
	.byte	0x4
	.value	0x13b
	.long	0x273
	.byte	0x3
	.byte	0x23
	.uleb128 0x1570
	.uleb128 0x13
	.long	.LASF280
	.byte	0x4
	.value	0x144
	.long	0x66
	.byte	0x3
	.byte	0x23
	.uleb128 0x1580
	.uleb128 0x13
	.long	.LASF281
	.byte	0x4
	.value	0x145
	.long	0x66
	.byte	0x3
	.byte	0x23
	.uleb128 0x1581
	.uleb128 0x13
	.long	.LASF282
	.byte	0x4
	.value	0x148
	.long	0x1221
	.byte	0x3
	.byte	0x23
	.uleb128 0x1582
	.uleb128 0x13
	.long	.LASF283
	.byte	0x4
	.value	0x149
	.long	0x1221
	.byte	0x3
	.byte	0x23
	.uleb128 0x1586
	.uleb128 0x13
	.long	.LASF284
	.byte	0x4
	.value	0x14a
	.long	0x1221
	.byte	0x3
	.byte	0x23
	.uleb128 0x158a
	.uleb128 0x13
	.long	.LASF285
	.byte	0x4
	.value	0x14b
	.long	0x1221
	.byte	0x3
	.byte	0x23
	.uleb128 0x158e
	.uleb128 0x13
	.long	.LASF286
	.byte	0x4
	.value	0x14e
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x1594
	.uleb128 0x13
	.long	.LASF287
	.byte	0x4
	.value	0x14f
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x1598
	.uleb128 0x13
	.long	.LASF288
	.byte	0x4
	.value	0x150
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x159c
	.uleb128 0x13
	.long	.LASF289
	.byte	0x4
	.value	0x151
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x15a0
	.uleb128 0x13
	.long	.LASF290
	.byte	0x4
	.value	0x153
	.long	0x7f
	.byte	0x3
	.byte	0x23
	.uleb128 0x15a4
	.uleb128 0x13
	.long	.LASF291
	.byte	0x4
	.value	0x154
	.long	0x7f
	.byte	0x3
	.byte	0x23
	.uleb128 0x15a8
	.uleb128 0x13
	.long	.LASF292
	.byte	0x4
	.value	0x155
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15b0
	.uleb128 0x13
	.long	.LASF293
	.byte	0x4
	.value	0x156
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15b8
	.uleb128 0x13
	.long	.LASF294
	.byte	0x4
	.value	0x157
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15c0
	.uleb128 0x13
	.long	.LASF295
	.byte	0x4
	.value	0x158
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15c8
	.uleb128 0x13
	.long	.LASF296
	.byte	0x4
	.value	0x159
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15d0
	.uleb128 0x13
	.long	.LASF297
	.byte	0x4
	.value	0x15a
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15d8
	.uleb128 0x13
	.long	.LASF298
	.byte	0x4
	.value	0x15b
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15e0
	.uleb128 0x13
	.long	.LASF299
	.byte	0x4
	.value	0x15c
	.long	0x78a
	.byte	0x3
	.byte	0x23
	.uleb128 0x15e8
	.uleb128 0x13
	.long	.LASF300
	.byte	0x4
	.value	0x15e
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x15f0
	.uleb128 0x13
	.long	.LASF301
	.byte	0x4
	.value	0x161
	.long	0x204
	.byte	0x3
	.byte	0x23
	.uleb128 0x15f8
	.uleb128 0x13
	.long	.LASF131
	.byte	0x4
	.value	0x163
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x1600
	.uleb128 0x13
	.long	.LASF302
	.byte	0x4
	.value	0x16a
	.long	0x1231
	.byte	0x3
	.byte	0x23
	.uleb128 0x1620
	.uleb128 0x13
	.long	.LASF303
	.byte	0x4
	.value	0x16e
	.long	0x12a9
	.byte	0x3
	.byte	0x23
	.uleb128 0x18e0
	.uleb128 0x13
	.long	.LASF304
	.byte	0x4
	.value	0x171
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x18e8
	.uleb128 0x13
	.long	.LASF305
	.byte	0x4
	.value	0x174
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x18ec
	.byte	0x0
	.uleb128 0xa
	.long	0x54
	.long	0x11be
	.uleb128 0x1e
	.long	0x1eb
	.value	0x18f
	.byte	0x0
	.uleb128 0xa
	.long	0x66
	.long	0x11cf
	.uleb128 0x1e
	.long	0x1eb
	.value	0x17f
	.byte	0x0
	.uleb128 0xa
	.long	0x26c
	.long	0x11df
	.uleb128 0xb
	.long	0x1eb
	.byte	0x18
	.byte	0x0
	.uleb128 0xa
	.long	0xe16
	.long	0x11ef
	.uleb128 0xb
	.long	0x1eb
	.byte	0x18
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0xd2e
	.uleb128 0xd
	.byte	0x8
	.long	0x9e0
	.uleb128 0xa
	.long	0x77f
	.long	0x120b
	.uleb128 0xb
	.long	0x1eb
	.byte	0x2
	.byte	0x0
	.uleb128 0xa
	.long	0x4d
	.long	0x1221
	.uleb128 0xb
	.long	0x1eb
	.byte	0x3
	.uleb128 0xb
	.long	0x1eb
	.byte	0x5
	.byte	0x0
	.uleb128 0xa
	.long	0x4d
	.long	0x1231
	.uleb128 0xb
	.long	0x1eb
	.byte	0x3
	.byte	0x0
	.uleb128 0xa
	.long	0x66
	.long	0x1242
	.uleb128 0x1e
	.long	0x1eb
	.value	0x2bf
	.byte	0x0
	.uleb128 0x12
	.long	.LASF306
	.value	0x240
	.byte	0x4
	.value	0x16e
	.long	0x12a9
	.uleb128 0x9
	.long	.LASF307
	.byte	0x15
	.byte	0x7b
	.long	0x15a2
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF308
	.byte	0x15
	.byte	0x7c
	.long	0x178b
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x9
	.long	.LASF309
	.byte	0x15
	.byte	0x7d
	.long	0x95d
	.byte	0x3
	.byte	0x23
	.uleb128 0xe0
	.uleb128 0x9
	.long	.LASF310
	.byte	0x15
	.byte	0x7e
	.long	0x14a4
	.byte	0x3
	.byte	0x23
	.uleb128 0x1c0
	.uleb128 0x9
	.long	.LASF311
	.byte	0x15
	.byte	0x80
	.long	0x1903
	.byte	0x3
	.byte	0x23
	.uleb128 0x200
	.uleb128 0x9
	.long	.LASF132
	.byte	0x15
	.byte	0x82
	.long	0x46
	.byte	0x3
	.byte	0x23
	.uleb128 0x238
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x1242
	.uleb128 0x16
	.long	.LASF312
	.byte	0x4
	.value	0x177
	.long	0xe5f
	.uleb128 0xa
	.long	0x66
	.long	0x12cb
	.uleb128 0xb
	.long	0x1eb
	.byte	0xff
	.byte	0x0
	.uleb128 0x1a
	.byte	0x1
	.long	0x12e1
	.uleb128 0x1b
	.long	0xe0a
	.uleb128 0x1b
	.long	0xe0a
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x12cb
	.uleb128 0xd
	.byte	0x8
	.long	0xe16
	.uleb128 0x11
	.byte	0x30
	.byte	0x5
	.byte	0x18
	.long	0x1366
	.uleb128 0x9
	.long	.LASF313
	.byte	0x5
	.byte	0x19
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF314
	.byte	0x5
	.byte	0x1a
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.long	.LASF315
	.byte	0x5
	.byte	0x1b
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF316
	.byte	0x5
	.byte	0x1c
	.long	0x46
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x8
	.string	"pos"
	.byte	0x5
	.byte	0x1d
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF317
	.byte	0x5
	.byte	0x1e
	.long	0x1ee
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF318
	.byte	0x5
	.byte	0x21
	.long	0x7f
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF319
	.byte	0x5
	.byte	0x22
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x3
	.long	.LASF320
	.byte	0x5
	.byte	0x23
	.long	0x12ed
	.uleb128 0x11
	.byte	0x20
	.byte	0x16
	.byte	0x2d
	.long	0x13b2
	.uleb128 0x9
	.long	.LASF321
	.byte	0x16
	.byte	0x2e
	.long	0x13b2
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF322
	.byte	0x16
	.byte	0x2f
	.long	0x13b2
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x8
	.string	"lim"
	.byte	0x16
	.byte	0x30
	.long	0x13b2
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF323
	.byte	0x16
	.byte	0x31
	.long	0x13b2
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x13b8
	.uleb128 0x10
	.long	0x66
	.uleb128 0x3
	.long	.LASF324
	.byte	0x16
	.byte	0x32
	.long	0x1371
	.uleb128 0x3
	.long	.LASF325
	.byte	0x16
	.byte	0x70
	.long	0x13d3
	.uleb128 0xd
	.byte	0x8
	.long	0x13d9
	.uleb128 0x1a
	.byte	0x1
	.long	0x13fe
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x13fe
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x13bd
	.uleb128 0x3
	.long	.LASF326
	.byte	0x16
	.byte	0x71
	.long	0x140f
	.uleb128 0xd
	.byte	0x8
	.long	0x1415
	.uleb128 0x1a
	.byte	0x1
	.long	0x142b
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x13b2
	.byte	0x0
	.uleb128 0x11
	.byte	0x40
	.byte	0x16
	.byte	0x73
	.long	0x14a4
	.uleb128 0x9
	.long	.LASF327
	.byte	0x16
	.byte	0x74
	.long	0x13c8
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF328
	.byte	0x16
	.byte	0x75
	.long	0x13c8
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF329
	.byte	0x16
	.byte	0x76
	.long	0x13c8
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF330
	.byte	0x16
	.byte	0x77
	.long	0x13c8
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF331
	.byte	0x16
	.byte	0x78
	.long	0x1404
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF332
	.byte	0x16
	.byte	0x79
	.long	0x1404
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF333
	.byte	0x16
	.byte	0x7a
	.long	0x1404
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x9
	.long	.LASF334
	.byte	0x16
	.byte	0x7b
	.long	0x1404
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.byte	0x0
	.uleb128 0x3
	.long	.LASF335
	.byte	0x16
	.byte	0x7c
	.long	0x142b
	.uleb128 0x3
	.long	.LASF336
	.byte	0x17
	.byte	0x6b
	.long	0x12e1
	.uleb128 0x3
	.long	.LASF337
	.byte	0x17
	.byte	0x6c
	.long	0x14c5
	.uleb128 0xd
	.byte	0x8
	.long	0x14cb
	.uleb128 0x1a
	.byte	0x1
	.long	0x14eb
	.uleb128 0x1b
	.long	0x54
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF338
	.byte	0x17
	.byte	0x6d
	.long	0x14f6
	.uleb128 0xd
	.byte	0x8
	.long	0x14fc
	.uleb128 0x1a
	.byte	0x1
	.long	0x150d
	.uleb128 0x1b
	.long	0xe0a
	.uleb128 0x1b
	.long	0xe0a
	.byte	0x0
	.uleb128 0x11
	.byte	0x50
	.byte	0x17
	.byte	0x6f
	.long	0x15a2
	.uleb128 0x9
	.long	.LASF339
	.byte	0x17
	.byte	0x70
	.long	0x14af
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF340
	.byte	0x17
	.byte	0x71
	.long	0x14af
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF341
	.byte	0x17
	.byte	0x72
	.long	0x14ba
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF342
	.byte	0x17
	.byte	0x74
	.long	0x14eb
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF343
	.byte	0x17
	.byte	0x75
	.long	0x14eb
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF344
	.byte	0x17
	.byte	0x77
	.long	0x14af
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF345
	.byte	0x17
	.byte	0x78
	.long	0x14af
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x9
	.long	.LASF346
	.byte	0x17
	.byte	0x79
	.long	0x14ba
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x9
	.long	.LASF347
	.byte	0x17
	.byte	0x7a
	.long	0x14af
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x9
	.long	.LASF348
	.byte	0x17
	.byte	0x7b
	.long	0x14af
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.byte	0x0
	.uleb128 0x3
	.long	.LASF349
	.byte	0x17
	.byte	0x7c
	.long	0x150d
	.uleb128 0x3
	.long	.LASF350
	.byte	0x18
	.byte	0xc5
	.long	0x15b8
	.uleb128 0xd
	.byte	0x8
	.long	0x15be
	.uleb128 0x1a
	.byte	0x1
	.long	0x15d9
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF351
	.byte	0x18
	.byte	0xc6
	.long	0x15e4
	.uleb128 0xd
	.byte	0x8
	.long	0x15ea
	.uleb128 0x1a
	.byte	0x1
	.long	0x1605
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0xe0a
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF352
	.byte	0x18
	.byte	0xc7
	.long	0x1610
	.uleb128 0xd
	.byte	0x8
	.long	0x1616
	.uleb128 0x1a
	.byte	0x1
	.long	0x1627
	.uleb128 0x1b
	.long	0x1627
	.uleb128 0x1b
	.long	0x1741
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x162d
	.uleb128 0x1f
	.long	.LASF353
	.long	0x1636
	.uleb128 0x7
	.long	.LASF353
	.byte	0x90
	.byte	0x18
	.byte	0x25
	.long	0x1741
	.uleb128 0x9
	.long	.LASF354
	.byte	0x18
	.byte	0xce
	.long	0x15ad
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF355
	.byte	0x18
	.byte	0xcf
	.long	0x15ad
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF356
	.byte	0x18
	.byte	0xd0
	.long	0x15ad
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF357
	.byte	0x18
	.byte	0xd1
	.long	0x15ad
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF358
	.byte	0x18
	.byte	0xd2
	.long	0x15ad
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF308
	.byte	0x18
	.byte	0xd3
	.long	0x15d9
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF359
	.byte	0x18
	.byte	0xd4
	.long	0x15d9
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x9
	.long	.LASF360
	.byte	0x18
	.byte	0xd5
	.long	0x15d9
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x9
	.long	.LASF361
	.byte	0x18
	.byte	0xd6
	.long	0x15d9
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x9
	.long	.LASF362
	.byte	0x18
	.byte	0xd7
	.long	0x1605
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x9
	.long	.LASF363
	.byte	0x18
	.byte	0xd8
	.long	0x1605
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x9
	.long	.LASF364
	.byte	0x18
	.byte	0xd9
	.long	0x1747
	.byte	0x2
	.byte	0x23
	.uleb128 0x58
	.uleb128 0x9
	.long	.LASF365
	.byte	0x18
	.byte	0xda
	.long	0x1747
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.uleb128 0x9
	.long	.LASF366
	.byte	0x18
	.byte	0xde
	.long	0x1747
	.byte	0x2
	.byte	0x23
	.uleb128 0x68
	.uleb128 0x9
	.long	.LASF367
	.byte	0x18
	.byte	0xdf
	.long	0x1747
	.byte	0x2
	.byte	0x23
	.uleb128 0x70
	.uleb128 0x9
	.long	.LASF368
	.byte	0x18
	.byte	0xe3
	.long	0x1764
	.byte	0x2
	.byte	0x23
	.uleb128 0x78
	.uleb128 0x9
	.long	.LASF369
	.byte	0x18
	.byte	0xe7
	.long	0x1764
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x9
	.long	.LASF370
	.byte	0x18
	.byte	0xeb
	.long	0x1764
	.byte	0x3
	.byte	0x23
	.uleb128 0x88
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x12af
	.uleb128 0x3
	.long	.LASF371
	.byte	0x18
	.byte	0xc8
	.long	0x1752
	.uleb128 0xd
	.byte	0x8
	.long	0x1758
	.uleb128 0x1a
	.byte	0x1
	.long	0x1764
	.uleb128 0x1b
	.long	0x1741
	.byte	0x0
	.uleb128 0x3
	.long	.LASF372
	.byte	0x18
	.byte	0xc9
	.long	0x176f
	.uleb128 0xd
	.byte	0x8
	.long	0x1775
	.uleb128 0x1a
	.byte	0x1
	.long	0x178b
	.uleb128 0x1b
	.long	0x12e7
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x1ee
	.byte	0x0
	.uleb128 0x3
	.long	.LASF373
	.byte	0x18
	.byte	0xef
	.long	0x1636
	.uleb128 0x3
	.long	.LASF374
	.byte	0x19
	.byte	0x4e
	.long	0x17a1
	.uleb128 0xd
	.byte	0x8
	.long	0x17a7
	.uleb128 0x1a
	.byte	0x1
	.long	0x17d1
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF375
	.byte	0x19
	.byte	0x4f
	.long	0x17dc
	.uleb128 0xd
	.byte	0x8
	.long	0x17e2
	.uleb128 0x1a
	.byte	0x1
	.long	0x1802
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF376
	.byte	0x19
	.byte	0x50
	.long	0x180d
	.uleb128 0xd
	.byte	0x8
	.long	0x1813
	.uleb128 0x1a
	.byte	0x1
	.long	0x1842
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x2fb
	.uleb128 0x1b
	.long	0x2fb
	.uleb128 0x1b
	.long	0x2fb
	.uleb128 0x1b
	.long	0x2fb
	.uleb128 0x1b
	.long	0x7f
	.uleb128 0x1b
	.long	0x7f
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF377
	.byte	0x19
	.byte	0x51
	.long	0x184d
	.uleb128 0xd
	.byte	0x8
	.long	0x1853
	.uleb128 0x1a
	.byte	0x1
	.long	0x1882
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x1ee
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.uleb128 0x1b
	.long	0x46
	.byte	0x0
	.uleb128 0x3
	.long	.LASF378
	.byte	0x19
	.byte	0x52
	.long	0x184d
	.uleb128 0x3
	.long	.LASF379
	.byte	0x19
	.byte	0x53
	.long	0x184d
	.uleb128 0x11
	.byte	0x38
	.byte	0x19
	.byte	0x54
	.long	0x1903
	.uleb128 0x9
	.long	.LASF380
	.byte	0x19
	.byte	0x55
	.long	0x17d1
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.long	.LASF381
	.byte	0x19
	.byte	0x56
	.long	0x17d1
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.long	.LASF382
	.byte	0x19
	.byte	0x57
	.long	0x1796
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x9
	.long	.LASF383
	.byte	0x19
	.byte	0x58
	.long	0x1802
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.long	.LASF384
	.byte	0x19
	.byte	0x59
	.long	0x1842
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.long	.LASF385
	.byte	0x19
	.byte	0x5a
	.long	0x1882
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.long	.LASF386
	.byte	0x19
	.byte	0x5b
	.long	0x188d
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.byte	0x0
	.uleb128 0x3
	.long	.LASF387
	.byte	0x19
	.byte	0x5c
	.long	0x1898
	.uleb128 0xd
	.byte	0x8
	.long	0x774
	.uleb128 0x20
	.long	.LASF445
	.byte	0x1
	.value	0x1b3
	.byte	0x1
	.quad	.LFB0
	.quad	.LFE0
	.long	.LLST0
	.long	0x1997
	.uleb128 0x21
	.long	.LASF388
	.byte	0x1
	.value	0x1b3
	.long	0x1997
	.byte	0x3
	.byte	0x91
	.sleb128 -248
	.uleb128 0x21
	.long	.LASF389
	.byte	0x1
	.value	0x1b4
	.long	0x256
	.byte	0x3
	.byte	0x91
	.sleb128 -252
	.uleb128 0x22
	.string	"fmt"
	.byte	0x1
	.value	0x1b5
	.long	0x261
	.byte	0x3
	.byte	0x91
	.sleb128 -264
	.uleb128 0x23
	.uleb128 0x24
	.string	"ap"
	.byte	0x1
	.value	0x1b7
	.long	0x2ea
	.byte	0x3
	.byte	0x91
	.sleb128 -240
	.uleb128 0x25
	.quad	.LBB2
	.quad	.LBE2
	.uleb128 0x24
	.string	"sz"
	.byte	0x1
	.value	0x1bd
	.long	0x34
	.byte	0x3
	.byte	0x91
	.sleb128 -216
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x3a0
	.uleb128 0x26
	.long	.LASF391
	.byte	0x2
	.byte	0x69
	.byte	0x1
	.long	0x19d0
	.quad	.LFB1
	.quad	.LFE1
	.long	.LLST1
	.long	0x19d0
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x69
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x49f
	.uleb128 0x26
	.long	.LASF392
	.byte	0x2
	.byte	0x69
	.byte	0x1
	.long	0x19d0
	.quad	.LFB2
	.quad	.LFE2
	.long	.LLST2
	.long	0x1a2e
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x69
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x69
	.long	0x1a21
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x69
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x69
	.long	0x19d0
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x69
	.long	0x1a06
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF393
	.byte	0x2
	.byte	0x6a
	.byte	0x1
	.long	0x19d0
	.quad	.LFB3
	.quad	.LFE3
	.long	.LLST3
	.long	0x1a61
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x6a
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x26
	.long	.LASF394
	.byte	0x2
	.byte	0x6a
	.byte	0x1
	.long	0x19d0
	.quad	.LFB4
	.quad	.LFE4
	.long	.LLST4
	.long	0x1ab9
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x6a
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x6a
	.long	0x1aac
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x6a
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x6a
	.long	0x19d0
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x6a
	.long	0x1a91
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF395
	.byte	0x2
	.byte	0x6b
	.byte	0x1
	.long	0x1aec
	.quad	.LFB5
	.quad	.LFE5
	.long	.LLST5
	.long	0x1aec
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x6b
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x441
	.uleb128 0x26
	.long	.LASF396
	.byte	0x2
	.byte	0x6b
	.byte	0x1
	.long	0x1aec
	.quad	.LFB6
	.quad	.LFE6
	.long	.LLST6
	.long	0x1b4a
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x6b
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x6b
	.long	0x1b3d
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x6b
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x6b
	.long	0x1aec
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x6b
	.long	0x1b22
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF397
	.byte	0x2
	.byte	0x6c
	.byte	0x1
	.long	0x46
	.quad	.LFB7
	.quad	.LFE7
	.long	.LLST7
	.long	0x1b7d
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x6c
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x26
	.long	.LASF398
	.byte	0x2
	.byte	0x6c
	.byte	0x1
	.long	0x46
	.quad	.LFB8
	.quad	.LFE8
	.long	.LLST8
	.long	0x1bd5
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x6c
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x6c
	.long	0x1bc8
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x6c
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x6c
	.long	0x46
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x6c
	.long	0x1bad
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF399
	.byte	0x2
	.byte	0x6d
	.byte	0x1
	.long	0x46
	.quad	.LFB9
	.quad	.LFE9
	.long	.LLST9
	.long	0x1c08
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x6d
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x26
	.long	.LASF400
	.byte	0x2
	.byte	0x6d
	.byte	0x1
	.long	0x46
	.quad	.LFB10
	.quad	.LFE10
	.long	.LLST10
	.long	0x1c60
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x6d
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x6d
	.long	0x1c53
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x6d
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x6d
	.long	0x46
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x6d
	.long	0x1c38
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF401
	.byte	0x2
	.byte	0x6e
	.byte	0x1
	.long	0x46
	.quad	.LFB11
	.quad	.LFE11
	.long	.LLST11
	.long	0x1c93
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x6e
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x26
	.long	.LASF402
	.byte	0x2
	.byte	0x6e
	.byte	0x1
	.long	0x46
	.quad	.LFB12
	.quad	.LFE12
	.long	.LLST12
	.long	0x1ceb
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x6e
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x6e
	.long	0x1cde
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x6e
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x6e
	.long	0x46
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x6e
	.long	0x1cc3
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF403
	.byte	0x2
	.byte	0x6f
	.byte	0x1
	.long	0x46
	.quad	.LFB13
	.quad	.LFE13
	.long	.LLST13
	.long	0x1d1e
	.uleb128 0x27
	.long	.LASF390
	.byte	0x2
	.byte	0x6f
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x26
	.long	.LASF404
	.byte	0x2
	.byte	0x6f
	.byte	0x1
	.long	0x46
	.quad	.LFB14
	.quad	.LFE14
	.long	.LLST14
	.long	0x1d76
	.uleb128 0x28
	.string	"x"
	.byte	0x2
	.byte	0x6f
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.byte	0x8
	.byte	0x2
	.byte	0x6f
	.long	0x1d69
	.uleb128 0x29
	.string	"x"
	.byte	0x2
	.byte	0x6f
	.long	0x204
	.uleb128 0x29
	.string	"d"
	.byte	0x2
	.byte	0x6f
	.long	0x46
	.byte	0x0
	.uleb128 0x2a
	.string	"u"
	.byte	0x2
	.byte	0x6f
	.long	0x1d4e
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF405
	.byte	0x3
	.value	0x122
	.byte	0x1
	.long	0x46
	.quad	.LFB15
	.quad	.LFE15
	.long	.LLST15
	.long	0x1dab
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x122
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF406
	.byte	0x3
	.value	0x122
	.byte	0x1
	.long	0x46
	.quad	.LFB16
	.quad	.LFE16
	.long	.LLST16
	.long	0x1e09
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x122
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x122
	.long	0x1dfb
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x122
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x122
	.long	0x46
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x122
	.long	0x1ddd
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF407
	.byte	0x3
	.value	0x123
	.byte	0x1
	.long	0x46
	.quad	.LFB17
	.quad	.LFE17
	.long	.LLST17
	.long	0x1e3e
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x123
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF408
	.byte	0x3
	.value	0x123
	.byte	0x1
	.long	0x46
	.quad	.LFB18
	.quad	.LFE18
	.long	.LLST18
	.long	0x1e9c
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x123
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x123
	.long	0x1e8e
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x123
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x123
	.long	0x46
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x123
	.long	0x1e70
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF409
	.byte	0x3
	.value	0x124
	.byte	0x1
	.long	0x46
	.quad	.LFB19
	.quad	.LFE19
	.long	.LLST19
	.long	0x1ed1
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x124
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF410
	.byte	0x3
	.value	0x124
	.byte	0x1
	.long	0x46
	.quad	.LFB20
	.quad	.LFE20
	.long	.LLST20
	.long	0x1f2f
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x124
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x124
	.long	0x1f21
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x124
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x124
	.long	0x46
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x124
	.long	0x1f03
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF411
	.byte	0x3
	.value	0x126
	.byte	0x1
	.long	0x1f64
	.quad	.LFB21
	.quad	.LFE21
	.long	.LLST21
	.long	0x1f64
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x126
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x53b
	.uleb128 0x2b
	.long	.LASF412
	.byte	0x3
	.value	0x126
	.byte	0x1
	.long	0x1f64
	.quad	.LFB22
	.quad	.LFE22
	.long	.LLST22
	.long	0x1fc8
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x126
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x126
	.long	0x1fba
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x126
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x126
	.long	0x1f64
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x126
	.long	0x1f9c
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF413
	.byte	0x3
	.value	0x127
	.byte	0x1
	.long	0x1ffd
	.quad	.LFB23
	.quad	.LFE23
	.long	.LLST23
	.long	0x1ffd
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x127
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x57d
	.uleb128 0x2b
	.long	.LASF414
	.byte	0x3
	.value	0x127
	.byte	0x1
	.long	0x1ffd
	.quad	.LFB24
	.quad	.LFE24
	.long	.LLST24
	.long	0x2061
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x127
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x127
	.long	0x2053
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x127
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x127
	.long	0x1ffd
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x127
	.long	0x2035
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF415
	.byte	0x3
	.value	0x128
	.byte	0x1
	.long	0x2096
	.quad	.LFB25
	.quad	.LFE25
	.long	.LLST25
	.long	0x2096
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x128
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x5b1
	.uleb128 0x2b
	.long	.LASF416
	.byte	0x3
	.value	0x128
	.byte	0x1
	.long	0x2096
	.quad	.LFB26
	.quad	.LFE26
	.long	.LLST26
	.long	0x20fa
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x128
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x128
	.long	0x20ec
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x128
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x128
	.long	0x2096
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x128
	.long	0x20ce
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF417
	.byte	0x3
	.value	0x12a
	.byte	0x1
	.long	0x46
	.quad	.LFB27
	.quad	.LFE27
	.long	.LLST27
	.long	0x212f
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x12a
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF418
	.byte	0x3
	.value	0x12a
	.byte	0x1
	.long	0x46
	.quad	.LFB28
	.quad	.LFE28
	.long	.LLST28
	.long	0x218d
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x12a
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x12a
	.long	0x217f
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x12a
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x12a
	.long	0x46
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x12a
	.long	0x2161
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF419
	.byte	0x3
	.value	0x12b
	.byte	0x1
	.long	0x7f
	.quad	.LFB29
	.quad	.LFE29
	.long	.LLST29
	.long	0x21c2
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x12b
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF420
	.byte	0x3
	.value	0x12b
	.byte	0x1
	.long	0x7f
	.quad	.LFB30
	.quad	.LFE30
	.long	.LLST30
	.long	0x2220
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x12b
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x12b
	.long	0x2212
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x12b
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x12b
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x12b
	.long	0x21f4
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF421
	.byte	0x3
	.value	0x12c
	.byte	0x1
	.long	0x7f
	.quad	.LFB31
	.quad	.LFE31
	.long	.LLST31
	.long	0x2255
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x12c
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF422
	.byte	0x3
	.value	0x12c
	.byte	0x1
	.long	0x7f
	.quad	.LFB32
	.quad	.LFE32
	.long	.LLST32
	.long	0x22b3
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x12c
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x12c
	.long	0x22a5
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x12c
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x12c
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x12c
	.long	0x2287
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF423
	.byte	0x3
	.value	0x12d
	.byte	0x1
	.long	0x7f
	.quad	.LFB33
	.quad	.LFE33
	.long	.LLST33
	.long	0x22e8
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x12d
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF424
	.byte	0x3
	.value	0x12d
	.byte	0x1
	.long	0x7f
	.quad	.LFB34
	.quad	.LFE34
	.long	.LLST34
	.long	0x2346
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x12d
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x12d
	.long	0x2338
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x12d
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x12d
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x12d
	.long	0x231a
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF425
	.byte	0x3
	.value	0x12e
	.byte	0x1
	.long	0x7f
	.quad	.LFB35
	.quad	.LFE35
	.long	.LLST35
	.long	0x237b
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x12e
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF426
	.byte	0x3
	.value	0x12e
	.byte	0x1
	.long	0x7f
	.quad	.LFB36
	.quad	.LFE36
	.long	.LLST36
	.long	0x23d9
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x12e
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x12e
	.long	0x23cb
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x12e
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x12e
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x12e
	.long	0x23ad
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF427
	.byte	0x3
	.value	0x12f
	.byte	0x1
	.long	0x5de
	.quad	.LFB37
	.quad	.LFE37
	.long	.LLST37
	.long	0x240e
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x12f
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF428
	.byte	0x3
	.value	0x12f
	.byte	0x1
	.long	0x5de
	.quad	.LFB38
	.quad	.LFE38
	.long	.LLST38
	.long	0x246c
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x12f
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x12f
	.long	0x245e
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x12f
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x12f
	.long	0x5de
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x12f
	.long	0x2440
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF429
	.byte	0x3
	.value	0x131
	.byte	0x1
	.long	0x7f
	.quad	.LFB39
	.quad	.LFE39
	.long	.LLST39
	.long	0x24a1
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x131
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF430
	.byte	0x3
	.value	0x131
	.byte	0x1
	.long	0x7f
	.quad	.LFB40
	.quad	.LFE40
	.long	.LLST40
	.long	0x24ff
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x131
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x131
	.long	0x24f1
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x131
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x131
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x131
	.long	0x24d3
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF431
	.byte	0x3
	.value	0x132
	.byte	0x1
	.long	0x7f
	.quad	.LFB41
	.quad	.LFE41
	.long	.LLST41
	.long	0x2534
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x132
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF432
	.byte	0x3
	.value	0x132
	.byte	0x1
	.long	0x7f
	.quad	.LFB42
	.quad	.LFE42
	.long	.LLST42
	.long	0x2592
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x132
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x132
	.long	0x2584
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x132
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x132
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x132
	.long	0x2566
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF433
	.byte	0x3
	.value	0x133
	.byte	0x1
	.long	0x7f
	.quad	.LFB43
	.quad	.LFE43
	.long	.LLST43
	.long	0x25c7
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x133
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF434
	.byte	0x3
	.value	0x133
	.byte	0x1
	.long	0x7f
	.quad	.LFB44
	.quad	.LFE44
	.long	.LLST44
	.long	0x2625
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x133
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x133
	.long	0x2617
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x133
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x133
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x133
	.long	0x25f9
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF435
	.byte	0x3
	.value	0x134
	.byte	0x1
	.long	0x600
	.quad	.LFB45
	.quad	.LFE45
	.long	.LLST45
	.long	0x265a
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x134
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF436
	.byte	0x3
	.value	0x134
	.byte	0x1
	.long	0x600
	.quad	.LFB46
	.quad	.LFE46
	.long	.LLST46
	.long	0x26b8
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x134
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x134
	.long	0x26aa
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x134
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x134
	.long	0x600
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x134
	.long	0x268c
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF437
	.byte	0x3
	.value	0x135
	.byte	0x1
	.long	0x7f
	.quad	.LFB47
	.quad	.LFE47
	.long	.LLST47
	.long	0x26ed
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x135
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF438
	.byte	0x3
	.value	0x135
	.byte	0x1
	.long	0x7f
	.quad	.LFB48
	.quad	.LFE48
	.long	.LLST48
	.long	0x274b
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x135
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x135
	.long	0x273d
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x135
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x135
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x135
	.long	0x271f
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF439
	.byte	0x3
	.value	0x137
	.byte	0x1
	.long	0x720
	.quad	.LFB49
	.quad	.LFE49
	.long	.LLST49
	.long	0x2780
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x137
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF440
	.byte	0x3
	.value	0x137
	.byte	0x1
	.long	0x720
	.quad	.LFB50
	.quad	.LFE50
	.long	.LLST50
	.long	0x27de
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x137
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x137
	.long	0x27d0
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x137
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x137
	.long	0x720
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x137
	.long	0x27b2
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF441
	.byte	0x3
	.value	0x138
	.byte	0x1
	.long	0x720
	.quad	.LFB51
	.quad	.LFE51
	.long	.LLST51
	.long	0x2813
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x138
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF442
	.byte	0x3
	.value	0x138
	.byte	0x1
	.long	0x720
	.quad	.LFB52
	.quad	.LFE52
	.long	.LLST52
	.long	0x2871
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x138
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x138
	.long	0x2863
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x138
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x138
	.long	0x720
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x138
	.long	0x2845
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF443
	.byte	0x3
	.value	0x13a
	.byte	0x1
	.long	0x7f
	.quad	.LFB53
	.quad	.LFE53
	.long	.LLST53
	.long	0x28a6
	.uleb128 0x21
	.long	.LASF390
	.byte	0x3
	.value	0x13a
	.long	0x2f5
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0x0
	.uleb128 0x2b
	.long	.LASF444
	.byte	0x3
	.value	0x13a
	.byte	0x1
	.long	0x7f
	.quad	.LFB54
	.quad	.LFE54
	.long	.LLST54
	.long	0x2904
	.uleb128 0x22
	.string	"x"
	.byte	0x3
	.value	0x13a
	.long	0x204
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2c
	.byte	0x8
	.byte	0x3
	.value	0x13a
	.long	0x28f6
	.uleb128 0x2d
	.string	"x"
	.byte	0x3
	.value	0x13a
	.long	0x204
	.uleb128 0x2d
	.string	"d"
	.byte	0x3
	.value	0x13a
	.long	0x7f
	.byte	0x0
	.uleb128 0x24
	.string	"u"
	.byte	0x3
	.value	0x13a
	.long	0x28d8
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x20
	.long	.LASF446
	.byte	0x4
	.value	0x17d
	.byte	0x1
	.quad	.LFB56
	.quad	.LFE56
	.long	.LLST55
	.long	0x2950
	.uleb128 0x22
	.string	"xd"
	.byte	0x4
	.value	0x17d
	.long	0x1741
	.byte	0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x24
	.string	"i"
	.byte	0x4
	.value	0x17e
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x2e
	.long	.LASF447
	.byte	0x4
	.value	0x17f
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2f
	.long	.LASF448
	.byte	0x5
	.byte	0x36
	.byte	0x1
	.quad	.LFB57
	.quad	.LFE57
	.long	.LLST56
	.long	0x2a1f
	.uleb128 0x28
	.string	"br"
	.byte	0x5
	.byte	0x36
	.long	0x2a1f
	.byte	0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x28
	.string	"bit"
	.byte	0x5
	.byte	0x36
	.long	0x46
	.byte	0x3
	.byte	0x91
	.sleb128 -68
	.uleb128 0x27
	.long	.LASF449
	.byte	0x5
	.byte	0x36
	.long	0x46
	.byte	0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0x30
	.long	.LASF450
	.byte	0x5
	.byte	0x37
	.long	0x7f
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x30
	.long	.LASF316
	.byte	0x5
	.byte	0x38
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x30
	.long	.LASF314
	.byte	0x5
	.byte	0x39
	.long	0x7f
	.byte	0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x30
	.long	.LASF313
	.byte	0x5
	.byte	0x3a
	.long	0x7f
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x30
	.long	.LASF451
	.byte	0x5
	.byte	0x3b
	.long	0x7f
	.byte	0x1
	.byte	0x53
	.uleb128 0x25
	.quad	.LBB3
	.quad	.LBE3
	.uleb128 0x30
	.long	.LASF452
	.byte	0x5
	.byte	0x57
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -44
	.uleb128 0x25
	.quad	.LBB4
	.quad	.LBE4
	.uleb128 0x2a
	.string	"x"
	.byte	0x5
	.byte	0x5a
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -48
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x1366
	.uleb128 0x2f
	.long	.LASF453
	.byte	0x6
	.byte	0x16
	.byte	0x1
	.quad	.LFB64
	.quad	.LFE64
	.long	.LLST57
	.long	0x2a8c
	.uleb128 0x27
	.long	.LASF454
	.byte	0x6
	.byte	0x16
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x27
	.long	.LASF455
	.byte	0x6
	.byte	0x16
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x28
	.string	"mvp"
	.byte	0x6
	.byte	0x16
	.long	0x190e
	.byte	0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x27
	.long	.LASF456
	.byte	0x6
	.byte	0x16
	.long	0x96f
	.byte	0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x2a
	.string	"xmv"
	.byte	0x6
	.byte	0x17
	.long	0x74b
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x2f
	.long	.LASF457
	.byte	0x6
	.byte	0x24
	.byte	0x1
	.quad	.LFB65
	.quad	.LFE65
	.long	.LLST58
	.long	0x2ac7
	.uleb128 0x28
	.string	"mv"
	.byte	0x6
	.byte	0x24
	.long	0x190e
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"xd"
	.byte	0x6
	.byte	0x24
	.long	0x2ac7
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x2acd
	.uleb128 0x10
	.long	0x12af
	.uleb128 0x2f
	.long	.LASF458
	.byte	0x6
	.byte	0x30
	.byte	0x1
	.quad	.LFB66
	.quad	.LFE66
	.long	.LLST59
	.long	0x2b38
	.uleb128 0x28
	.string	"mv"
	.byte	0x6
	.byte	0x30
	.long	0x190e
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x27
	.long	.LASF286
	.byte	0x6
	.byte	0x30
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x27
	.long	.LASF287
	.byte	0x6
	.byte	0x30
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x27
	.long	.LASF288
	.byte	0x6
	.byte	0x31
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x27
	.long	.LASF289
	.byte	0x6
	.byte	0x31
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.byte	0x0
	.uleb128 0x26
	.long	.LASF459
	.byte	0x6
	.byte	0x3b
	.byte	0x1
	.long	0x7f
	.quad	.LFB67
	.quad	.LFE67
	.long	.LLST60
	.long	0x2bb0
	.uleb128 0x28
	.string	"mv"
	.byte	0x6
	.byte	0x3b
	.long	0x190e
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x27
	.long	.LASF286
	.byte	0x6
	.byte	0x3b
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -44
	.uleb128 0x27
	.long	.LASF287
	.byte	0x6
	.byte	0x3c
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x27
	.long	.LASF288
	.byte	0x6
	.byte	0x3c
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -52
	.uleb128 0x27
	.long	.LASF289
	.byte	0x6
	.byte	0x3d
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x30
	.long	.LASF460
	.byte	0x6
	.byte	0x3e
	.long	0x7f
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.byte	0x0
	.uleb128 0x26
	.long	.LASF461
	.byte	0x6
	.byte	0x58
	.byte	0x1
	.long	0x46
	.quad	.LFB68
	.quad	.LFE68
	.long	.LLST61
	.long	0x2bef
	.uleb128 0x27
	.long	.LASF462
	.byte	0x6
	.byte	0x58
	.long	0x2bef
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"b"
	.byte	0x6
	.byte	0x58
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.byte	0x0
	.uleb128 0xd
	.byte	0x8
	.long	0x2bf5
	.uleb128 0x10
	.long	0xd2e
	.uleb128 0x26
	.long	.LASF463
	.byte	0x6
	.byte	0x65
	.byte	0x1
	.long	0x46
	.quad	.LFB69
	.quad	.LFE69
	.long	.LLST62
	.long	0x2c39
	.uleb128 0x27
	.long	.LASF462
	.byte	0x6
	.byte	0x65
	.long	0x2bef
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"b"
	.byte	0x6
	.byte	0x65
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.byte	0x0
	.uleb128 0x26
	.long	.LASF464
	.byte	0x6
	.byte	0x72
	.byte	0x1
	.long	0x46
	.quad	.LFB70
	.quad	.LFE70
	.long	.LLST63
	.long	0x2c86
	.uleb128 0x27
	.long	.LASF462
	.byte	0x6
	.byte	0x72
	.long	0x2bef
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"b"
	.byte	0x6
	.byte	0x72
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x27
	.long	.LASF465
	.byte	0x6
	.byte	0x72
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF466
	.byte	0x6
	.byte	0x7f
	.byte	0x1
	.long	0x46
	.quad	.LFB71
	.quad	.LFE71
	.long	.LLST64
	.long	0x2cd3
	.uleb128 0x27
	.long	.LASF462
	.byte	0x6
	.byte	0x7f
	.long	0x2bef
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"b"
	.byte	0x6
	.byte	0x7f
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x27
	.long	.LASF465
	.byte	0x6
	.byte	0x7f
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x26
	.long	.LASF467
	.byte	0x6
	.byte	0x8c
	.byte	0x1
	.long	0xb40
	.quad	.LFB72
	.quad	.LFE72
	.long	.LLST65
	.long	0x2d12
	.uleb128 0x27
	.long	.LASF462
	.byte	0x6
	.byte	0x8c
	.long	0x2bef
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"b"
	.byte	0x6
	.byte	0x8c
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.byte	0x0
	.uleb128 0x26
	.long	.LASF468
	.byte	0x6
	.byte	0xa3
	.byte	0x1
	.long	0xb40
	.quad	.LFB73
	.quad	.LFE73
	.long	.LLST66
	.long	0x2d5f
	.uleb128 0x27
	.long	.LASF462
	.byte	0x6
	.byte	0xa4
	.long	0x2bef
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x28
	.string	"b"
	.byte	0x6
	.byte	0xa4
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x27
	.long	.LASF465
	.byte	0x6
	.byte	0xa4
	.long	0x46
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0x0
	.uleb128 0x31
	.byte	0x1
	.long	.LASF475
	.byte	0x7
	.byte	0x14
	.byte	0x1
	.long	0x46
	.quad	.LFB74
	.quad	.LFE74
	.long	.LLST67
	.uleb128 0xa
	.long	0x7f
	.long	0x2d90
	.uleb128 0xb
	.long	0x1eb
	.byte	0xff
	.byte	0x0
	.uleb128 0x32
	.long	.LASF469
	.byte	0x5
	.byte	0x29
	.long	0x2d9d
	.byte	0x1
	.byte	0x1
	.uleb128 0x10
	.long	0x2d80
	.uleb128 0x32
	.long	.LASF470
	.byte	0x5
	.byte	0x33
	.long	0x2daf
	.byte	0x1
	.byte	0x1
	.uleb128 0x10
	.long	0x12bb
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x8
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x9
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0xa
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xb
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xc
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xe
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xf
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x10
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x11
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x12
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x13
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x14
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x15
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x16
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x17
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x18
	.uleb128 0x17
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x19
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1a
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1b
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1c
	.uleb128 0x17
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1d
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1e
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0x1f
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x20
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x22
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x23
	.uleb128 0x18
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.uleb128 0x24
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x25
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x26
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x27
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x28
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x29
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2a
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x2b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2c
	.uleb128 0x17
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2d
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2e
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x2f
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x30
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x32
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.long	0x17
	.value	0x2
	.long	.Ldebug_info0
	.long	0x2db5
	.long	0x2d5f
	.string	"main"
	.long	0x0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0x0
	.value	0x0
	.value	0x0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0x0
	.quad	0x0
	.section	.debug_str,"MS",@progbits,1
.LASF273:
	.string	"segmentation_enabled"
.LASF85:
	.string	"vpx_ref_frame_t"
.LASF368:
	.string	"intra4x4_predict"
.LASF109:
	.string	"VP8_FOUR_TOKENPARTITION"
.LASF371:
	.string	"vp8_build_intra_pred_fn_t"
.LASF313:
	.string	"lowvalue"
.LASF215:
	.string	"ABOVE4X4"
.LASF206:
	.string	"B_VE_PRED"
.LASF394:
	.string	"VP8_COPY_REFERENCE__convert"
.LASF17:
	.string	"VPX_IMG_FMT_YVYU"
.LASF363:
	.string	"recon_mby"
.LASF151:
	.string	"eighttap_avg8x8_sharp"
.LASF91:
	.string	"VPX_SCALING_MODE"
.LASF54:
	.string	"gp_offset"
.LASF290:
	.string	"frames_since_golden"
.LASF278:
	.string	"segment_feature_data"
.LASF194:
	.string	"MB_PREDICTION_MODE"
.LASF308:
	.string	"recon"
.LASF42:
	.string	"VPX_CODEC_INCAPABLE"
.LASF12:
	.string	"VPX_IMG_FMT_RGB32"
.LASF174:
	.string	"INTER_FRAME"
.LASF342:
	.string	"iwalsh1"
.LASF406:
	.string	"VP8E_UPD_ENTROPY__convert"
.LASF320:
	.string	"BOOL_CODER"
.LASF331:
	.string	"simple_mb_v"
.LASF77:
	.string	"vpx_img_fmt"
.LASF405:
	.string	"VP8E_UPD_ENTROPY__value"
.LASF269:
	.string	"up_available"
.LASF158:
	.string	"sixtap_avg8x8"
.LASF317:
	.string	"buffer"
.LASF28:
	.string	"vpx_img_fmt_t"
.LASF454:
	.string	"refmb_ref_frame_sign_bias"
.LASF242:
	.string	"segment_id"
.LASF24:
	.string	"VPX_IMG_FMT_YV12"
.LASF260:
	.string	"BLOCKD"
.LASF281:
	.string	"mode_ref_lf_delta_update"
.LASF415:
	.string	"VP8E_SET_SCALEMODE__value"
.LASF240:
	.string	"need_to_clamp_mvs"
.LASF31:
	.string	"planes"
.LASF162:
	.string	"bilinear16x16"
.LASF235:
	.string	"second_ref_frame"
.LASF283:
	.string	"ref_lf_deltas"
.LASF101:
	.string	"active_map"
.LASF221:
	.string	"test"
.LASF426:
	.string	"VP8E_SET_STATIC_THRESHOLD__convert"
.LASF15:
	.string	"VPX_IMG_FMT_UYVY"
.LASF205:
	.string	"B_TM_PRED"
.LASF289:
	.string	"mb_to_bottom_edge"
.LASF10:
	.string	"VPX_IMG_FMT_NONE"
.LASF181:
	.string	"D117_PRED"
.LASF442:
	.string	"VP8E_GET_LAST_QUANTIZER_64__convert"
.LASF288:
	.string	"mb_to_top_edge"
.LASF21:
	.string	"VPX_IMG_FMT_ARGB_LE"
.LASF302:
	.string	"y_buf"
.LASF432:
	.string	"VP8E_SET_ARNR_STRENGTH__convert"
.LASF389:
	.string	"error"
.LASF427:
	.string	"VP8E_SET_TOKEN_PARTITIONS__value"
.LASF411:
	.string	"VP8E_SET_ROI_MAP__value"
.LASF238:
	.string	"partitioning"
.LASF185:
	.string	"TM_PRED"
.LASF428:
	.string	"VP8E_SET_TOKEN_PARTITIONS__convert"
.LASF291:
	.string	"frames_till_alt_ref_frame"
.LASF71:
	.string	"setjmp"
.LASF176:
	.string	"DC_PRED"
.LASF225:
	.string	"INTRA_FRAME"
.LASF103:
	.string	"vpx_scaling_mode"
.LASF93:
	.string	"roi_map"
.LASF444:
	.string	"VP8E_SET_MAX_INTRA_BITRATE_PCT__convert"
.LASF110:
	.string	"VP8_EIGHT_TOKENPARTITION"
.LASF155:
	.string	"sixtap16x16"
.LASF168:
	.string	"bilinear_avg4x4"
.LASF208:
	.string	"B_LD_PRED"
.LASF94:
	.string	"rows"
.LASF362:
	.string	"recon_mb"
.LASF2:
	.string	"signed char"
.LASF326:
	.string	"vp8_slf_block_fn_t"
.LASF286:
	.string	"mb_to_left_edge"
.LASF445:
	.string	"vpx_internal_error"
.LASF214:
	.string	"LEFT4X4"
.LASF8:
	.string	"uint32_t"
.LASF301:
	.string	"current_bc"
.LASF469:
	.string	"vp8_prob_cost"
.LASF380:
	.string	"down"
.LASF204:
	.string	"B_DC_PRED"
.LASF329:
	.string	"normal_mb_h"
.LASF381:
	.string	"across"
.LASF135:
	.string	"float"
.LASF451:
	.string	"shift"
.LASF448:
	.string	"vp8_encode_bool"
.LASF450:
	.string	"split"
.LASF440:
	.string	"VP8E_GET_LAST_QUANTIZER__convert"
.LASF458:
	.string	"vp8_clamp_mv"
.LASF210:
	.string	"B_VR_PRED"
.LASF430:
	.string	"VP8E_SET_ARNR_MAXFRAMES__convert"
.LASF316:
	.string	"count"
.LASF416:
	.string	"VP8E_SET_SCALEMODE__convert"
.LASF471:
	.string	"GNU C 4.4.3"
.LASF245:
	.string	"mb_in_image"
.LASF453:
	.string	"mv_bias"
.LASF156:
	.string	"sixtap8x8"
.LASF141:
	.string	"eighttap16x16"
.LASF333:
	.string	"simple_mb_h"
.LASF335:
	.string	"vp8_loopfilter_rtcd_vtable_t"
.LASF277:
	.string	"mb_segment_tree_probs"
.LASF56:
	.string	"overflow_arg_area"
.LASF213:
	.string	"B_HU_PRED"
.LASF170:
	.string	"long long unsigned int"
.LASF149:
	.string	"eighttap8x8_sharp"
.LASF107:
	.string	"VP8_ONE_TOKENPARTITION"
.LASF112:
	.string	"VP8_TUNE_PSNR"
.LASF117:
	.string	"YUV_TYPE"
.LASF396:
	.string	"VP8_SET_POSTPROC__convert"
.LASF323:
	.string	"hev_thr"
.LASF106:
	.string	"vpx_scaling_mode_t"
.LASF70:
	.string	"detail"
.LASF82:
	.string	"vpx_ref_frame_type_t"
.LASF59:
	.string	"__jmp_buf"
.LASF76:
	.string	"vp8_postproc_cfg_t"
.LASF393:
	.string	"VP8_COPY_REFERENCE__value"
.LASF148:
	.string	"eighttap16x16_sharp"
.LASF270:
	.string	"left_available"
.LASF46:
	.string	"VPX_CODEC_INVALID_PARAM"
.LASF128:
	.string	"border"
.LASF198:
	.string	"TX_SIZE"
.LASF18:
	.string	"VPX_IMG_FMT_BGR24"
.LASF48:
	.string	"vpx_codec_err_t"
.LASF200:
	.string	"ADST_DCT"
.LASF373:
	.string	"vp8_recon_rtcd_vtable_t"
.LASF201:
	.string	"DCT_ADST"
.LASF315:
	.string	"value"
.LASF424:
	.string	"VP8E_SET_SHARPNESS__convert"
.LASF459:
	.string	"vp8_check_mv_bounds"
.LASF29:
	.string	"x_chroma_shift"
.LASF13:
	.string	"VPX_IMG_FMT_RGB565"
.LASF192:
	.string	"SPLITMV"
.LASF51:
	.string	"__gnuc_va_list"
.LASF309:
	.string	"subpix"
.LASF11:
	.string	"VPX_IMG_FMT_RGB24"
.LASF230:
	.string	"MV_REFERENCE_FRAME"
.LASF252:
	.string	"predictor"
.LASF438:
	.string	"VP8E_SET_CQ_LEVEL__convert"
.LASF4:
	.string	"size_t"
.LASF86:
	.string	"vpx_scaling_mode_1d"
.LASF226:
	.string	"LAST_FRAME"
.LASF452:
	.string	"offset"
.LASF88:
	.string	"VP8E_FOURFIVE"
.LASF447:
	.string	"is_4x4"
.LASF231:
	.string	"mode"
.LASF418:
	.string	"VP8E_SET_CPUUSED__convert"
.LASF129:
	.string	"frame_size"
.LASF349:
	.string	"vp8_idct_rtcd_vtable_t"
.LASF250:
	.string	"qcoeff"
.LASF98:
	.string	"static_threshold"
.LASF457:
	.string	"vp8_clamp_mv2"
.LASF449:
	.string	"probability"
.LASF20:
	.string	"VPX_IMG_FMT_ARGB"
.LASF40:
	.string	"VPX_CODEC_MEM_ERROR"
.LASF399:
	.string	"VP8_SET_DBG_COLOR_MB_MODES__value"
.LASF462:
	.string	"cur_mb"
.LASF246:
	.string	"pred_filter_enabled"
.LASF132:
	.string	"flags"
.LASF280:
	.string	"mode_ref_lf_delta_enabled"
.LASF191:
	.string	"NEWMV"
.LASF237:
	.string	"second_mv"
.LASF402:
	.string	"VP8_SET_DBG_COLOR_B_MODES__convert"
.LASF348:
	.string	"ihaar2_1"
.LASF30:
	.string	"y_chroma_shift"
.LASF264:
	.string	"fullpixel_mask"
.LASF433:
	.string	"VP8E_SET_ARNR_TYPE__value"
.LASF429:
	.string	"VP8E_SET_ARNR_MAXFRAMES__value"
.LASF63:
	.string	"__jmpbuf"
.LASF397:
	.string	"VP8_SET_DBG_COLOR_REF_FRAME__value"
.LASF223:
	.string	"second"
.LASF257:
	.string	"pre_stride"
.LASF321:
	.string	"mblim"
.LASF217:
	.string	"NEW4X4"
.LASF359:
	.string	"recon_uv"
.LASF255:
	.string	"base_pre"
.LASF461:
	.string	"left_block_mv"
.LASF311:
	.string	"postproc"
.LASF89:
	.string	"VP8E_THREEFIVE"
.LASF157:
	.string	"sixtap_avg16x16"
.LASF163:
	.string	"bilinear8x8"
.LASF262:
	.string	"eobs"
.LASF49:
	.string	"char"
.LASF267:
	.string	"mode_info_context"
.LASF296:
	.string	"subpixel_predict_avg"
.LASF307:
	.string	"idct"
.LASF52:
	.string	"vpx_image"
.LASF81:
	.string	"VP8_ALTR_FRAME"
.LASF78:
	.string	"vpx_ref_frame_type"
.LASF423:
	.string	"VP8E_SET_SHARPNESS__value"
.LASF127:
	.string	"buffer_alloc"
.LASF83:
	.string	"vpx_ref_frame"
.LASF202:
	.string	"ADST_ADST"
.LASF66:
	.string	"jmp_buf"
.LASF178:
	.string	"H_PRED"
.LASF435:
	.string	"VP8E_SET_TUNING__value"
.LASF143:
	.string	"eighttap_avg16x16"
.LASF102:
	.string	"vpx_active_map_t"
.LASF33:
	.string	"user_priv"
.LASF378:
	.string	"vp8_postproc_blend_mb_outer_fn_t"
.LASF146:
	.string	"eighttap8x4"
.LASF249:
	.string	"MODE_INFO"
.LASF5:
	.string	"uint8_t"
.LASF142:
	.string	"eighttap8x8"
.LASF391:
	.string	"VP8_SET_REFERENCE__value"
.LASF169:
	.string	"vp8_subpix_rtcd_vtable_t"
.LASF211:
	.string	"B_VL_PRED"
.LASF197:
	.string	"TX_SIZE_MAX"
.LASF467:
	.string	"left_block_mode"
.LASF466:
	.string	"above_block_second_mv"
.LASF300:
	.string	"allow_high_precision_mv"
.LASF99:
	.string	"vpx_roi_map_t"
.LASF354:
	.string	"copy16x16"
.LASF137:
	.string	"as_mv"
.LASF345:
	.string	"idct8_1"
.LASF6:
	.string	"unsigned char"
.LASF275:
	.string	"update_mb_segmentation_data"
.LASF43:
	.string	"VPX_CODEC_UNSUP_BITSTREAM"
.LASF382:
	.string	"downacross"
.LASF247:
	.string	"MB_MODE_INFO"
.LASF58:
	.string	"va_list"
.LASF369:
	.string	"intra8x8_predict"
.LASF347:
	.string	"ihaar2"
.LASF336:
	.string	"vp8_idct_fn_t"
.LASF384:
	.string	"blend_mb_inner"
.LASF134:
	.string	"long long int"
.LASF34:
	.string	"img_data"
.LASF179:
	.string	"D45_PRED"
.LASF234:
	.string	"ref_frame"
.LASF186:
	.string	"I8X8_PRED"
.LASF437:
	.string	"VP8E_SET_CQ_LEVEL__value"
.LASF37:
	.string	"vpx_image_t"
.LASF304:
	.string	"mb_index"
.LASF125:
	.string	"u_buffer"
.LASF341:
	.string	"idct1_scalar_add"
.LASF233:
	.string	"mode_rdopt"
.LASF439:
	.string	"VP8E_GET_LAST_QUANTIZER__value"
.LASF47:
	.string	"VPX_CODEC_LIST_END"
.LASF425:
	.string	"VP8E_SET_STATIC_THRESHOLD__value"
.LASF364:
	.string	"build_intra_predictors_mby_s"
.LASF119:
	.string	"y_height"
.LASF386:
	.string	"blend_b"
.LASF222:
	.string	"tx_type"
.LASF69:
	.string	"has_detail"
.LASF173:
	.string	"KEY_FRAME"
.LASF16:
	.string	"VPX_IMG_FMT_YUY2"
.LASF297:
	.string	"subpixel_predict_avg8x4"
.LASF111:
	.string	"vp8e_token_partitions"
.LASF324:
	.string	"loop_filter_info"
.LASF203:
	.string	"TX_TYPE"
.LASF298:
	.string	"subpixel_predict_avg8x8"
.LASF207:
	.string	"B_HE_PRED"
.LASF144:
	.string	"eighttap_avg8x8"
.LASF370:
	.string	"intra_uv4x4_predict"
.LASF152:
	.string	"eighttap_avg4x4_sharp"
.LASF44:
	.string	"VPX_CODEC_UNSUP_FEATURE"
.LASF122:
	.string	"uv_height"
.LASF314:
	.string	"range"
.LASF279:
	.string	"segment_feature_mask"
.LASF172:
	.string	"ENTROPY_CONTEXT_PLANES"
.LASF26:
	.string	"VPX_IMG_FMT_VPXYV12"
.LASF409:
	.string	"VP8E_USE_REFERENCE__value"
.LASF41:
	.string	"VPX_CODEC_ABI_MISMATCH"
.LASF417:
	.string	"VP8E_SET_CPUUSED__value"
.LASF19:
	.string	"VPX_IMG_FMT_RGB32_LE"
.LASF388:
	.string	"info"
.LASF248:
	.string	"mbmi"
.LASF295:
	.string	"subpixel_predict16x16"
.LASF330:
	.string	"normal_b_h"
.LASF1:
	.string	"long unsigned int"
.LASF187:
	.string	"B_PRED"
.LASF244:
	.string	"ref_predicted"
.LASF421:
	.string	"VP8E_SET_NOISE_SENSITIVITY__value"
.LASF138:
	.string	"int_mv"
.LASF464:
	.string	"above_block_mv"
.LASF90:
	.string	"VP8E_ONETWO"
.LASF350:
	.string	"vp8_copy_block_fn_t"
.LASF272:
	.string	"left_context"
.LASF97:
	.string	"delta_lf"
.LASF340:
	.string	"idct16"
.LASF239:
	.string	"mb_skip_coeff"
.LASF196:
	.string	"TX_8X8"
.LASF256:
	.string	"base_second_pre"
.LASF84:
	.string	"frame_type"
.LASF22:
	.string	"VPX_IMG_FMT_RGB565_LE"
.LASF412:
	.string	"VP8E_SET_ROI_MAP__convert"
.LASF346:
	.string	"idct1_scalar_add_8x8"
.LASF120:
	.string	"y_stride"
.LASF366:
	.string	"build_intra_predictors_mbuv_s"
.LASF390:
	.string	"args"
.LASF35:
	.string	"img_data_owner"
.LASF7:
	.string	"short unsigned int"
.LASF251:
	.string	"dqcoeff"
.LASF422:
	.string	"VP8E_SET_NOISE_SENSITIVITY__convert"
.LASF79:
	.string	"VP8_LAST_FRAME"
.LASF14:
	.string	"VPX_IMG_FMT_RGB555"
.LASF443:
	.string	"VP8E_SET_MAX_INTRA_BITRATE_PCT__value"
.LASF470:
	.string	"vp8_norm"
.LASF32:
	.string	"stride"
.LASF468:
	.string	"above_block_mode"
.LASF385:
	.string	"blend_mb_outer"
.LASF123:
	.string	"uv_stride"
.LASF268:
	.string	"mode_info_stride"
.LASF160:
	.string	"sixtap4x4"
.LASF184:
	.string	"D63_PRED"
.LASF113:
	.string	"VP8_TUNE_SSIM"
.LASF352:
	.string	"vp8_recon_mb_fn_t"
.LASF395:
	.string	"VP8_SET_POSTPROC__value"
.LASF232:
	.string	"uv_mode"
.LASF126:
	.string	"v_buffer"
.LASF285:
	.string	"mode_lf_deltas"
.LASF460:
	.string	"need_to_clamp"
.LASF25:
	.string	"VPX_IMG_FMT_I420"
.LASF139:
	.string	"vp8_prob"
.LASF224:
	.string	"as_mode"
.LASF45:
	.string	"VPX_CODEC_CORRUPT_FRAME"
.LASF218:
	.string	"B_MODE_COUNT"
.LASF401:
	.string	"VP8_SET_DBG_COLOR_B_MODES__value"
.LASF3:
	.string	"short int"
.LASF259:
	.string	"dst_stride"
.LASF465:
	.string	"mi_stride"
.LASF299:
	.string	"subpixel_predict_avg16x16"
.LASF121:
	.string	"uv_width"
.LASF327:
	.string	"normal_mb_v"
.LASF310:
	.string	"loopfilter"
.LASF0:
	.string	"long int"
.LASF161:
	.string	"sixtap_avg4x4"
.LASF229:
	.string	"MAX_REF_FRAMES"
.LASF140:
	.string	"vp8_subpix_fn_t"
.LASF377:
	.string	"vp8_postproc_blend_mb_inner_fn_t"
.LASF351:
	.string	"vp8_recon_fn_t"
.LASF408:
	.string	"VP8E_UPD_REFERENCE__convert"
.LASF360:
	.string	"recon2"
.LASF361:
	.string	"recon4"
.LASF282:
	.string	"last_ref_lf_deltas"
.LASF253:
	.string	"diff"
.LASF67:
	.string	"vpx_internal_error_info"
.LASF130:
	.string	"clrtype"
.LASF400:
	.string	"VP8_SET_DBG_COLOR_MB_MODES__convert"
.LASF353:
	.string	"vp8_recon_rtcd_vtable"
.LASF274:
	.string	"update_mb_segmentation_map"
.LASF463:
	.string	"left_block_second_mv"
.LASF183:
	.string	"D27_PRED"
.LASF258:
	.string	"base_dst"
.LASF254:
	.string	"dequant"
.LASF177:
	.string	"V_PRED"
.LASF414:
	.string	"VP8E_SET_ACTIVEMAP__convert"
.LASF150:
	.string	"eighttap_avg16x16_sharp"
.LASF53:
	.string	"__va_list_tag"
.LASF188:
	.string	"NEARESTMV"
.LASF61:
	.string	"__sigset_t"
.LASF367:
	.string	"build_intra_predictors_mbuv"
.LASF164:
	.string	"bilinear_avg16x16"
.LASF263:
	.string	"block"
.LASF241:
	.string	"need_to_clamp_secondmv"
.LASF190:
	.string	"ZEROMV"
.LASF55:
	.string	"fp_offset"
.LASF175:
	.string	"FRAME_TYPE"
.LASF73:
	.string	"post_proc_flag"
.LASF64:
	.string	"__mask_was_saved"
.LASF392:
	.string	"VP8_SET_REFERENCE__convert"
.LASF9:
	.string	"unsigned int"
.LASF136:
	.string	"as_int"
.LASF105:
	.string	"v_scaling_mode"
.LASF171:
	.string	"ENTROPY_CONTEXT"
.LASF72:
	.string	"vp8_postproc_cfg"
.LASF65:
	.string	"__saved_mask"
.LASF287:
	.string	"mb_to_right_edge"
.LASF312:
	.string	"MACROBLOCKD"
.LASF475:
	.string	"main"
.LASF431:
	.string	"VP8E_SET_ARNR_STRENGTH__value"
.LASF357:
	.string	"avg8x8"
.LASF318:
	.string	"measure_cost"
.LASF284:
	.string	"last_mode_lf_deltas"
.LASF338:
	.string	"vp8_second_order_fn_t"
.LASF455:
	.string	"refframe"
.LASF358:
	.string	"copy8x4"
.LASF38:
	.string	"VPX_CODEC_OK"
.LASF379:
	.string	"vp8_postproc_blend_b_fn_t"
.LASF355:
	.string	"copy8x8"
.LASF92:
	.string	"vpx_roi_map"
.LASF57:
	.string	"reg_save_area"
.LASF180:
	.string	"D135_PRED"
.LASF74:
	.string	"deblocking_level"
.LASF374:
	.string	"vp8_postproc_fn_t"
.LASF100:
	.string	"vpx_active_map"
.LASF328:
	.string	"normal_b_v"
.LASF96:
	.string	"delta_q"
.LASF276:
	.string	"mb_segment_abs_delta"
.LASF167:
	.string	"bilinear4x4"
.LASF434:
	.string	"VP8E_SET_ARNR_TYPE__convert"
.LASF436:
	.string	"VP8E_SET_TUNING__convert"
.LASF159:
	.string	"sixtap8x4"
.LASF36:
	.string	"self_allocd"
.LASF133:
	.string	"YV12_BUFFER_CONFIG"
.LASF474:
	.string	"b_mode_info"
.LASF387:
	.string	"vp8_postproc_rtcd_vtable_t"
.LASF472:
	.string	"../../vp8/encoder/asm_enc_offsets.c"
.LASF303:
	.string	"rtcd"
.LASF294:
	.string	"subpixel_predict8x8"
.LASF265:
	.string	"second_pre"
.LASF154:
	.string	"eighttap4x4_sharp"
.LASF293:
	.string	"subpixel_predict8x4"
.LASF292:
	.string	"subpixel_predict"
.LASF131:
	.string	"corrupted"
.LASF441:
	.string	"VP8E_GET_LAST_QUANTIZER_64__value"
.LASF220:
	.string	"first"
.LASF407:
	.string	"VP8E_UPD_REFERENCE__value"
.LASF147:
	.string	"eighttap4x4"
.LASF356:
	.string	"avg16x16"
.LASF165:
	.string	"bilinear_avg8x8"
.LASF343:
	.string	"iwalsh16"
.LASF334:
	.string	"simple_b_h"
.LASF68:
	.string	"error_code"
.LASF189:
	.string	"NEARMV"
.LASF420:
	.string	"VP8E_SET_ENABLEAUTOALTREF__convert"
.LASF365:
	.string	"build_intra_predictors_mby"
.LASF375:
	.string	"vp8_postproc_inplace_fn_t"
.LASF332:
	.string	"simple_b_v"
.LASF319:
	.string	"bit_counter"
.LASF166:
	.string	"bilinear8x4"
.LASF261:
	.string	"MacroBlockD"
.LASF39:
	.string	"VPX_CODEC_ERROR"
.LASF209:
	.string	"B_RD_PRED"
.LASF124:
	.string	"y_buffer"
.LASF404:
	.string	"VP8_SET_DBG_DISPLAY_MV__convert"
.LASF116:
	.string	"INT_YUV"
.LASF305:
	.string	"q_index"
.LASF95:
	.string	"cols"
.LASF75:
	.string	"noise_level"
.LASF212:
	.string	"B_HD_PRED"
.LASF403:
	.string	"VP8_SET_DBG_DISPLAY_MV__value"
.LASF27:
	.string	"VPX_IMG_FMT_VPXI420"
.LASF216:
	.string	"ZERO4X4"
.LASF108:
	.string	"VP8_TWO_TOKENPARTITION"
.LASF60:
	.string	"__val"
.LASF419:
	.string	"VP8E_SET_ENABLEAUTOALTREF__value"
.LASF228:
	.string	"ALTREF_FRAME"
.LASF322:
	.string	"blim"
.LASF383:
	.string	"addnoise"
.LASF50:
	.string	"double"
.LASF446:
	.string	"update_blockd_bmi"
.LASF227:
	.string	"GOLDEN_FRAME"
.LASF219:
	.string	"B_PREDICTION_MODE"
.LASF410:
	.string	"VP8E_USE_REFERENCE__convert"
.LASF306:
	.string	"VP8_COMMON_RTCD"
.LASF23:
	.string	"VPX_IMG_FMT_RGB555_LE"
.LASF271:
	.string	"above_context"
.LASF337:
	.string	"vp8_idct_scalar_add_fn_t"
.LASF80:
	.string	"VP8_GOLD_FRAME"
.LASF456:
	.string	"ref_frame_sign_bias"
.LASF398:
	.string	"VP8_SET_DBG_COLOR_REF_FRAME__convert"
.LASF145:
	.string	"eighttap_avg4x4"
.LASF104:
	.string	"h_scaling_mode"
.LASF182:
	.string	"D153_PRED"
.LASF266:
	.string	"prev_mode_info_context"
.LASF193:
	.string	"MB_MODE_COUNT"
.LASF376:
	.string	"vp8_postproc_addnoise_fn_t"
.LASF199:
	.string	"DCT_DCT"
.LASF195:
	.string	"TX_4X4"
.LASF339:
	.string	"idct1"
.LASF118:
	.string	"y_width"
.LASF473:
	.string	"/usr/local/google/home/jingning/explibvpx-pub/libvpx/build/linuxbuild"
.LASF236:
	.string	"txfm_size"
.LASF115:
	.string	"REG_YUV"
.LASF344:
	.string	"idct8"
.LASF413:
	.string	"VP8E_SET_ACTIVEMAP__value"
.LASF87:
	.string	"VP8E_NORMAL"
.LASF372:
	.string	"vp8_intra4x4_pred_fn_t"
.LASF153:
	.string	"eighttap8x4_sharp"
.LASF62:
	.string	"__jmp_buf_tag"
.LASF325:
	.string	"vp8_lf_block_fn_t"
.LASF243:
	.string	"seg_id_predicted"
.LASF114:
	.string	"vp8e_tuning"
	.ident	"GCC: (Ubuntu 4.4.3-4ubuntu5.1) 4.4.3"
	.section	.note.GNU-stack,"",@progbits
