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
#include <trainspeed.h>

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
    char reply[30] = {0};
    int sender_tid, msglen = 10, rpllen = 10, expected_time, actual_time;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    int requests[64] = {-1};
    char sensors_ahead[5] = {-1};
    int courier_tid, notifier_tid, train_count = 0;
    int train_info[MAX_TRAIN_COUNT][7];
    int train_speed[MAX_TRAIN_COUNT][80];
    // each tid can only call one delay at a time
    int i, j;
    for (i = 0; i < MAX_TRAIN_COUNT; i++) {
        train_info[i][TRAIN_INFO_ID] = -1;
        train_info[i][TRAIN_INFO_SENSOR] = -1;
        train_info[i][TRAIN_INFO_NEXT_SENSOR] = -1;
        train_info[i][TRAIN_INFO_TIME] = 0;
        train_info[i][TRAIN_INFO_TASK] = -1;
        train_info[i][TRAIN_INFO_COURIER] = -1;
        train_info[i][TRAIN_INFO_TIME_PREDICTION] = 0;
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
                        actual_time = Time();
                        train_info[i][TRAIN_INFO_TIME] = actual_time;

                        for (j=0; j < 64; j++) {
                            if (requests[j] == train_info[i][TRAIN_INFO_ID]) {
                                reply_struct.value[0] = train_info[i][TRAIN_INFO_SENSOR];
                                reply_struct.value[1] = train_info[i][TRAIN_INFO_NEXT_SENSOR];
                                bwi2a(train_info[i][TRAIN_INFO_TIME_PREDICTION], reply + 2);
                                reply_struct.iValue = train_info[i][TRAIN_INFO_TIME];
                                Reply (j, (char *)&reply_struct, 30);
                                break;
                            }
                        }

                        expected_time = predictArrivalTime(train_info[i][TRAIN_INFO_SENSOR],
                                                                     train_info[i][TRAIN_INFO_NEXT_SENSOR],
                                                                     train_info[i][TRAIN_INFO_TIME],
                                                                     train_speed[i]);
                        train_info[i][TRAIN_INFO_TIME_PREDICTION] = expected_time;
                        // bwprintf(COM2, "%d", train_info[i][TRAIN_INFO_TIME_PREDICTION]);
                        // Assert();
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
                init_train_speed(train_info[train_count][TRAIN_INFO_ID], train_speed[train_count]);
                // predictArrivalTime(42, 20, -1, train_speed[train_count]);
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

int predictArrivalTime( int sensor, int next_sensor, int init_time, int *train_speed) {
    int delta_distance = findDistanceBetweenLandmarks( sensor, next_sensor, 10);  //distance is in mm
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;

    int delta_time = (int) delta_distance / velocity;
    int ret =  init_time + delta_time;
    // bwprintf(COM2, "%d", ret);
    return ret;
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
        train_task_id = Create(3, (&TrainTask));
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

int waitTrainInfo ( int train_id, int *train_info) {
    char msg[10] = {0};
    char reply[30] = {0};
    int msglen = 10, rpllen = 30;
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

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        // if succeded
        train_info[0] = reply_struct.value[0];
        train_info[1] = reply_struct.value[1];
        train_info[2] = (int) atoi(reply_struct.value + 2);
        // Assert();
        train_info[3] = reply_struct.iValue;
        return 0;
    }
    return -1;
}
