#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <train.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <sensors.h>
#include <commandcenter.h>

void CommandCenterNotifier() {
    int i;
    char c;
    int courier_tid;

    char msg[11] = {0};
    char reply[11] = {0};
    int receiver_tid, msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = COMMAND_CENTER_NOTIFIER;
    reply_struct.value = reply;

    Receive( &courier_tid, (char*)&msg_struct, msglen );
    Reply (courier_tid, (char *)&reply_struct, rpllen);

    FOREVER {
        Receive( &courier_tid, (char*)&msg_struct, msglen );
        waitForSensor((int) msg_struct.value[0]);
        Reply (courier_tid, (char *)&reply_struct, rpllen);
    }
}

void CommandCenterCourier() {
    int notifier_tid, server_tid;
    char sensor;
    char msg[11] = {0};
    char rpl[11] = {0};
    int receiver_tid, msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = COMMAND_CENTER_NOTIFIER;
    reply_struct.value = rpl;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    notifier_tid = msg_struct.iValue;
    sensor = msg_struct.value[0];
    Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    // bwprintf(COM2, "%d", server_tid);
    Reply (server_tid, (char *)&reply_struct, rpllen);
                // Assert();
    msg_struct.value[0] = sensor;
    FOREVER {
        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
        Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    }
}

void CommandCenterServer() {
    // Create Notifier and send any initialization data
    // msg shits
    char msg[11] = {0};
    char reply[11] = {0};
    int sender_tid, msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    int requests[64] = {-1};
    char sensors_ahead[5] = {-1};
    int courier_tid, notifier_tid, train_count = 0;
    int train_info[MAX_TRAIN_COUNT][6];
    // each tid can only call one delay at a time
    int i, j;
    for (i = 0; i < MAX_TRAIN_COUNT; i++) {
        train_info[i][0] = -1;
        train_info[i][1] = -1;
        train_info[i][2] = -1;
        train_info[i][3] = -1;
        train_info[i][4] = -1;
        train_info[i][5] = -1;
    }
    RegisterAs(COMMAND_CENTER_SERVER_NAME);
    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case COMMAND_CENTER_NOTIFIER:
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i][TRAIN_INFO_COURIER] == sender_tid) {
                        train_info[i][TRAIN_INFO_SENSOR] = train_info[i][TRAIN_INFO_NEXT_SENSOR];
                        predictSensor(train_info[i][TRAIN_INFO_SENSOR], 2, sensors_ahead);
                        train_info[i][TRAIN_INFO_NEXT_SENSOR] = sensors_ahead[0];

                        for (j=0; j < 64; j++) {
                            if (requests[j] == train_info[i][TRAIN_INFO_ID]) {
                                reply_struct.value[0] = train_info[i][TRAIN_INFO_SENSOR];
                                reply_struct.value[1] = train_info[i][TRAIN_INFO_NEXT_SENSOR];
                                Reply (j, (char *)&reply_struct, rpllen);
                                break;
                            }
                        }
                        break;
                    }
                }
                reply_struct.value[0] = train_info[i][TRAIN_INFO_NEXT_SENSOR];
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case INIT_TRAIN_REQUEST:
                train_info[train_count][TRAIN_INFO_SENSOR] = msg_struct.iValue;
                train_info[train_count][TRAIN_INFO_ID] = (int) msg_struct.value[0];
                train_info[train_count][TRAIN_INFO_TASK] = (int) msg_struct.value[1];

                notifier_tid = Create(1, (&CommandCenterNotifier));
                courier_tid = Create(2, (&CommandCenterCourier));
                train_info[train_count][TRAIN_INFO_COURIER] = courier_tid;
                msg_struct.iValue = notifier_tid;

                predictSensor(train_info[train_count][TRAIN_INFO_SENSOR], 2, sensors_ahead);
                train_info[train_count][TRAIN_INFO_NEXT_SENSOR] = sensors_ahead[0];
                msg_struct.value[0] = train_info[train_count][TRAIN_INFO_NEXT_SENSOR];
                Send (courier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
                // Assert();
                train_count++;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                // reply what the time is
                break;
            case GET_TRAIN_INFO_REQUEST:
                requests[sender_tid] = msg_struct.iValue;
                break;
            default:
                // wtf
                break;
        }
    }
}

int initTrainLocation( int train_id, int sensor ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10, rpllen = 10;
    int receiver_tid, train_task_id;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = INIT_TRAIN_REQUEST;
    reply_struct.value = reply;

    receiver_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);
    genTrainName(train_id, msg);
    train_task_id = WhoIs(msg);

    if (train_task_id == -1) {
        train_task_id = Create(2, (&TrainTask));
        msg_struct.iValue = train_id;
        Send (train_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
    msg_struct.iValue = sensor;
    msg_struct.value[0] = (char) train_id;
    msg_struct.value[1] = (char) train_task_id;
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return 0;
    }
    return -1;
}

int waitTrainInfo ( int train_id, char *train_info) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.iValue = train_id;
    msg_struct.type = GET_TRAIN_INFO_REQUEST;
    reply_struct.value = reply;
    reply_struct.iValue = 0;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        // if succeded
        train_info[0] = reply_struct.value[0];
        train_info[1] = reply_struct.value[1];
        return 0;
    }
    return -1;
}
