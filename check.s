	.file	"input_for_matmul.cpp"
                                        # Start of file scope inline assembly
	.globl	_ZSt21ios_base_library_initv

                                        # End of file scope inline assembly
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function main
.LCPI0_0:
	.long	0x30000000                      # float 4.65661287E-10
.LCPI0_1:
	.long	0x0da24260                      # float 1.0E-30
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0
.LCPI0_2:
	.quad	0x412e848000000000              # double 1.0E+6
	.text
	.globl	main
	.p2align	4
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movl	$3, %edi
	callq	_ZL10run_matmulIdLi100EEdi
	movq	_ZSt4cout@GOTPCREL(%rip), %rbx
	leaq	.L.str(%rip), %rsi
	movl	$35, %edx
	movq	%rbx, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movl	$500, %edi                      # imm = 0x1F4
	callq	_ZL10run_matmulIdLi100EEdi
	movq	%rbx, %rdi
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	leaq	.L.str.1(%rip), %rsi
	movl	$3, %edx
	movq	%rax, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	leaq	.L.str.2(%rip), %rsi
	movl	$34, %edx
	movq	%rbx, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	xorl	%ebx, %ebx
	xorl	%edi, %edi
	callq	srand@PLT
	leaq	_ZZL10run_matmulIfLi100EEdiE4mat1(%rip), %r14
	leaq	_ZZL10run_matmulIfLi100EEdiE4mat2(%rip), %r15
	xorl	%r12d, %r12d
	.p2align	4
.LBB0_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_2 Depth 2
	xorl	%r13d, %r13d
	.p2align	4
.LBB0_2:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	callq	rand@PLT
	xorps	%xmm0, %xmm0
	cvtsi2ss	%eax, %xmm0
	mulss	.LCPI0_0(%rip), %xmm0
	leaq	(%rbx,%r13), %rbp
	movss	%xmm0, (%r14,%rbp)
	callq	rand@PLT
	movss	.LCPI0_0(%rip), %xmm1           # xmm1 = [4.65661287E-10,0.0E+0,0.0E+0,0.0E+0]
	xorps	%xmm0, %xmm0
	cvtsi2ss	%eax, %xmm0
	mulss	%xmm1, %xmm0
	movss	%xmm0, (%r15,%rbp)
	addq	$4, %r13
	cmpq	$400, %r13                      # imm = 0x190
	jne	.LBB0_2
# %bb.3:                                #   in Loop: Header=BB0_1 Depth=1
	incq	%r12
	addq	$400, %rbx                      # imm = 0x190
	cmpq	$100, %r12
	jne	.LBB0_1
# %bb.4:
	leaq	_ZZL10run_matmulIfLi100EEdiE4mat2(%rip), %rax
	#APP
	#NO_APP
	movl	$500, %ebp                      # imm = 0x1F4
	xorl	%r12d, %r12d
	callq	_ZNSt6chrono3_V212steady_clock3nowEv@PLT
	movq	%rax, 16(%rsp)                  # 8-byte Spill
	leaq	_ZZL10run_matmulIfLi100EEdiE3res(%rip), %r15
	xorl	%r13d, %r13d
	.p2align	4
.LBB0_5:                                # =>This Inner Loop Header: Depth=1
	movq	%r12, %rax
	shrq	$2, %rax
	movabsq	$2951479051793528259, %rcx      # imm = 0x28F5C28F5C28F5C3
	mulq	%rcx
	shrq	$2, %rdx
	imulq	$40400, %rdx, %rax              # imm = 0x9DD0
	movq	%r13, %rbx
	subq	%rax, %rbx
	movl	$40000, %edx                    # imm = 0x9C40
	movq	%r15, %rdi
	xorl	%esi, %esi
	callq	memset@PLT
	#APP
	#NO_APP
	movss	(%r15,%rbx), %xmm0              # xmm0 = mem[0],zero,zero,zero
	mulss	.LCPI0_1(%rip), %xmm0
	addss	(%r14,%rbx), %xmm0
	movss	%xmm0, (%r14,%rbx)
	addq	$404, %r13                      # imm = 0x194
	incq	%r12
	decl	%ebp
	jne	.LBB0_5
