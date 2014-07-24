#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <track.h>
#include <train.h>
#include <posintlist.h>

#include <track_data.h>
#include <track_node.h>

void genTrainName( int train_id, char* bf) {
	*bf = 't';
	*(bf+1) = 'r';
	bwi2a(train_id, bf+2);
}

void TrainTask () {
    // msg shits
    char msg[10] = {0};
    char reply[2] = {0};
    char commandstr[10] = {0};

    int sender_tid, msglen = 10, rpllen = 2;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    int train_id, speed = 0;

    Receive( &sender_tid, (char*)&msg_struct, msglen );
    Reply (sender_tid, (char *)&reply_struct, rpllen);

    RegisterAs(msg_struct.value);
    train_id = msg_struct.iValue;

    FOREVER {
        rpllen = 2;
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch (msg_struct.type) {
            case TRAIN_REVERSE:
                commandstr[0] = 15;
                commandstr[1] = train_id;
                // commandstr[2] = speed;
                // commandstr[3] = train_id;
                commandstr[2] = 0;
                putstr_len(COM1, commandstr, 2);
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case TRAIN_SET_SPEED:
                speed = msg_struct.iValue;
                commandstr[0] = speed;
                commandstr[1] = train_id;
                commandstr[2] = 0;
                putstr_len(COM1, commandstr, 2);
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            default:
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll TRAINSERVER %d", msg_struct.type);
                Assert();
                break;
        }
    }
}

int reverseTrain( int num ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10, rpllen = 10;
    static int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TRAIN_REVERSE;
    reply_struct.value = reply;

    genTrainName(num, msg);
    receiver_tid = WhoIs(msg);

    if (receiver_tid == -1) {
        receiver_tid = Create(3, (&TrainTask));
        msg_struct.iValue = num;
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return 0;
    }
    return -1;
}

int setTrainSpeed( int num, int speed ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10, rpllen = 10;
    static int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TRAIN_SET_SPEED;
    reply_struct.value = reply;

    genTrainName(num, msg);
    receiver_tid = WhoIs(msg);

    if (receiver_tid == -1) {
        receiver_tid = Create(3, (&TrainTask));
        msg_struct.iValue = num;
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }

    msg_struct.iValue = speed;
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return 0;
    }
    return -1;
}
