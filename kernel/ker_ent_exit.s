.file	"ker_ent_exit.c"
.text

.global	ker_entry
.type	ker_entry, %function
ker_entry:
	mov r3, lr
	msr cpsr_c, #0xdf
	stmdb   sp!, {r4-r10}
	mov r1, sp
	msr cpsr_c, #0xd3

    ldmia   sp!, {r0, r4-r12, lr}
	str r3, [r0, #0x4]
	mrs r2, spsr
	str r2, [r0, #0xc]
	str r1, [r0, #0x8]
	mov pc, lr
.size	ker_entry, .-ker_entry

.global	ker_exit
.type	ker_exit, %function
ker_exit:
	mov     ip, sp
    stmdb   sp!, {r0, r4-r12, lr}

    msr cpsr_c, #0xdf
    mov   r1, r0
    ldr sp, [r1, #8]
    ldmia   sp!, {r4-r10}
    ldr r0, [r1, #0x10]
    msr cpsr_c, #0xd3
    
    ldr r2, [r1, #0xc]
    msr cpsr_c, r2
    ldr pc, [r1, #4]
.size	ker_exit, .-ker_exit

.ident	"GCC: (GNU) 4.0.2"