# %bb.6:
	xorl	%r12d, %r12d
	callq	_ZNSt6chrono3_V212steady_clock3nowEv@PLT
	xorps	%xmm0, %xmm0
	movq	%rax, %r14
	.p2align	4
.LBB0_7:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_8 Depth 2
	movl	$4, %eax
	.p2align	4
.LBB0_8:                                #   Parent Loop BB0_7 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addss	-16(%r15,%rax,4), %xmm0
	addss	-12(%r15,%rax,4), %xmm0
	addss	-8(%r15,%rax,4), %xmm0
	addss	-4(%r15,%rax,4), %xmm0
	addss	(%r15,%rax,4), %xmm0
	addq	$5, %rax
	cmpq	$104, %rax
	jne	.LBB0_8
# %bb.9:                                #   in Loop: Header=BB0_7 Depth=1
	incq	%r12
	addq	$400, %r15                      # imm = 0x190
	cmpq	$100, %r12
	jne	.LBB0_7
# %bb.10:
	movq	_ZSt4cout@GOTPCREL(%rip), %r15
	leaq	.L.str.3(%rip), %rsi
	movl	$9, %edx
	movq	%r15, %rdi
	movss	%xmm0, 12(%rsp)                 # 4-byte Spill
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movss	12(%rsp), %xmm0                 # 4-byte Reload
                                        # xmm0 = mem[0],zero,zero,zero
	cvtss2sd	%xmm0, %xmm0
	movq	%r15, %rdi
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %r15
	testq	%r15, %r15
	je	.LBB0_15
# %bb.11:
	cmpb	$0, 56(%r15)
	je	.LBB0_13
# %bb.12:
	movzbl	67(%r15), %ecx
	jmp	.LBB0_14
.LBB0_13:
	movq	%r15, %rdi
	movq	%rax, %rbx
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%r15), %rax
	movq	%r15, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%rbx, %rax
.LBB0_14:
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	subq	16(%rsp), %r14                  # 8-byte Folded Reload
	xorps	%xmm0, %xmm0
	cvtsi2sd	%r14, %xmm0
	divsd	.LCPI0_2(%rip), %xmm0
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	leaq	.L.str.1(%rip), %rsi
	movl	$3, %edx
	movq	%rax, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	xorl	%eax, %eax
	addq	$24, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB0_15:
	.cfi_def_cfa_offset 80
	callq	_ZSt16__throw_bad_castv@PLT
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function _ZL10run_matmulIdLi100EEdi
.LCPI1_0:
	.quad	0x3e00000000000000              # double 4.6566128730773926E-10
.LCPI1_1:
	.quad	0x39b4484bfeebc2a0              # double 1.0000000000000001E-30
.LCPI1_2:
	.quad	0x412e848000000000              # double 1.0E+6
	.text
	.p2align	4
	.type	_ZL10run_matmulIdLi100EEdi,@function
_ZL10run_matmulIdLi100EEdi:             # @_ZL10run_matmulIdLi100EEdi
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movl	%edi, 8(%rsp)                   # 4-byte Spill
	xorl	%ebx, %ebx
	xorl	%edi, %edi
	callq	srand@PLT
	leaq	_ZZL10run_matmulIdLi100EEdiE4mat1(%rip), %r14
	leaq	_ZZL10run_matmulIdLi100EEdiE4mat2(%rip), %r15
	xorl	%r12d, %r12d
	.p2align	4
.LBB1_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB1_2 Depth 2
	xorl	%r13d, %r13d
	.p2align	4
