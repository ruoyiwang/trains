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

int schedule ( td_queue td_pq[16] ) {
	int i = 0;
    // 0 is the highest pri
    for (i = 0; i < 16; i++) {
        // if the front of the queue is not null
        if (td_pq[i].front != NULL) {
            return pq_pop_front(td_pq, i);
            return pq_pop_front(td_pq, i);
        }
    }
    // if I didn't find anything return null;
    return -1;
}

void initialize_td_pq(td_queue td_pq[16]) {
    // set pq to nulls
    int i = 0;
    for (i = 0; i < 16; i++) {
        td_pq[i].front = NULL;
        td_pq[i].back = NULL;
    }
}

int pq_pop_front(td_queue td_pq[16], int pri) {
    if (td_pq[pri].front == NULL) {
        return -1;
    }
    else if (td_pq[pri].front == td_pq[pri].back) {
        int tid = td_pq[pri].front->tid;
        td_pq[pri].front = NULL;
        td_pq[pri].back = NULL;
        return tid;
    }
    else {
        int tid = td_pq[pri].front->tid;
        td_pq[pri].front = td_pq[pri].front->next;
        return tid;
    }
    return -1;
}

void pq_push_back(td_queue td_pq[16], td tds[64], int tid) {
    int pri = tds[tid].priority;
    if (td_pq[pri].front == NULL) {
        td_pq[pri].front = tds + tid;
        td_pq[pri].back = tds + tid;
    }
    else {
        td_pq[pri].back->next = tds + tid;
        td_pq[pri].back = tds + tid;
    }
}

int initialize_td(
    int pri, 
    unsigned int* free_list_lo, 
    unsigned int * free_list_hi, 
    unsigned int pc,
    td tds[64],
    td_queue td_pq[16]
) {
    int tid = get_free_td(free_list_lo, free_list_hi);
    tds[tid].tid = tid;
    tds[tid].pc = pc;
    tds[tid].sp = 0x1000000 + tid * 4 * 128;
    tds[tid].spsr = 0xdf;
    tds[tid].ret = 0;
    tds[tid].priority = pri;
    tds[tid].state = 0;
    tds[tid].next = 0;

    // stick this at the end of the pq
    pq_push_back(td_pq, tds, tid);

    // return tid
    return tid;
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

void initialize () {
    // place the svc_handler to jump table
    void (*syscall)();
    syscall = (void *) (CODE_OFFSET+(&ker_entry));
    int *handler;
    handler = (void*)0x28;
    *handler = (int) syscall;

    // initialize the FirstUserTask
    //int tid = get_free_td(free_list_lo, free_list_hi);
    // tds[0].tid = 3;
    // tds[0].pc = CODE_OFFSET + (&FirstUserTask);
    // tds[0].sp = 0x1000000;
    // tds[0].spsr = 0xdf;
    return;
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
    
    unsigned int free_list_lo = 0, free_list_hi = 0;
    initialize();
    initialize_td_pq(td_pq);
    int tid = initialize_td(0, &free_list_lo, &free_list_hi, CODE_OFFSET + (&FirstUserTask), tds, td_pq);

    int i, ret, req = 0;
    for ( i = 0; i<10; i++ ) {
        // bwprintf ( COM2, "%d\n", schedule(td_pq) );
        tid = schedule(td_pq);
        active = (tds + tid);
        req = ker_exit ( active );
        // put the task back on the queue
        req = req & 0xf;
        handle( active, req );
        pq_push_back(td_pq, tds, tid);
    }
    return 0;
}
