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
    char commandstr[10] = {0};

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
                commandstr[0] = 15;
                commandstr[1] = train_id;
                commandstr[2] = speed;
                commandstr[3] = train_id;
                commandstr[4] = 0;
                putstr(COM1, commandstr);
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            case TRAIN_SET_SPEED:
                speed = msg_struct.iValue;
                commandstr[0] = speed;
                commandstr[1] = train_id;
                commandstr[2] = 0;
                putstr(COM1, commandstr);
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
    char commandstr[10] = {0};

    RegisterAs(TRACK_TASK);

    putc(COM1, 0x60);

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch (msg_struct.type) {
            case SET_SWITCH:
                if (msg_struct.value[0] == 's') {
                    commandstr[0] = SW_STRAIGHT;
                }
                else if (msg_struct.value[0] == 'c'){
                    commandstr[0] = SW_CURVE;
                }
                commandstr[1] = msg_struct.iValue;
                commandstr[2] = 32;
                commandstr[3] = 0;
                putstr(COM1, commandstr);
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            default:
                break;
        }
    }
}