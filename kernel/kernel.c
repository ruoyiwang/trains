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
    while (1) {
        int tid = MyTid();
        bwprintf( COM2, "My TID is: %d\n", tid);
    }
}

void spawnedTask () {
    while (1) {
        bwprintf( COM2, "spawned task: ROY\n");
    }
}

td* schedule ( td_queue td_pq[16] ) {
	int i = 0;
    // 0 is the highest pri
    for (i = 0; i < 16; i++) {
        td_queue cur_queue = td_pq[i];

        // if the front of the queue is not null
        if (cur_queue.front != NULL) {
            // we return the front of the queue
            td* returning_td = cur_queue.front;
            // pop it at the front
            cur_queue.front = cur_queue.front->next;
            return returning_td;
        }
    }
    // if I didn't fine anything return null;
    return NULL;
}

void initialize_td_pq(td_queue td_pq[16]) {
    // set pq to nulls
    int i = 0;
    for (i = 0; i < 16; i++) {
        td_pq[i].front = NULL;
        td_pq[i].back = NULL;
    }
}

int MyTid() {
    asm ("swi 3");
}

int Create( int priority, int * pc) {

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
    tds[0].tid = 3;
    tds[0].pc = CODE_OFFSET + (&FirstUserTask);
    tds[0].sp = 0x1000000;
    tds[0].spsr = 0xdf;
    return 0;
}

void handle (td *active, int req ) {
    switch ( req ) {
        case 3:
            active->ret = active->tid;
            break;
    }
}

int main( int argc, char* argv[] ) {
    td *active;
    td tds[64];
    td_queue td_pq[16];
    
    unsigned int free_list_lo, free_list_hi;
    int tid = initialize( tds, &free_list_lo, &free_list_hi );
    int i, ret, req = 0;
    for ( i = 0; i<10; i++ ) {
        active = (tds);
        req = ker_exit ( active );  
        req = req & 0xf;
        handle( active, req );
    }
    return 0;
}
