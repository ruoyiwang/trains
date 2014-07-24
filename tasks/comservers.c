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
    int receiver_tid, msglen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM2_PUT_SERVER);
    int data;
    // int * uart2_ctrl = (int *)( UART2_BASE + UART_CTLR_OFFSET );
    int * uart2_data = (int *)( UART2_BASE + UART_DATA_OFFSET );

    FOREVER {
        msglen = 2;
        AwaitEvent( EVENT_COM2_TRANSMIT );
        msg[0] = (char) data;
        // send evt to data
        msg_struct.type = PUTC_NOTIFIER;
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        *uart2_data = (char) reply_struct.value[0];
    }
}

void Com1PutServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM1_PUT_SERVER);
    int data;
    // int * uart1_ctrl = (int *)( UART1_BASE + UART_CTLR_OFFSET );
    int * uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );

    FOREVER {
        msglen = 2;
        AwaitEvent( EVENT_COM1_TRANSMIT );
        msg[0] = (char) data;
        // send evt to data
        msg_struct.type = PUTC_NOTIFIER;
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        *uart1_data = (char) reply_struct.value[0];
        // Delay(10);
    }
}

void PutServer() {
    // msg shits
    char msg[1010] = {0};
    char reply[2] = {0};
    int sender_tid, msglen = 1010, rpllen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    char *copystr;

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
    Reply (sender_tid, (char *)&reply_struct, rpllen);
    int copy_len;

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
            bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll COMSERVEREXIT %d", msg_struct.type);
            Assert();
            Exit();
    }

    FOREVER {
        msglen = 1010, rpllen = 2;
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case PUTC_REQUEST:

                // if a task wanted something and is blocked, we return it to it
                char_from_request = msg_struct.value[0];
                if (blocked_notifier != -1) {
                    reply[0] = char_from_request;
                    Reply (blocked_notifier, (char *)&reply_struct, rpllen);
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    blocked_notifier = -1;
                }
                else {
                    // put char on the buffer
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_request;
                    buf_len++;
                    // don't really care what we reply
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                break;
            case PUTSTR_REQUEST:
                i=0;
                // copy_len = msg_struct.iValue;
                copystr = (char *)msg_struct.iValue;
                while ( (char_from_request = copystr[i++]) ) {
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_request;
                    buf_len++;
                }
                // if a task wanted something and is blocked, we return it to it
                if (blocked_notifier != -1) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (blocked_notifier, (char *)&reply_struct, rpllen);
                    blocked_notifier = -1;
                }
                // don't really care what we reply
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case PUTSTR_LEN_REQUEST:
                i=0;
                copy_len = msg_struct.iValue;
                while ( copy_len-- > 0) {
                    char_from_request = msg[i++];
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_request;
                    buf_len++;
                }
                // if a task wanted something and is blocked, we return it to it
                if (blocked_notifier != -1) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (blocked_notifier, (char *)&reply_struct, rpllen);
                    blocked_notifier = -1;
                }
                // don't really care what we reply
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case PUTC_NOTIFIER:
                // read char from the buffer
                // if there is char on the buffer, return it
                if (buf_len > 0) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                else {
                    // task is blocked;
                    blocked_notifier = sender_tid;
                }
                break;
            default:
                // shit
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll COMPUTSERVER %d", msg_struct.type);
                Assert();
                break;
        }
    }
}

void Com1GetServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM1_GET_SERVER);
    char data;

    FOREVER {
        data =(char) AwaitEvent( EVENT_COM1_RECEIVE );
        msg[0] = (char) data;
        // send evt to data
        msg_struct.type = GETC_NOTIFIER;
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
}

void Com2GetServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM2_GET_SERVER);
    char data;

    FOREVER {
        data =(char) AwaitEvent( EVENT_COM2_RECEIVE );
        msg[0] = (char) data;
        // send evt to data
        msg_struct.type = GETC_NOTIFIER;
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
}

