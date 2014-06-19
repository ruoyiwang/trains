#include <comservers.h>
#include <nameserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>

void Com2PutServerNotifier() {

}

void Com2PutServer() {

}

void Com2GetServerNotifier() {
    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int receiver_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = NOTIFIER;
    reply_struct.value = reply;
    receiver_tid = WhoIs(COM2_GET_SERVER);
    int data;

    FOREVER {
        data = AwaitEvent( EVENT_COM2_RECEIVE );
        msg[0] = (char) data;
        // send evt to data
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
}

// assuming only 1 thing is gonna call get
void Com2GetServer() {
    RegisterAs(COM2_GET_SERVER);

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

    // Create Notifier and send any initialization data
    Create(1, CODE_OFFSET + (&Com2GetServerNotifier));

    // msg shits
    char msg[2] = {0};
    char reply[2] = {0};
    int sender_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case COM2_GETC_NOTIFIER:
                // put char on the buffer
                char_from_notifier = msg[1];
                insetion_point = ( buf_start + buf_len ) % BUFFER_SIZE;
                string_buffer[insetion_point] = char_from_notifier;
                buf_len++;
                // don't really care what we reply
                Reply (sender_tid, (char *)&reply_struct, msglen);

                // if a task wanted something and is blocked, we return it to it
                if (blocked_task != -1) {
                    reply[0] = string_buffer[buf_start];
                    buf_start = (buf_start+1) % BUFFER_SIZE;
                    buf_len--;
                    Reply (blocked_task, (char *)&reply_struct, msglen);
                    blocked_task = -1;
                }

                break;
            case COM2_GETC_REQUEST:
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
