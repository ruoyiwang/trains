#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <train.h>

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
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch (msg_struct.type) {
            case TRAIN_REVERSE:
                commandstr[0] = 15;
                commandstr[1] = train_id;
                commandstr[2] = speed;
                commandstr[3] = train_id;
                commandstr[4] = 0;
                putstr_len(COM1, commandstr, 4);
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
                break;
        }
    }
}

int reverseTrain( int num ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10, rpllen = 10;
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TRAIN_REVERSE;
    reply_struct.value = reply;

    genTrainName(num, msg);
    receiver_tid = WhoIs(msg);

    if (receiver_tid == -1) {
        receiver_tid = Create(5, (&TrainTask));
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
    int receiver_tid;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TRAIN_SET_SPEED;
    reply_struct.value = reply;

    genTrainName(num, msg);
    receiver_tid = WhoIs(msg);

    if (receiver_tid == -1) {
        receiver_tid = Create(5, (&TrainTask));
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

void TracksTask () {
    RegisterAs(TRACK_TASK);

    // msg shits
    char msg[10] = {0};
    char reply[10] = {0};
    int sender_tid, msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    char commandstr[10] = {0};

    // switch status, bitwise 32-0,
    // think we only 22-1?
    // 0: curve
    // 1: stright
    unsigned int switch_status = 0;

    // init the tracks
    track_node tracks[TRACK_MAX];
    // and we are using track b
    init_trackb(tracks);
    int cur_sensor, prediction_len = 8;

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch (msg_struct.type) {
            case SET_SWITCH:
                if (msg_struct.value[0] == 's') {
                    commandstr[0] = SW_STRAIGHT;
                    setSwitchStatus(&switch_status, msg_struct.iValue, SW_STRAIGHT);
                }
                else if (msg_struct.value[0] == 'c'){
                    commandstr[0] = SW_CURVE;
                    setSwitchStatus(&switch_status, msg_struct.iValue, SW_CURVE);
                }
                commandstr[1] = msg_struct.iValue;
                commandstr[2] = 32;
                commandstr[3] = 0;
                putstr_len(COM1, commandstr, 3);
                // bwprintf(COM2, "received sw command\n\n");
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case PREDICT:
                // predict
                cur_sensor = msg[0];
                prediction_len = msg_struct.iValue;
                // type PREDICT + value sensor (in int_8 lol) + ivalue n
                // <== next n switches after sensor, default 8
                predictPath(tracks, switch_status, cur_sensor, prediction_len, reply);
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case PATH_FIND:
                // path find
                // type: PATH_FIND + value cur_sensor, destination
                break;
            default:
                break;
        }
    }
}

void setSwitchStatus(unsigned int* switch_status, int sw, int dir) {
    if( sw > 18 ) {
        sw -= 134;
    }
    if (dir == SW_STRAIGHT) {
        (*switch_status) = (*switch_status) | (1 << sw);
    }
    else {
        (*switch_status) = (*switch_status) & ~(1 << sw);
    }
}

int getSwitchStatus(unsigned int* switch_status, int sw) {
    if( sw > 18 ) {
        sw -= 134;
    }
    if ((*switch_status) & (1 << sw)){
        return SW_STRAIGHT;
    }
    return SW_CURVE;
}

void predictPath(
    track_node *tracks,     // the initialized array of tracks
    unsigned int switch_status,
    int cur_sensor,         // 0 based
    int prediction_len,     // amount of predictions wanted
    char* paths              // the triggers to be triggered
) {
    track_node* cur_node = &tracks[cur_sensor];
    track_node* next_node = cur_node->edge[DIR_AHEAD].dest;
    int i = 0;
    int cur_branch_status;

    for (i = 0; i < prediction_len; i++) {

        while (next_node->type != NODE_SENSOR) {
            cur_node = next_node;
            // calc next_node
            if (cur_node->type == NODE_BRANCH) {
                // look up switch status, then find next_node
                cur_branch_status = getSwitchStatus(&switch_status, cur_node->num);
                if (cur_branch_status == SW_STRAIGHT) {
                    next_node = cur_node->edge[DIR_STRAIGHT].dest;
                }
                else {
                    next_node = cur_node->edge[DIR_CURVED].dest;
                }
            }
            else {
                next_node = cur_node->edge[DIR_AHEAD].dest;
            }
        }

        paths[i] = (char) next_node->num;
        cur_node = next_node;
        next_node = cur_node->edge[DIR_AHEAD].dest;
    }
}

// int waitForSensor( int sensor ) {
//     char msg[10] = {0};
//     char reply[10] = {0};
//     int msglen = 10;
//     static int receiver_tid = -1;
//     if (receiver_tid < 0) {
//         receiver_tid = WhoIs(SENSOR_SERVER_NAME);
//     }
//     message msg_struct, reply_struct;
//     msg_struct.value = msg;
//     msg_struct.iValue = sensor;
//     msg_struct.type = WAIT_REQUEST;
//     reply_struct.value = reply;

//     Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

//     if (strcmp(msg_struct.value, "FAIL") != 0) {
//         // if succeded
//         return 0;
//     }
//     return -1;
// }
