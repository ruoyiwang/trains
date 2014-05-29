 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>
#include <kernel.h>
#include <nameserver.h>
#include <queue.h>
#include <Tasks.h>
#include <util.h>

int schedule ( td_queue td_pq[16] ) {
	int i = 0;
    // 0 is the highest pri
    for (i = 0; i < 16; i++) {
        // if the front of the queue is not null
        if (td_pq[i].front != NULL) {
            td_pq[i].front->state = STATE_ACTIVE;

            return pq_pop_front(td_pq, i);
        }
    }
    // if I didn't find anything return null;
    return -1;
}

int MyTid() {
    asm ("swi 5");
}

int MyParentTid() {
    asm ("swi 6");
}

int Create( int priority,  void (* pc) ()) {
    asm ("swi 2");
}

int Pass() {
    asm ("swi 7");
}

int Exit() {
    asm ("swi 8");
}

int Send ( int tid, char *msg, int msglen, char * reply, int replylen ) {
    asm ("swi 11");
}

int Receive ( int *tid, char *msg, int msglen ) {
    asm ("swi 12");
}

int Reply ( int tid, char *msg, int msglen ) {
    asm ("swi 13");
}

int get_free_td (unsigned int* free_list_lo, unsigned int* free_list_hi) {
	int i;
	for ( i = 0; i < 0x20; i++ ) {
		if ( !((*free_list_lo) & (1 << i)) ) {
			*free_list_lo = (*free_list_lo) | (1 << i);
			return i;
		}
	}
	for ( i = 0; i < 0x20; i++ ) {
		if ( !((*free_list_hi) & (1 << i)) ) {
			*free_list_hi = (*free_list_hi) | (1 << i);
			return 32+i;
		}
	}
    return -1;
}

int initialize_td(
    int pri, 
    unsigned int* free_list_lo, 
    unsigned int * free_list_hi, 
    unsigned int pc,
    td tds[64],
    td_queue td_pq[16], 
    int parent_tid
) {
    int tid = get_free_td(free_list_lo, free_list_hi);
    tds[tid].pc = pc;
    tds[tid].priority = pri;
    tds[tid].parent_tid = parent_tid;

    // stick this at the end of the pq
    pq_push_back(td_pq, tds, tid);

    // return tid
    return tid;
}

void initialize (td tds[64]) {
    int i = 0;
    // place the svc_handler to jump table
    void (*syscall)();
    syscall = (void *) (CODE_OFFSET+(&ker_entry));
    int *handler;
    handler = (void*)0x28;
    *handler = (int) syscall;

    // initialize all the tds;
    for (i = 0 ; i < 64; i++) {
        tds[i].tid          = i;
        tds[i].sp           = USER_STACK_BEGIN + i * 4 * USER_STACK_SIZE;
        tds[i].spsr         = 0xd0;
        tds[i].ret          = 0;
        tds[i].priority     = 15;
        tds[i].parent_tid   = -1;
        tds[i].state        = STATE_READY;
        tds[i].next         = NULL;
        tds[i].msg.has_msg  = 0;
        tds[i].msg.sender_tid  = -1;
    }

    return;
}

void handle (td *active, int req, int args[5],
            unsigned int* free_list_lo, 
            unsigned int * free_list_hi, 
            td tds[64],
            td_queue td_pq[16] )   {

    switch ( req ) {
        case 5:
            active->ret = active->tid;
            break;
        case 2:
            active->ret = initialize_td( args[0], free_list_lo, free_list_hi, args[1], tds, td_pq, active->tid);
            break;
        case 6:
            active->ret = active->parent_tid;
            break;
        case 7:
            break;
        case 8:
            active->state = STATE_ZOMBIE;
            break;
        case 11:    //Send
            if ( tds[args[0]].state == STATE_SND_BLK ) {    //if receive first
                *tds[args[0]].msg.sender_tid = active->tid;
                strcpy(tds[args[0]].msg.msg->value, ((message *)args[1])->value);
                tds[args[0]].msg.msg->type = ((message *)args[1])->type;
                tds[args[0]].msg.msg_len = args[2];
                tds[args[0]].msg.rpl = args[3];
                tds[args[0]].msg.rpl_len = args[4];
                tds[args[0]].msg.has_msg = 0;
                pq_push_back(td_pq, tds, args[0]);
                active->state = STATE_RPL_BLK;
            }
            else {          //if no receive yet
                tds[args[0]].msg.sender_tid = &(active->tid);
                tds[args[0]].msg.msg = args[1];
                tds[args[0]].msg.msg_len = args[2];
                tds[args[0]].msg.rpl = args[3];
                tds[args[0]].msg.rpl_len = args[4];
                tds[args[0]].msg.has_msg = 1;
                active->state = STATE_RCV_BLK;
            }
            break;
        case 12:    //Receive
            if ( active->msg.has_msg ) {//tds[*active->msg.sender_tid].state == STATE_RCV_BLK ) {    //if send first
                *((int *)args[0]) = *active->msg.sender_tid;
                strcpy(((message *)args[1])->value, active->msg.msg->value);
                ((message *)args[1])->type = tds[args[0]].msg.msg->type;
                active->msg.has_msg = 0;
                tds[*active->msg.sender_tid].state = STATE_RPL_BLK;
            }
            else {      //waiting on send
                active->msg.sender_tid = (int *) args[0];
                active->msg.msg = args[1];
                active->msg.msg_len = args[2];
                active->msg.has_msg = 0;
                active->state = STATE_SND_BLK;
            }
            break;
        case 13:    //Reply
            if ( tds[args[0]].state == STATE_RPL_BLK ) {
                strcpy(active->msg.rpl->value, ((message *)args[1])->value);
                active->msg.rpl->type = ((message *)args[1])->type;
                pq_push_back(td_pq, tds, args[0]);
            }
            break;
    }
}

int main( int argc, char* argv[] ) {
    td *active;
    td tds[64];
    message messages[64];
    td_queue td_pq[16];
    int args[5];

    unsigned int free_list_lo = 0, free_list_hi = 0;
    initialize(tds);
    initialize_td_pq(td_pq);
    int tid = initialize_td(2, &free_list_lo, &free_list_hi, CODE_OFFSET + (&FirstUserTask), tds, td_pq, -1);

    int i, req = 0;
    for (;;) {
        tid = schedule(td_pq);
        if (tid == -1) break;
        active = (tds + tid);
        req = ker_exit ( active, (int *) args );
        // put the task back on the queue
        req = req & 0xf;
        handle( active, req, args, &free_list_lo, &free_list_hi, tds, td_pq );
        if (active->state == STATE_READY || active->state == STATE_ACTIVE ){
            pq_push_back(td_pq, tds, active->tid);   
        }
    }
    return 0;
}
