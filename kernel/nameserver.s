	.file	"nameserver.c"
	.text
	.align	2
	.global	tempFunction
	.type	tempFunction, %function
tempFunction:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	ldmfd	sp, {fp, sp, pc}
	.size	tempFunction, .-tempFunction
	.ident	"GCC: (GNU) 4.0.2"
