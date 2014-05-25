	.file	"kernel.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"first task: ROY\012\000"
	.align	2
.LC1:
	.ascii	"first task: BILL1\012\000"
	.align	2
.LC2:
	.ascii	"first task: HE\012\000"
	.align	2
.LC3:
	.ascii	"first task: BILL2\012\000"
	.align	2
.LC4:
	.ascii	"first task: BILL3\012\000"
	.align	2
.LC5:
	.ascii	"first task: BILL4\012\000"
	.text
	.align	2
	.global	FirstUserTask
	.type	FirstUserTask, %function
FirstUserTask:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	sl, .L5
.L4:
	add	sl, pc, sl
	mov	r0, #1
	ldr	r3, .L5+4
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
.L2:
	mov	r0, #1
	ldr	r3, .L5+8
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+12
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+16
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+12
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+20
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+12
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+24
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	mov	r0, #1
	ldr	r3, .L5+12
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	swi
	b	.L2
.L6:
	.align	2
.L5:
	.word	_GLOBAL_OFFSET_TABLE_-(.L4+8)
	.word	.LC0(GOTOFF)
	.word	.LC1(GOTOFF)
	.word	.LC2(GOTOFF)
	.word	.LC3(GOTOFF)
	.word	.LC4(GOTOFF)
	.word	.LC5(GOTOFF)
	.size	FirstUserTask, .-FirstUserTask
	.align	2
	.global	schedule
	.type	schedule, %function
schedule:
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #24
	str	r0, [fp, #-36]
	mov	r3, #0
	str	r3, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-24]
	b	.L8