// void GetTimeoutNotifier() {
//     // msg shits
//     char msg[2] = {0};
//     char reply[2] = {0};
//     int receiver_tid, msglen = 2;
//     message msg_struct, reply_struct;
//     msg_struct.value = msg;
//     reply_struct.value = reply;
//     char data;

//     FOREVER {
//         Receive( &receiver_tid, (char*)&msg_struct, msglen );
//         Reply (receiver_tid, (char *)&reply_struct, msglen);
//         Delay(msg_struct.iValue);
//         msg_struct.type = GETC_TIMEOUT;
//         Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
//     }
// }

// assuming only 1 thing is gonna call get
void GetServer() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int sender_tid, msglen = 2, rpllen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    // int timeout_notifier_tid;

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
    Reply (sender_tid, (char *)&reply_struct, rpllen);
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
            bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll GETSERVEREXIT %d", msg_struct.type);
            Assert();
            Exit();
    }
    // timeout_notifier_tid = Create(1, (&GetTimeoutNotifier));

    FOREVER {
        msglen = 2, rpllen = 2;
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case GETC_NOTIFIER:
                char_from_notifier = msg[0];
                // if a task wanted something and is blocked, we return it to it
                if (blocked_task != -1) {
                    reply[0] = char_from_notifier;
                    Reply (blocked_task, (char *)&reply_struct, rpllen);
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    blocked_task = -1;
                }
                else {
                    // put char on the buffer
                    insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                    string_buffer[insetion_point] = char_from_notifier;
                    buf_len++;
                    // don't really care what we reply
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                break;
            case GETC_REQUEST:
                if (msg_struct.type != GETC_REQUEST) {
                    bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll shits going down %d", msg_struct.type);
                    Assert();
                    break;
                }
                // read char from the buffer
                // if there is char on the buffer, return it
                if (buf_len > 0) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                else {
                    // task is blocked;
                    blocked_task = sender_tid;
                }
                break;
            // case GETC_TIMEOUT:
            //     if (blocked_task != -1) {
            //         reply[0] = -1;
            //         reply_struct.iValue = -1;
            //         Reply (blocked_task, (char *)&reply_struct, rpllen);
            //         Reply (sender_tid, (char *)&reply_struct, rpllen);
            //         blocked_task = -1;
            //     }
            //     break;
            // case GETC_TIMEOUT_REQUEST:
            //     // read char from the buffer
            //     // if there is char on the buffer, return it
            //     if (buf_len > 0) {
            //         reply[0] = string_buffer[buf_start];
            //         buf_start = (buf_start+1) % BUFFER_SIZE;
            //         buf_len--;
            //         Reply (sender_tid, (char *)&reply_struct, rpllen);
            //     }
            //     else {
            //         // task is blocked;
            //         blocked_task = sender_tid;
            //         msg[0] = blocked_task;
            //         Send (timeout_notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
            //     }
            //     break;
            default:
                // shit
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll COMGETSERVER %d", msg_struct.type);
                Assert();
                break;
        }
    }
}

void putc(int COM, char c) {
    char msg[2] = {0};
    char reply[4] = {0};
    int msglen = 2, rpllen = 4;
    static int com1_receiver_tid = -1, com2_receiver_tid = -1;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    reply_struct.iValue = 0;
    msg[0] = c;

    if (COM == COM1) {
        if (com1_receiver_tid < 0) {
            com1_receiver_tid = WhoIs(COM1_PUT_SERVER);
        }
        receiver_tid = com1_receiver_tid;
    }
    else if (COM == COM2) {
        if (com2_receiver_tid < 0) {
            com2_receiver_tid = WhoIs(COM2_PUT_SERVER);
        }
        receiver_tid = com2_receiver_tid;
    }
    else {
        bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll putc %d", COM);
        Assert();
        return;
    }

    msg_struct.type = PUTC_REQUEST;
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    return ;
}

