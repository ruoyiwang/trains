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
#include <debug.h>

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

int IdleUsage() {
    asm ("swi 15");
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

int AwaitEvent( int eventId ) {
    asm ("swi 9");
}

int _send ( int tid, mailbox *mail ) {
    asm ("swi 11");
}

int Send ( int tid, char *msg, int msglen, char * reply, int replylen ) {
    mailbox mail;
    mail.msg = msg;
    mail.msg_len = msglen;
    mail.rpl = reply;
    mail.rpl_len = replylen;
    mail.next = NULL;
    return _send (tid, &mail);
}

int _receive ( mailbox *mail ) {
    asm ("swi 12");
}

int Receive ( int *tid, char *msg, int msglen ) {
    mailbox mail;
    mail.sender_tid = tid;
    mail.msg = msg;
    mail.msg_len = msglen;
    mail.next = NULL;
    return _receive ( &mail );
}

int Reply ( int tid, char *msg, int msglen ) {
    asm ("swi 13");
}

int Assert ( ) {
    asm ("swi 14");
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

void calculate_idle_usage( int *before_idle, int *time_idled, int *current_frame, int *usage) {
    volatile unsigned int * timer_4_low;
    timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );
    int after_idle = *timer_4_low;
    int frame_size = after_idle - *current_frame;
    *time_idled += after_idle - *before_idle;
    if ( frame_size > 1000000 ) {
        *usage = *time_idled * 100 / frame_size;
        *current_frame = * timer_4_low;
        *time_idled = 0;
    }
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

void initialize_interrupts() {
    int * VIC2Enable =(int *) (VIC2_BASE + VICxIntEnable);
    int * VIC1Enable =(int *) (VIC1_BASE + VICxIntEnable);
    *VIC2Enable = *VIC2Enable | (1 << 19);
    *VIC2Enable = *VIC2Enable | (1 << 20);
    *VIC2Enable = *VIC2Enable | (1 << 22);
    // *VIC1Enable = *VIC1Enable | (1 << 23);
    // *VIC1Enable = *VIC1Enable | (1 << 24);
    // *VIC1Enable = *VIC1Enable | (1 << 25);
    // *VIC1Enable = *VIC1Enable | (1 << 26);

    int * uart2_ctrl = (int *)( UART2_BASE + UART_CTLR_OFFSET );
    int * uart1_ctrl = (int *)( UART1_BASE + UART_CTLR_OFFSET );

    uart_noops();
    * uart2_ctrl = * uart2_ctrl | RIEN_MASK;
    uart_noops();
    * uart2_ctrl = * uart2_ctrl | UARTEN_MASK;
    uart_noops();
    * uart1_ctrl = * uart1_ctrl | RIEN_MASK;
    uart_noops();
    * uart1_ctrl = * uart1_ctrl | UARTEN_MASK;
}

void uninitialize() {
    int * timer3ctrl = (int *)( TIMER3_BASE + CRTL_OFFSET );
    * timer3ctrl = 0;
}

void initialize (td tds[64], int event_blocked_tds[5]) {
    initTimers();
    TurnCacheOn();
    int i = 0;
    // place the svc_handler to jump table
    void (*syscall)();
    syscall = (void *) (&ker_entry);
    int *handler;
    handler = (void*)0x28;
    *handler = (int) syscall;

    syscall = (void *) (&int_ker_entry);
    handler = (void *) 0x38;
    *handler = (int) syscall;
    initialize_interrupts();
    volatile _tds = tds;

    // set SHena to enable halt mode
    int *device_cfg = (int *) DEVICE_CFG;
    *device_cfg = *device_cfg | SHENA_MASK;

    // initialize all the tds;
    for (i = 0 ; i < 64; i++) {
        tds[i].tid          = i;
        tds[i].sp           = USER_STACK_BEGIN - i * 4 * USER_STACK_SIZE;
        tds[i].spsr         = 0x5f;
        tds[i].ret          = 0;
        tds[i].priority     = 15;
        tds[i].parent_tid   = -1;
        tds[i].state        = STATE_READY;
        tds[i].next         = NULL;
        tds[i].sendQ        = NULL;
        tds[i].flags        = 0;
    }
    for (i = 0 ; i < 5; i++) {
        event_blocked_tds[i] = 0;
    }
    return;
}

void handle (td *active, int req, int args[5],
            unsigned int* free_list_lo,
            unsigned int * free_list_hi,
            td tds[64],
            td_queue td_pq[16],
            int event_blocked_tds[5],
            int interrupt_queue[5],
            int idle_usage,
            unsigned int *uart1_tx_flag,
            unsigned int *uart1_cts_flag )   {
    int i, *timer3clear, *VIC2Status, *VIC1Status;
    char c;
    int *uart2_flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
    int *uart2_data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    int * uart2_ctrl = (int *)( UART2_BASE + UART_CTLR_OFFSET );
    int * uart2_intr = (int *)( UART2_BASE + UART_INTR_OFFSET );

    int *uart1_flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    int *uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    int * uart1_ctrl = (int *)( UART1_BASE + UART_CTLR_OFFSET );
    int * uart1_intr = (int *)( UART1_BASE + UART_INTR_OFFSET );

    for (i = 0; i<5 ; i++) {
        active->args[i] = args[i];
    }

    switch ( req ) {
        case 20:    //interrupts
            VIC2Status = (int *) (VIC2_BASE + VICxIRQStatus);
            VIC1Status = (int *) (VIC1_BASE + VICxIRQStatus);

            if (*VIC2Status & (1 << 19) ){
                timer3clear = (int *) ( TIMER3_BASE + CLR_OFFSET );
                *timer3clear = 1;
                if (event_blocked_tds[EVENT_CLOCK]) {
                    pq_push_back(td_pq, tds, ((td *) event_blocked_tds[EVENT_CLOCK])->tid);
                    event_blocked_tds[EVENT_CLOCK] = 0;
                }
                else {
                    interrupt_queue[EVENT_CLOCK] = 1;
                }
            }
            if (*VIC2Status & (1 << 20)) {
                if (*uart1_intr & TIS_MASK ){
                    *uart1_tx_flag = 1;
                    uart_noops();
                    *uart1_ctrl = * uart1_ctrl & ~TIEN_MASK;
                }
                if (*uart1_intr & MIS_MASK ){
                    *uart1_intr = 0;
                    // assert_ker(tds, td_pq);
                    if (*uart1_flags & CTS_MASK ) {
                        uart_noops();
                        *uart1_ctrl = * uart1_ctrl & ~MSIEN_MASK;
                        *uart1_cts_flag = 1;
                    }
                }
                if ( *uart1_tx_flag && *uart1_cts_flag) {
                    *uart1_cts_flag = 0;
                    *uart1_tx_flag = 0;
                    if (event_blocked_tds[EVENT_COM1_TRANSMIT]) {
                        pq_push_back(td_pq, tds, ((td *) event_blocked_tds[EVENT_COM1_TRANSMIT])->tid);
                        event_blocked_tds[EVENT_COM1_TRANSMIT] = 0;
                    }
                    else {
                        interrupt_queue[EVENT_COM1_TRANSMIT] = 1;
                    }
                }

                if (*uart1_intr & RIS_MASK ){
                    if (event_blocked_tds[EVENT_COM1_RECEIVE]) {
                        ((td *) event_blocked_tds[EVENT_COM1_RECEIVE])->ret = *uart1_data;
                        pq_push_back(td_pq, tds, ((td *) event_blocked_tds[EVENT_COM1_RECEIVE])->tid);
                        event_blocked_tds[EVENT_COM1_RECEIVE] = 0;
                    }
                    else {
                        interrupt_queue[EVENT_COM1_TRANSMIT] = *uart1_data;
                    }
                }
            }
            if (*VIC2Status & (1 << 22)) {
                if (*uart2_intr & TIS_MASK ){
                    if (event_blocked_tds[EVENT_COM2_TRANSMIT]) {
                        uart_noops();
                        *uart2_ctrl = * uart2_ctrl & ~TIEN_MASK;
                        pq_push_back(td_pq, tds, ((td *) event_blocked_tds[EVENT_COM2_TRANSMIT])->tid);
                        event_blocked_tds[EVENT_COM2_TRANSMIT] = 0;
                    }
                    else {
                        interrupt_queue[EVENT_COM2_TRANSMIT] = 1;
                    }
                }
                if (*uart2_intr & RIS_MASK ){
                    if (event_blocked_tds[EVENT_COM2_RECEIVE]) {
                        ((td *) event_blocked_tds[EVENT_COM2_RECEIVE])->ret = *uart2_data;
                        pq_push_back(td_pq, tds, ((td *) event_blocked_tds[EVENT_COM2_RECEIVE])->tid);
                        event_blocked_tds[EVENT_COM2_RECEIVE] = 0;
                    }
                    else {
                        interrupt_queue[EVENT_COM2_RECEIVE] = *uart2_data;
                    }
                }
            }
            break;
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
        case 9:     // wait
            if (args[0] == EVENT_CLOCK && interrupt_queue[EVENT_CLOCK]) {
                interrupt_queue[EVENT_CLOCK] = 0;
            }
            else if (args[0] == EVENT_COM2_RECEIVE && interrupt_queue[EVENT_COM2_RECEIVE]) {
                active->ret = interrupt_queue[EVENT_COM2_RECEIVE];
                interrupt_queue[EVENT_COM2_RECEIVE] = 0;
            }
            else if (args[0] == EVENT_COM2_TRANSMIT && interrupt_queue[EVENT_COM2_TRANSMIT]) {
                interrupt_queue[EVENT_COM2_TRANSMIT] = 0;
            }
            else if (args[0] == EVENT_COM1_RECEIVE && interrupt_queue[EVENT_COM1_RECEIVE]) {
                active->ret = interrupt_queue[EVENT_COM1_RECEIVE];
                interrupt_queue[EVENT_COM1_RECEIVE] = 0;
            }
            else if (args[0] == EVENT_COM1_TRANSMIT && interrupt_queue[EVENT_COM1_TRANSMIT]) {
                interrupt_queue[EVENT_COM1_TRANSMIT] = 0;
            }
            else {
                if (args[0] == EVENT_COM2_TRANSMIT) {
                    uart_noops();
                    *uart2_ctrl = * uart2_ctrl | TIEN_MASK;
                }
                if (args[0] == EVENT_COM1_TRANSMIT) {
                    uart_noops();
                    *uart1_ctrl = * uart1_ctrl | TIEN_MASK | MSIEN_MASK;
                    uart1_cts_flag = *uart1_flags & CTS_MASK;
                }
                active->state = STATE_EVT_BLK;
                event_blocked_tds[args[0]] = active;
            }
            break;
        case 11:    //Send
            if ( tds[args[0]].state == STATE_SND_BLK ) {    //if receive first
                *(tds[args[0]].sendQ->sender_tid) = active->tid;
                if (tds[args[0]].sendQ->msg_len < ((mailbox *)args[1])->msg_len) {
                     // bwprintf(COM2, "%c[2JRECEIVE %d %d %d\n", 0x1b,active->tid,tds[args[0]].sendQ->msg_len, ((mailbox *)args[1])->msg_len);
                    assert_ker(tds, td_pq);
                }
                memcpy(tds[args[0]].sendQ->msg->value, ((mailbox *)args[1])->msg->value, (unsigned int)((mailbox *)args[1])->msg_len);
                tds[args[0]].sendQ->msg->iValue = ((mailbox *)args[1])->msg->iValue;
                tds[args[0]].sendQ->msg->type = ((mailbox *)args[1])->msg->type;
                tds[args[0]].sendQ->msg_len = ((mailbox *)args[1])->msg_len;
                tds[args[0]].sendQ->rpl = ((mailbox *)args[1])->rpl;
                tds[args[0]].sendQ->rpl_len = ((mailbox *)args[1])->rpl_len;
                tds[args[0]].sendQ = NULL;
                pq_push_back(td_pq, tds, args[0]);
                active->state = STATE_RPL_BLK;
            }
            else {
                // bwprintf(COM2, "CRYING2\n");
                ((mailbox *)args[1])->sender_tid = &(active->tid);
                if (tds[args[0]].sendQ) {
                    mailbox *lastNode = tds[args[0]].sendQ;
                    while (lastNode->next ) {
                        lastNode = lastNode->next;
                    }
                    lastNode->next = (mailbox *)args[1];
                }
                else {
                    tds[args[0]].sendQ = (mailbox *)args[1];
                }
                active->state = STATE_RCV_BLK;
            }
            break;
        case 12:    //Receive
            if ( active->sendQ ) {    //if send first
                // bwprintf(COM2, "CRYING3\n");
                if (((mailbox*)args[0])->msg_len < active->sendQ->msg_len) {
                     // bwprintf(COM2, "%c[2JSEND %d %d %d\n", 0x1b,active->tid,((mailbox*)args[0])->msg_len, active->sendQ->msg_len);
                    assert_ker(tds, td_pq);
                }
                *((mailbox*)args[0])->sender_tid = *(active->sendQ->sender_tid);
                memcpy(((mailbox*)args[0])->msg->value, active->sendQ->msg->value, (unsigned int)active->sendQ->msg_len);
                ((mailbox*)args[0])->msg->iValue = active->sendQ->msg->iValue;
                ((mailbox*)args[0])->msg->type = active->sendQ->msg->type;
                tds[*(active->sendQ->sender_tid)].state = STATE_RPL_BLK;
                active->sendQ = active->sendQ->next;
            }
            else {      //waiting on send
                // bwprintf(COM2, "CRYING4\n");
                active->sendQ = (mailbox *)args[0];
                active->state = STATE_SND_BLK;
            }
            break;
        case 13:    //Reply
            if ( tds[args[0]].state == STATE_RPL_BLK ) {
                // bwprintf(COM2, "CRYING5\n");
                if (((mailbox *)(tds[args[0]].args[1]))->rpl_len < (unsigned int)args[2]) {
                     // bwprintf(COM2, "%c[2JREPLY %d %d %d\n", 0x1b,active->tid,((mailbox *)(tds[args[0]].args[1]))->rpl_len, (unsigned int)args[2]);
                    assert_ker(tds, td_pq);
                }
                memcpy(((mailbox *)(tds[args[0]].args[1]))->rpl->value, ((message *)args[1])->value, (unsigned int)args[2]);
                ((mailbox *)(tds[args[0]].args[1]))->rpl->iValue = ((message *)args[1])->iValue;
                ((mailbox *)(tds[args[0]].args[1]))->rpl->type = ((message *)args[1])->type;
                pq_push_back(td_pq, tds, args[0]);
            }
            break;
        case 14:
            assert_ker(tds, td_pq);
            break;
        case 15:
            active->ret = idle_usage;
            break;
    }
}

int main( int argc, char* argv[] ) {
    td *active;
    td tds[64];
    message messages[64];
    td_queue td_pq[16];
    int args[5];
    int event_blocked_tds[5];
    int interrupt_queue[5] = {0};
    unsigned int free_list_lo = 0, free_list_hi = 0;


    initialize(tds, event_blocked_tds);
    initialize_td_pq(td_pq);
    int tid = initialize_td(2, &free_list_lo, &free_list_hi, (&FirstUserTask), tds, td_pq, -1);
    // for tracking idle usage
    volatile unsigned int * timer_4_low;
    timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );
    unsigned int time_idled = 0, before_idle, idle_frame =*timer_4_low , idle_usage = 0;


    int *uart1_flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    unsigned int uart1_tx_flag = 0, uart1_cts_flag = *uart1_flags & CTS_MASK;

    int i, req = 0;
    for (;;) {
        tid = schedule(td_pq);
        if (tid == -1) break;
        if (tid == IDLE_TID ) {
            before_idle = *timer_4_low;
        }
        active = (tds + tid);
        if ( active->flags & 1 ) {
            req = int_ker_exit ( active, (int *) args );
            active->flags = active->flags & ~1;
        }
        else {
            req = ker_exit ( active, (int *) args );
        }

        // put the task back on the queue
        req = req & 0xff;
        handle( active, req, args,
            &free_list_lo, &free_list_hi,
            tds, td_pq, event_blocked_tds, interrupt_queue, idle_usage,
            &uart1_tx_flag, &uart1_cts_flag );

        if (active->state == STATE_READY || active->state == STATE_ACTIVE ){
            pq_push_back(td_pq, tds, active->tid);
        }
        if (req == 20){
            active->flags = active->flags | 1;
        }
        if (active->tid == IDLE_TID) {
            calculate_idle_usage( &before_idle, &time_idled, &idle_frame, &idle_usage);
        }
    }
    uninitialize();
    return 0;
}
