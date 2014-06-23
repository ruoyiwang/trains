#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <train.h>

void genTrainName( int train_id, char* bf) {
	*bf = 't';
	*(bf+1) = 'r';
	bwi2a(train_id, bf+2);
}

void TrainTask () {
    // msg shits
    char msg[10] = {0};
    char reply[10] = {0};
    int sender_tid, msglen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    int train_id, speed = 0;

    Receive( &sender_tid, (char*)&msg_struct, msglen );
    Reply (sender_tid, (char *)&reply_struct, msglen);

    RegisterAs(msg_struct.value);
    train_id = msg_struct.iValue;

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch (msg_struct.type) {
            case TRAIN_REVERSE:
                Delay(COMMAND_DELAY);
                putc(COM1, 15);
                Delay(COM1_PUT_DELAY);
                putc(COM1, train_id);
                Delay(COMMAND_DELAY);
                putc(COM1, speed);
                Delay(COM1_PUT_DELAY);
                putc(COM1, train_id);
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            case TRAIN_SET_SPEED:
                speed = msg_struct.iValue;
                Delay(COM1_PUT_DELAY);
                putc(COM1, speed);
                Delay(COM1_PUT_DELAY);
                putc(COM1, train_id);
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            default:
                break;
        }
    }
}

void TracksTask () {
    // msg shits
    char msg[10] = {0};
    char reply[10] = {0};
    int sender_tid, msglen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    RegisterAs(TRACK_TASK);

    putc(COM1, 0x60);

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch (msg_struct.type) {
            case SET_SWITCH:
                Delay(COMMAND_DELAY);
                if (msg_struct.value[0] == 's') {
                    putc(COM1, SW_STRAIGHT);
                }
                else if (msg_struct.value[0] == 'c'){
                    putc(COM1, SW_CURVE);
                }
                Delay(COM1_PUT_DELAY);
                putc(COM1, msg_struct.iValue);
                Delay(COMMAND_DELAY);
                putc(COM1, 32);
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            default:
                break;
        }
    }
}