void putstr(int COM, char* str ) {
    char msg[1] = {0};
    char reply[4] = {0};
    int msglen = 1 , rpllen = 4;
    static int com1_receiver_tid = -1, com2_receiver_tid = -1;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    // reply_struct.iValue = 0;
    // msglen = strlen(str);
    // memcpy(msg, str, msglen);
    msg_struct.iValue = (int)str;

    if (COM == COM1) {
        if (com1_receiver_tid < 0) {
            com1_receiver_tid = WhoIs(COM1_PUT_SERVER);
        }
        receiver_tid = com1_receiver_tid;
    }
    else if (COM == COM2) {
        if (com2_receiver_tid < 0) {
            com2_receiver_tid = WhoIs(COM2_PUT_SERVER);
        }
        receiver_tid = com2_receiver_tid;
    }
    else {
        bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll putstr %d", COM);
        // Assert();
        return;
    }
    msg_struct.type = PUTSTR_REQUEST;
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    return ;
}

void putstr_len(int COM, char* str, int msglen ) {
    char msg[710] = {0};
    char reply[4] = {0};
    int rpllen = 4;
    static int com1_receiver_tid = -1, com2_receiver_tid = -1;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = str;
    msg_struct.iValue = msglen;
    reply_struct.value = reply;
    reply_struct.iValue = 0;
    memcpy(msg, str, msglen);

    if (COM == COM1) {
        if (com1_receiver_tid < 0) {
            com1_receiver_tid = WhoIs(COM1_PUT_SERVER);
        }
        receiver_tid = com1_receiver_tid;
    }
    else if (COM == COM2) {
        if (com2_receiver_tid < 0) {
            com2_receiver_tid = WhoIs(COM2_PUT_SERVER);
        }
        receiver_tid = com2_receiver_tid;
    }
    else {
        bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll putstr_len %d", COM);
        // Assert();
        return;
    }
    msg_struct.type = PUTSTR_LEN_REQUEST;
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    return ;
}

char getc(int COM) {
    char msg[2] = {0};
    char reply[2] = {0};
    int msglen = 2;
    static int com1_receiver_tid = -1, com2_receiver_tid = -1;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    reply_struct.iValue = 0;
    msg_struct.type = GETC_REQUEST;

    if (COM == COM1) {
        if (com1_receiver_tid < 0) {
            com1_receiver_tid = WhoIs(COM1_GET_SERVER);
        }
        receiver_tid = com1_receiver_tid;
    }
    else if (COM == COM2) {
        if (com2_receiver_tid < 0) {
            com2_receiver_tid = WhoIs(COM2_GET_SERVER);
        }
        receiver_tid = com2_receiver_tid;
    }
    else {
        bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll getc %d", COM);
        // Assert();
        return 0;
    }

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    return reply_struct.value[0];
}

// char getc_timeout(int COM, int timeout, int* timedout) {
//     char msg[2] = {0};
//     char reply[2] = {0};
//     int msglen = 2;
//     int com1_receiver_tid = -1, com2_receiver_tid = -1;
//     int receiver_tid;
//     message msg_struct, reply_struct;
//     msg_struct.value = msg;
//     reply_struct.value = reply;
//     reply_struct.iValue = 0;
//     msg_struct.type = GETC_TIMEOUT_REQUEST;

//     if (COM == COM1) {
//         if (com1_receiver_tid < 0) {
//             com1_receiver_tid = WhoIs(COM1_GET_SERVER);
//         }
//         receiver_tid = com1_receiver_tid;
//     }
//     else if (COM == COM2) {
//         if (com2_receiver_tid < 0) {
//             com2_receiver_tid = WhoIs(COM2_GET_SERVER);
//         }
//         receiver_tid = com2_receiver_tid;
//     }
//     else {
//         bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll getc_timeout %d", COM);
//         // Assert();
//         return 0;
//     }

//     msg_struct.iValue = timeout;
//     Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

//     *timedout = 0;
//     if (reply_struct.iValue < 0){
//         *timedout = 1;
//     }

//     return reply_struct.value[0];
// }
