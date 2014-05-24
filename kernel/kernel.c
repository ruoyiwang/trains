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
    bwprintf( COM2, "first task: ROY\n");
    while (1) {
        bwprintf( COM2, "first task: BILL1\n");
        asm ("swi");
        bwprintf( COM2, "first task: HE\n");
        asm ("swi");
        bwprintf( COM2, "first task: BILL2\n");
        asm ("swi");
        bwprintf( COM2, "first task: HE\n");
        asm ("swi");
        bwprintf( COM2, "first task: BILL3\n");
        asm ("swi");
        bwprintf( COM2, "first task: HE\n");
        asm ("swi");
        bwprintf( COM2, "first task: BILL4\n");
        asm ("swi");
        bwprintf( COM2, "first task: HE\n");
        asm ("swi");
    }
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
    for ( i = 0; i<50; i++ ) {
        active = (tds);
       // bwprintf( COM2, "testContext: begin\n\r" );
        ker_exit ( active, 1 );
       // bwprintf( COM2, "testContext: begin\n\r" );
    
    //  ret = handle( REQUEST_CREATE );
    }
    return 0;
}