.LBB1_2:                                #   Parent Loop BB1_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	callq	rand@PLT
	xorps	%xmm0, %xmm0
	cvtsi2sd	%eax, %xmm0
	mulsd	.LCPI1_0(%rip), %xmm0
	leaq	(%rbx,%r13), %rbp
	movsd	%xmm0, (%r14,%rbp)
	callq	rand@PLT
	movsd	.LCPI1_0(%rip), %xmm1           # xmm1 = [4.6566128730773926E-10,0.0E+0]
	xorps	%xmm0, %xmm0
	cvtsi2sd	%eax, %xmm0
	mulsd	%xmm1, %xmm0
	movsd	%xmm0, (%r15,%rbp)
	addq	$8, %r13
	cmpq	$800, %r13                      # imm = 0x320
	jne	.LBB1_2
# %bb.3:                                #   in Loop: Header=BB1_1 Depth=1
	incq	%r12
	addq	$800, %rbx                      # imm = 0x320
	cmpq	$100, %r12
	jne	.LBB1_1
# %bb.4:
	leaq	_ZZL10run_matmulIdLi100EEdiE4mat2(%rip), %rax
	#APP
	#NO_APP
	xorl	%r12d, %r12d
	callq	_ZNSt6chrono3_V212steady_clock3nowEv@PLT
	movq	%rax, 16(%rsp)                  # 8-byte Spill
	leaq	_ZZL10run_matmulIdLi100EEdiE3res(%rip), %r15
	xorl	%ebx, %ebx
	movl	8(%rsp), %r13d                  # 4-byte Reload
	.p2align	4
.LBB1_5:                                # =>This Inner Loop Header: Depth=1
	movq	%r12, %rax
	shrq	$2, %rax
	movabsq	$2951479051793528259, %rcx      # imm = 0x28F5C28F5C28F5C3
	mulq	%rcx
	shrq	$2, %rdx
	imulq	$80800, %rdx, %rax              # imm = 0x13BA0
	movq	%rbx, %rbp
	subq	%rax, %rbp
	movl	$80000, %edx                    # imm = 0x13880
	movq	%r15, %rdi
	xorl	%esi, %esi
	callq	memset@PLT
	#APP
	#NO_APP
	movsd	(%r15,%rbp), %xmm0              # xmm0 = mem[0],zero
	mulsd	.LCPI1_1(%rip), %xmm0
	addsd	(%r14,%rbp), %xmm0
	movsd	%xmm0, (%r14,%rbp)
	addq	$808, %rbx                      # imm = 0x328
	incq	%r12
	decl	%r13d
	jne	.LBB1_5
# %bb.6:
	xorl	%r12d, %r12d
	callq	_ZNSt6chrono3_V212steady_clock3nowEv@PLT
	xorpd	%xmm0, %xmm0
	movq	%rax, %r14
	.p2align	4
.LBB1_7:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB1_8 Depth 2
	movl	$4, %eax
	.p2align	4
.LBB1_8:                                #   Parent Loop BB1_7 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addsd	-32(%r15,%rax,8), %xmm0
	addsd	-24(%r15,%rax,8), %xmm0
	addsd	-16(%r15,%rax,8), %xmm0
	addsd	-8(%r15,%rax,8), %xmm0
	addsd	(%r15,%rax,8), %xmm0
	addq	$5, %rax
	cmpq	$104, %rax
	jne	.LBB1_8
# %bb.9:                                #   in Loop: Header=BB1_7 Depth=1
	incq	%r12
	addq	$800, %r15                      # imm = 0x320
	cmpq	$100, %r12
	jne	.LBB1_7
# %bb.10:
	movq	_ZSt4cout@GOTPCREL(%rip), %r15
	leaq	.L.str.3(%rip), %rsi
	movl	$9, %edx
	movq	%r15, %rdi
	movsd	%xmm0, 8(%rsp)                  # 8-byte Spill
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movq	%r15, %rdi
	movsd	8(%rsp), %xmm0                  # 8-byte Reload
                                        # xmm0 = mem[0],zero
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %r15
	testq	%r15, %r15
	je	.LBB1_15
