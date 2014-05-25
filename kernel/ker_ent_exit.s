.file	"ker_ent_exit.c"
.text

.global	ker_entry
.type	ker_entry, %function
ker_entry:
	mov r5, r1
	ldmia sp!, {r1}
	str r0, [r1]
	str r5, [r1, #0x4]
	str r2, [r1, #0x8]
	str r3, [r1, #0xc]

	mov r3, lr 						/* save the lr to r3 */
	msr cpsr_c, #0xdf 				/* change to system mode */
	stmdb   sp!, {r4-r12, lr} 			/* store the task registers */
	mov ip, sp 						/* save the sp to r1 */
	msr cpsr_c, #0xd3 				/* change back to superviser mode */

 	ldmia   sp!, {r0}			 	/* pop the kernal stack */
	str r3, [r0, #0x4] 				/* write to the lr of the TD */
	mrs r2, spsr 					/* save the spsr to r2 */
	str r2, [r0, #0xc]				/* store spsr to TD */
	str ip, [r0, #0x8]				/* store the sp to the TD */ 
	ldr r0, [r3, #-4]				/* get the swi arguemnt and return it */
	ldmia   sp!, { r4-r12, lr} 		/* pop the kernal stack */
	mov pc, lr 						/* this go back to kernel */
.size	ker_entry, .-ker_entry

.global	ker_exit
.type	ker_exit, %function
ker_exit:				 
	stmdb   sp!, { r4-r12, lr}		/* move registers to the kernal stack */
    stmdb   sp!, {r0}				/* move registers to the kernal stack */
 	stmdb   sp!, {r1}				/* move registers to the kernal stack */
	
    msr cpsr_c, #0xdf				/* change to system mode */
    mov   r1, r0 					/* move r0 which is the active task to r1 */
    ldr sp, [r1, #8]				/* load the td sp */
    ldmia   sp!, {r4-r12, lr}			/* reload r4 - r10 of the task */
    ldr r0, [r1, #0x10] 			/* load the return value into r0 */
    msr cpsr_c, #0xd3 				/* change back to superviser mode */
    
    ldr r2, [r1, #0xc]				/* load the task spsr bak into cpsr */
    msr cpsr, r2 					/* TODO: need to investigate this line */
    ldr pc, [r1, #4] 				/* load the pc of the td */
.size	ker_exit, .-ker_exit

.ident	"GCC: (GNU) 4.0.2"