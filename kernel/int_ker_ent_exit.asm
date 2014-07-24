.file	"int_ker_ent_exit.c"
.text

.global	int_ker_entry
.type	int_ker_entry, %function
int_ker_entry:
	msr cpsr_c, #0xdf 				/* change to system mode */
	stmdb   sp!, {r0-r12, lr, ip} 	/* store the task registers */
	mov ip, sp

	msr cpsr_c, #0xd2 				/* change back to irq mode */
	mov r3, lr 						/* save the lr to r3 */
	sub r3, lr, #4

	msr cpsr_c, #0xd3
	ldmia 	sp!, {r1}					/* save all the argments */
	ldmia   sp!, {r0}			 	/* pop the kernal stack */
	str r3, [r0, #0x4] 				/* write to the lr of the TD */
	mrs r2, spsr 					/* save the spsr to r2 */
	str r2, [r0, #0xc]				/* store spsr to TD */
	str ip, [r0, #0x8]				/* store the sp to the TD */
	mov r0, #20						/* get the swi arguemnt and return it */
	ldmia   sp!, { r4-r12, pc} 		/* pop the kernal stack */
	/*mov pc, lr 						 this go back to kernel */
.size	int_ker_entry, .-int_ker_entry

.global	int_ker_exit
.type	int_ker_exit, %function
int_ker_exit:
	stmdb   sp!, { r4-r12, lr}		/* move registers to the kernal stack */
    stmdb   sp!, {r0}				/* move registers to the kernal stack */
 	stmdb   sp!, {r1}				/* move registers to the kernal stack */

    ldr r2, [r0, #0xc]				/* load the task spsr bak into cpsr */
    msr spsr, r2 					/* TODO: need to investigate this line */
	ldr lr, [r0, #4] 				/* load the pc of the td */

	msr cpsr_c, #0xdf				/* change to system mode */
    ldr sp, [r0, #8]				/* load the td sp */
    ldmia   sp!, {r0-r12, lr, ip}		/* reload r0 - r10 of the task */

    msr cpsr_c, #0xd3				/* change to system mode */
    movs pc, lr
.size	int_ker_exit, .-int_ker_exit

.ident	"GCC: (GNU) 4.0.2"