# %bb.11:
	cmpb	$0, 56(%r15)
	je	.LBB1_13
# %bb.12:
	movzbl	67(%r15), %ecx
	jmp	.LBB1_14
.LBB1_13:
	movq	%r15, %rdi
	movq	%rax, %rbx
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%r15), %rax
	movq	%r15, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%rbx, %rax
.LBB1_14:
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	subq	16(%rsp), %r14                  # 8-byte Folded Reload
	xorps	%xmm0, %xmm0
	cvtsi2sd	%r14, %xmm0
	divsd	.LCPI1_2(%rip), %xmm0
	addq	$24, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB1_15:
	.cfi_def_cfa_offset 80
	callq	_ZSt16__throw_bad_castv@PLT
.Lfunc_end1:
	.size	_ZL10run_matmulIdLi100EEdi, .Lfunc_end1-_ZL10run_matmulIdLi100EEdi
	.cfi_endproc
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"Double type matmul execution time: "
	.size	.L.str, 36

	.type	.L.str.1,@object                # @.str.1
.L.str.1:
	.asciz	"ms\n"
	.size	.L.str.1, 4

	.type	.L.str.2,@object                # @.str.2
.L.str.2:
	.asciz	"Float type matmul execution time: "
	.size	.L.str.2, 35

	.type	_ZZL10run_matmulIdLi100EEdiE4mat1,@object # @_ZZL10run_matmulIdLi100EEdiE4mat1
	.local	_ZZL10run_matmulIdLi100EEdiE4mat1
	.comm	_ZZL10run_matmulIdLi100EEdiE4mat1,80000,16
	.type	_ZZL10run_matmulIdLi100EEdiE4mat2,@object # @_ZZL10run_matmulIdLi100EEdiE4mat2
	.local	_ZZL10run_matmulIdLi100EEdiE4mat2
	.comm	_ZZL10run_matmulIdLi100EEdiE4mat2,80000,16
	.type	_ZZL10run_matmulIdLi100EEdiE3res,@object # @_ZZL10run_matmulIdLi100EEdiE3res
	.local	_ZZL10run_matmulIdLi100EEdiE3res
	.comm	_ZZL10run_matmulIdLi100EEdiE3res,80000,16
	.type	.L.str.3,@object                # @.str.3
.L.str.3:
	.asciz	"checksum="
	.size	.L.str.3, 10

	.type	_ZZL10run_matmulIfLi100EEdiE4mat1,@object # @_ZZL10run_matmulIfLi100EEdiE4mat1
	.local	_ZZL10run_matmulIfLi100EEdiE4mat1
	.comm	_ZZL10run_matmulIfLi100EEdiE4mat1,40000,16
	.type	_ZZL10run_matmulIfLi100EEdiE4mat2,@object # @_ZZL10run_matmulIfLi100EEdiE4mat2
	.local	_ZZL10run_matmulIfLi100EEdiE4mat2
	.comm	_ZZL10run_matmulIfLi100EEdiE4mat2,40000,16
	.type	_ZZL10run_matmulIfLi100EEdiE3res,@object # @_ZZL10run_matmulIfLi100EEdiE3res
	.local	_ZZL10run_matmulIfLi100EEdiE3res
	.comm	_ZZL10run_matmulIfLi100EEdiE3res,40000,16
	.ident	"clang version 21.1.8 (https://github.com/llvm/llvm-project.git 2078da43e25a4623cab2d0d60decddf709aaea28)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym _ZSt4cout
	.addrsig_sym _ZZL10run_matmulIdLi100EEdiE4mat1
	.addrsig_sym _ZZL10run_matmulIdLi100EEdiE4mat2
	.addrsig_sym _ZZL10run_matmulIdLi100EEdiE3res
	.addrsig_sym _ZZL10run_matmulIfLi100EEdiE4mat1
	.addrsig_sym _ZZL10run_matmulIfLi100EEdiE4mat2
	.addrsig_sym _ZZL10run_matmulIfLi100EEdiE3res