.L9:
	ldr	r3, [fp, #-24]
	mov	r3, r3, asl #3
	mov	r2, r3
	ldr	r3, [fp, #-36]
	add	r3, r2, r3
	ldmia	r3, {r3-r4}
	str	r3, [fp, #-32]
	str	r4, [fp, #-28]
	ldr	r3, [fp, #-32]
	cmp	r3, #0
	beq	.L10
	ldr	r3, [fp, #-32]
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #24]
	str	r3, [fp, #-32]
	ldr	r3, [fp, #-20]
	str	r3, [fp, #-40]
	b	.L12
.L10:
	ldr	r3, [fp, #-24]
	add	r3, r3, #1
	str	r3, [fp, #-24]
.L8:
	ldr	r3, [fp, #-24]
	cmp	r3, #15
	ble	.L9
	mov	r3, #0
	str	r3, [fp, #-40]
.L12:
	ldr	r3, [fp, #-40]
	mov	r0, r3
	sub	sp, fp, #16
	ldmfd	sp, {r4, fp, sp, pc}
	.size	schedule, .-schedule
	.align	2
	.global	initialize_td_pq
	.type	initialize_td_pq, %function
initialize_td_pq:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	str	r0, [fp, #-20]
	mov	r3, #0
	str	r3, [fp, #-16]
	mov	r3, #0
	str	r3, [fp, #-16]
	b	.L16
.L17:
	ldr	r3, [fp, #-16]
	mov	r3, r3, asl #3
	mov	r2, r3
	ldr	r3, [fp, #-20]
	add	r2, r2, r3
	mov	r3, #0
	str	r3, [r2, #0]
	ldr	r3, [fp, #-16]
	mov	r3, r3, asl #3
	mov	r2, r3
	ldr	r3, [fp, #-20]
	add	r2, r2, r3
	mov	r3, #0
	str	r3, [r2, #4]
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	str	r3, [fp, #-16]
.L16:
	ldr	r3, [fp, #-16]
	cmp	r3, #15
	ble	.L17
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	initialize_td_pq, .-initialize_td_pq
	.align	2
	.global	get_free_td
	.type	get_free_td, %function
get_free_td:
	@ args = 0, pretend = 0, frame = 20
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #20
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-16]
	b	.L21
.L22:
	ldr	r3, [fp, #-20]
	ldr	r1, [r3, #0]
	mov	r2, #1
	ldr	r3, [fp, #-16]
	mov	r3, r2, asl r3
	and	r3, r1, r3
	cmp	r3, #0
	bne	.L23
	ldr	r3, [fp, #-20]
	ldr	r1, [r3, #0]
	mov	r2, #1
	ldr	r3, [fp, #-16]
	mov	r3, r2, asl r3
	orr	r2, r1, r3
	ldr	r3, [fp, #-20]
	str	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	str	r3, [fp, #-28]
	b	.L25
.L23:
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	str	r3, [fp, #-16]
.L21:
	ldr	r3, [fp, #-16]
	cmp	r3, #31
	ble	.L22
	mov	r3, #0
	str	r3, [fp, #-16]
	b	.L27
.L28:
	ldr	r3, [fp, #-24]
	ldr	r1, [r3, #0]
	mov	r2, #1
	ldr	r3, [fp, #-16]
	mov	r3, r2, asl r3
	and	r3, r1, r3
	cmp	r3, #0
	bne	.L29
	ldr	r3, [fp, #-24]
	ldr	r1, [r3, #0]
	mov	r2, #1
	ldr	r3, [fp, #-16]
	mov	r3, r2, asl r3
	orr	r2, r1, r3
	ldr	r3, [fp, #-24]
	str	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	add	r3, r3, #32
	str	r3, [fp, #-28]
	b	.L25
.L29:
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	str	r3, [fp, #-16]
.L27:
	ldr	r3, [fp, #-16]
	cmp	r3, #31
	ble	.L28
	b	.L20
.L25:
	ldr	r3, [fp, #-28]
	str	r3, [fp, #-32]
.L20:
	ldr	r0, [fp, #-32]
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	get_free_td, .-get_free_td
	.align	2
	.global	initialize
	.type	initialize, %function
initialize:
	@ args = 0, pretend = 0, frame = 20
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #20
	ldr	sl, .L36
.L35:
	add	sl, pc, sl
	str	r0, [fp, #-28]
	str	r1, [fp, #-32]
	str	r2, [fp, #-36]
	ldr	r3, .L36+4
	ldr	r3, [sl, r3]
	mov	r2, #2195456
	add	r3, r3, r2
	str	r3, [fp, #-24]
	mov	r3, #40
	str	r3, [fp, #-20]
	ldr	r2, [fp, #-24]
	ldr	r3, [fp, #-20]
	str	r2, [r3, #0]
	ldr	r2, [fp, #-28]
	mov	r3, #0
	str	r3, [r2, #0]
	ldr	r3, .L36+8
	ldr	r3, [sl, r3]
	mov	r2, #2195456
	add	r3, r3, r2
	mov	r2, r3
	ldr	r3, [fp, #-28]
	str	r2, [r3, #4]
	ldr	r2, [fp, #-28]
	mov	r3, #16777216
	str	r3, [r2, #8]
	ldr	r2, [fp, #-28]
	mov	r3, #223
	str	r3, [r2, #12]
	mov	r3, #0
	mov	r0, r3
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
.L37:
	.align	2
.L36:
	.word	_GLOBAL_OFFSET_TABLE_-(.L35+8)
	.word	ker_entry(GOT)
	.word	FirstUserTask(GOT)
	.size	initialize, .-initialize
	.align	2
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 1952
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #1952
	str	r0, [fp, #-1960]
	str	r1, [fp, #-1964]
	sub	r1, fp, #1808
	sub	r1, r1, #12
	sub	r2, fp, #1936
	sub	r2, r2, #12
	sub	r2, r2, #4
	sub	r3, fp, #1936
	sub	r3, r3, #12
	sub	r3, r3, #8
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	bl	initialize(PLT)
	mov	r3, r0
	str	r3, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-20]
	b	.L39
.L40:
	sub	r3, fp, #1808
	sub	r3, r3, #12
	str	r3, [fp, #-28]
	ldr	r0, [fp, #-28]
	mov	r1, #1
	bl	ker_exit(PLT)
	ldr	r3, [fp, #-20]
	add	r3, r3, #1
	str	r3, [fp, #-20]
.L39:
	ldr	r3, [fp, #-20]
	cmp	r3, #49
	ble	.L40
	mov	r3, #0
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	main, .-main
	.ident	"GCC: (GNU) 4.0.2"
