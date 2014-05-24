 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>
#include <kernel.h>

	// asm ("mov r4, #0x28");
	// asm ("ldr r5, [r4]");
	// asm ("mov r0, #1");
	// asm ("mov r1, r5");
	// asm ("bl bwputr");

void FirstUserTask (){
	bwprintf( COM2, "first task: BILL\n\r");
    asm ("swi");
    bwprintf( COM2, "first task: HE\n\r");

	// for (;;) {
	// 	bwprintf( COM2, "first task: byebye\n\r");
	// 	bwprintf( COM2, "first task: HIIII\n\r");
	// }
}

int schedule ( int* tds ) {
	return tds[0];
}

int get_free_td (unsigned int* free_list_lo, unsigned int* free_list_hi) {
	int i;
	for ( i = 0; i < 0x20; i++ ) {
		if ( !((*free_list_lo) & (1 << i)) ) {
			*free_list_lo = *free_list_lo | (1 << i);
			return i;
		}
	}
	for ( i = 0; i < 0x20; i++ ) {
		if ( !((*free_list_hi) & (1 << i)) ) {
			*free_list_hi = *free_list_hi | (1 << i);
			return 32+i;
		}
	}
}

// void ker_entry () {
// 	// get arguments
//     asm ("mov r3, lr");
//     asm ("msr cpsr_c, #0xdf");
//     asm ("stmdb   sp!, {r4-r10}");
//     asm ("mov r4, sp");
//     asm ("msr cpsr_c, #0xd3");
//     asm ("ldmia   sp!, {r0-r12, lr}");
    

//     asm ("str r3, [r0, #0x4]");
//     asm ("mrs r2, spsr");
//     asm ("str r2, [r0, #0xc]");
//     asm ("str r4, [r0, #0x8]");
//      // bwprintf( COM2, "testContext: end\n\r" );
// }

// void ker_exit ( td *active, int req ) {
//     // register int *r0 asm ("r0");
//     // *r0 = (int) &active;
//     // asm ("ldr r1, [r0, #8]");
//     // asm ("mov r0, #1"); //debug
//     // asm ("bl bwputr"); //debug
    
//     // DON'T BWPRINNTF, IT FUCKS EVERYTHING UP
//     asm ("mov     ip, sp");
//     asm ("stmdb   sp!, {r0-r12, lr}" );

//     asm ("msr cpsr_c, #0xdf");
//     asm ("mov   r1, r0");
//     asm ("ldr sp, [r1, #8]");
//     asm ("ldmia   sp, {r4-r10}");
//     asm ("ldr r0, [r1, #0x10]");
//     asm ("msr cpsr_c, #0xd3");

//     // asm ("mov r0, #1"); //debug
//     // asm ("ldr r1, [r1, #0xc]"); //debug
//     // asm ("bl bwputr"); //debug
    
//     asm ("ldr r2, [r1, #0xc]");
//     asm ("msr cpsr_c, r2");
//     asm ("ldr pc, [r1, #4]");

// }

int initialize ( td tds[64], unsigned int* free_list_lo, unsigned int* free_list_hi) {
    // place the svc_handler to jump table
    void (*syscall)();
    syscall = (void *) (CODE_OFFSET+(&ker_entry));
    int *handler;
    handler = (void*)0x28;
    *handler = (int) syscall;

    // initialize the FirstUserTask
    //int tid = get_free_td(free_list_lo, free_list_hi);
    tds[0].tid = 0;
    tds[0].pc = CODE_OFFSET + (&FirstUserTask);
    tds[0].sp = 0x1000000;
    tds[0].spsr = 0xdf;
    return 0;
}

int main( int argc, char* argv[] ) {
    td *active;
    td tds[64];
    unsigned int free_list_lo, free_list_hi;
    int tid = initialize( tds, &free_list_lo, &free_list_hi );
    int i, ret;
    for ( i = 0; i<1; i++ ) {
        active = (tds);
       // bwprintf( COM2, "testContext: begin\n\r" );
        ker_exit ( active, 1 );
       // bwprintf( COM2, "testContext: begin\n\r" );
    
    //  ret = handle( REQUEST_CREATE );
    }
    return 0;
}
