#include <comservers.h>
#include <nameserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>

void Com2PutServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = PUTC_NOTIFIER;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM2_PUT_SERVER);
    int data;
    int * uart2_ctrl = (int *)( UART2_BASE + UART_CTLR_OFFSET );
    int * uart2_data = (int *)( UART2_BASE + UART_DATA_OFFSET );

    FOREVER {
        AwaitEvent( EVENT_COM2_TRANSMIT );
        msg[0] = (char) data;
        // send evt to data
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        *uart2_data = (char) reply_struct.value[0];
    }
}

void Com1PutServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = PUTC_NOTIFIER;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM1_PUT_SERVER);
    int data;
    int * uart1_ctrl = (int *)( UART1_BASE + UART_CTLR_OFFSET );
    int * uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );

    FOREVER {
        AwaitEvent( EVENT_COM1_TRANSMIT );
        msg[0] = (char) data;
        // send evt to data
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        *uart1_data = (char) reply_struct.value[0];
        Delay(10);
    }
}

void PutServer() {
    // msg shits
    char msg[710] = {0};
    char reply[8] = {0};
    int sender_tid, msglen = 710;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    // current task blocked
    int blocked_notifier = -1;

    // string buffer
    char char_from_request;
    int insetion_point;

    // this is a circ buffer
    unsigned char string_buffer[BUFFER_SIZE];
    int buf_start = 0, buf_len = 0, i = 0;
    for (i = 0; i < BUFFER_SIZE; i++) {
        string_buffer[i] = 0;
    }

    Receive( &sender_tid, (char*)&msg_struct, msglen );
    Reply (sender_tid, (char *)&reply_struct, msglen);

    switch(msg_struct.iValue) {
        case COM1:
            RegisterAs(COM1_PUT_SERVER);
            // Create Notifier and send any initialization data
            Create(1, (&Com1PutServerNotifier));
            break;
        case COM2:
            RegisterAs(COM2_PUT_SERVER);
            // Create Notifier and send any initialization data
            Create(1, (&Com2PutServerNotifier));
            break;
        default:
            Exit();
    }

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case PUTC_REQUEST:

                // if a task wanted something and is blocked, we return it to it
                char_from_request = msg[0];
                if (blocked_notifier != -1) {
                    reply[0] = char_from_request;
                    Reply (blocked_notifier, (char *)&reply_struct, msglen);
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                    blocked_notifier = -1;
                }
                else {
                    // put char on the buffer
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_request;
                    buf_len++;
                    // don't really care what we reply
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                }
                break;
            case PUTSTR_REQUEST:
                i=0;
                while ( (char_from_request = msg[i++]) ) {
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_request;
                    buf_len++;
                }
                // if a task wanted something and is blocked, we return it to it
                if (blocked_notifier != -1) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (blocked_notifier, (char *)&reply_struct, msglen);
                    blocked_notifier = -1;
                }
                // don't really care what we reply
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            case PUTC_NOTIFIER:
                // read char from the buffer
                // if there is char on the buffer, return it
                if (buf_len > 0) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                }
                else {
                    // task is blocked;
                    blocked_notifier = sender_tid;
                }
                break;
            default:
                // shit
                break;
        }
    }
}

void Com1GetServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = GETC_NOTIFIER;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM1_GET_SERVER);
    char data;

    FOREVER {
        data =(char) AwaitEvent( EVENT_COM1_RECEIVE );
        msg[0] = (char) data;
        // send evt to data
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
}

void Com2GetServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = GETC_NOTIFIER;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM2_GET_SERVER);
    char data;

    FOREVER {
        data =(char) AwaitEvent( EVENT_COM2_RECEIVE );
        msg[0] = (char) data;
        // send evt to data
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
}

// assuming only 1 thing is gonna call get
void GetServer() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int sender_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    // current task blocked
    int blocked_task = -1;

    // string buffer
    char char_from_notifier;
    int insetion_point;

    // this is a circ buffer
    unsigned char string_buffer[BUFFER_SIZE];
    int buf_start = 0, buf_len = 0, i = 0;
    for (i = 0; i < BUFFER_SIZE; i++) {
        string_buffer[i] = 0;
    }

    Receive( &sender_tid, (char*)&msg_struct, msglen );
    Reply (sender_tid, (char *)&reply_struct, msglen);

    switch(msg_struct.iValue) {
        case COM1:
            RegisterAs(COM1_GET_SERVER);
            // Create Notifier and send any initialization data
            Create(1, (&Com1GetServerNotifier));
            break;
        case COM2:
            RegisterAs(COM2_GET_SERVER);
            // Create Notifier and send any initialization data
            Create(1, (&Com2GetServerNotifier));
            break;
        default:
            Exit();
    }

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case GETC_NOTIFIER:
                char_from_notifier = msg[0];
                // if a task wanted something and is blocked, we return it to it
                if (blocked_task != -1) {
                    reply[0] = char_from_notifier;
                    Reply (blocked_task, (char *)&reply_struct, msglen);
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                    blocked_task = -1;
                }
                else {
                    // put char on the buffer
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_notifier;
                    buf_len++;
                    // don't really care what we reply
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                }
                break;
            case GETC_REQUEST:
                // read char from the buffer
                // if there is char on the buffer, return it
                if (buf_len > 0) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                }
                else {
                    // task is blocked;
                    blocked_task = sender_tid;
                }
                break;
            default:
                // shit
                break;
        }
    }
}

void putc(int COM, char c) {
    char msg[2] = {0};
    char reply[64] = {0};
    int msglen = 2;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    reply_struct.iValue = 0;
    msg[0] = c;
    msg_struct.type = PUTC_REQUEST;

    switch (COM){
        case COM1:
            receiver_tid = WhoIs(COM1_PUT_SERVER);
            break;
        case COM2:
            receiver_tid = WhoIs(COM2_PUT_SERVER);
            break;
        default:
            break;
    }

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    return ;
}

void putstr(int COM, char* str ) {
    char msg[710] = {0};
    char reply[64] = {0};
    int msglen = 710;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    reply_struct.iValue = 0;
    strcpy(msg, str);
    msg_struct.type = PUTSTR_REQUEST;

    switch (COM){
        case COM1:
            receiver_tid = WhoIs(COM1_PUT_SERVER);
            break;
        case COM2:
            receiver_tid = WhoIs(COM2_PUT_SERVER);
            break;
        default:
            break;
    }
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    return ;
}

char getc(int COM) {
    char msg[2] = {0};
    char reply[2] = {0};
    int msglen = 2;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    reply_struct.iValue = 0;
    msg_struct.type = GETC_REQUEST;

    switch (COM){
        case COM1:
            receiver_tid = WhoIs(COM1_GET_SERVER);
            break;
        case COM2:
            receiver_tid = WhoIs(COM2_GET_SERVER);
            break;
        default:
            break;
    }

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    return reply_struct.value[0];
}
