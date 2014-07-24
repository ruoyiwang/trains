#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <train.h>
#include <track.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <sensors.h>
#include <commandcenter.h>
#include "track_data.h"
#include <trainspeed.h>

void CommandCenterNotifier() {
    int courier_tid;

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = COMMAND_CENTER_NOTIFIER;
    reply_struct.value = reply;

    Receive( &courier_tid, (char*)&msg_struct, msglen );
    Reply (courier_tid, (char *)&reply_struct, rpllen);

    FOREVER {
        Receive( &courier_tid, (char*)&msg_struct, 10 );
        reply_struct.iValue = waitForSensors( msg_struct.value, 4, msg_struct.iValue);
        Reply (courier_tid, (char *)&reply_struct, rpllen);
    }
}

void CommandCenterCourier() {
    int notifier_tid, server_tid;
    char msg[11] = {0};
    char rpl[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = COMMAND_CENTER_NOTIFIER;
    reply_struct.value = rpl;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    notifier_tid = msg_struct.iValue;
    Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    Reply (server_tid, (char *)&reply_struct, rpllen);

    FOREVER {
        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
        Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    }
}

void CommandCenterStoppingNotifier() {
    int server_tid;

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = COMMAND_CENTER_STOPPING_NOTIFIER;
    reply_struct.value = reply;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    Reply (server_tid, (char *)&reply_struct, rpllen);

    DebugPutStr("sdsd", "DEBUG: notifier: ", msg_struct.value[0], " : ", msg_struct.iValue);
    int is_short_move = 0;
    int delay = (int) msg_struct.iValue;
    if (!(msg_struct.value[0] < (char) 80)) {
        is_short_move = 1;
    }
    if (!is_short_move){
        waitForSensors(msg_struct.value, 1, 1000000);
    }
    Delay(delay);
    Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    if (is_short_move) {
        // DebugPutStr("s", "FAST STOP");
        Delay(delay);
    }
    else {
        Delay(790);
    }
    reply_struct.type = COMMAND_CENTER_TRAIN_STOPPED;
    Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    Exit();
}

void CommandCenterServer() {
    // Create Notifier and send any initialization data
    // msg shits
    char msg[11] = {0};
    char reply[30] = {0};
    int sender_tid, msglen = 10, rpllen = 10, expected_time;
    int actual_time, total_distance;

    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    int requests[64] = {-1};
    char sensors_ahead[5] = {-1};
    int courier_tid, notifier_tid, train_count = 0;

    int train_info[MAX_TRAIN_COUNT][TRAIN_INFO_SIZE];
    int train_speed[MAX_TRAIN_COUNT][80];
    // each tid can only call one delay at a time
    int i, j;
    for (i = 0; i < MAX_TRAIN_COUNT; i++) {
        train_info[i][TRAIN_INFO_ID] = -1;
        train_info[i][TRAIN_INFO_SENSOR] = -1;
        train_info[i][TRAIN_INFO_NEXT_SENSOR] = -1;
        train_info[i][TRAIN_INFO_SENSOR_OFFSET] = 0;
        train_info[i][TRAIN_INFO_TIME] = 0;
        train_info[i][TRAIN_INFO_TASK] = -1;
        train_info[i][TRAIN_INFO_COURIER] = -1;
        train_info[i][TRAIN_INFO_TIME_PREDICTION] = 0;
        train_info[i][TRAIN_INFO_STOPPING_NOTIFIER] = -1;
        train_info[i][TRAIN_INFO_STOPPING_SENSOR] = -1;
        train_info[i][TRAIN_INFO_STOPPING_OFFSET] = 0;
        train_info[i][TRAIN_INFO_STOPPED] = 1;
        train_info[i][TRAIN_INFO_TIMEOUT] = 300;
        train_info[i][TRAIN_INFO_DEST_SENSOR] = -1;
        train_info[i][TRAIN_INFO_DEST_OFFSET] = 0;
        train_info[i][TRAIN_INFO_REVERSED] = 0;
    }

    RegisterAs(COMMAND_CENTER_SERVER_NAME);
    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {

            case COMMAND_CENTER_NOTIFIER:
                // handle notifer for sensors that are triggered
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    // find the right train
                    if (train_info[i][TRAIN_INFO_COURIER] == sender_tid) {

                        actual_time = Time();
                        train_info[i][TRAIN_INFO_TIME] = actual_time;
                        // -1 means the sensor timedout
                        if (msg_struct.iValue == -1 ) {
                            bwprintf(COM2, "next sensor is -1????\n\n");
                            Assert();
                            // double the next timeout
                            train_info[i][TRAIN_INFO_TIMEOUT] *= 2;
                            if (train_info[i][TRAIN_INFO_TIMEOUT] > 10000){
                                train_info[i][TRAIN_INFO_TIMEOUT] = 10000;
                            }
                            // if the train doesn't reach the stopping sensor, assume it stopped early
                            if (train_info[i][TRAIN_INFO_STOPPING_SENSOR] == train_info[i][TRAIN_INFO_NEXT_SENSOR]) {
                                total_distance = findDistanceBetweenLandmarks( train_info[i][TRAIN_INFO_SENSOR], train_info[i][TRAIN_INFO_NEXT_SENSOR], 2);
                                train_info[i][TRAIN_INFO_SENSOR_OFFSET] = total_distance;
                                train_info[i][TRAIN_INFO_STOPPED] = 1;
                                train_info[i][TRAIN_INFO_STOPPING_SENSOR] = -1;
                                train_info[i][TRAIN_INFO_STOPPING_OFFSET] = 0;
                            }
                            // get the next sensor
                            predictSensor(train_info[i][TRAIN_INFO_SENSOR], 2, sensors_ahead);
                            break;
                        }

                        // get which sensor was triggered
                        train_info[i][TRAIN_INFO_SENSOR] = msg_struct.iValue;
                        // get the next sensor
                        predictSensor(train_info[i][TRAIN_INFO_SENSOR], 2, sensors_ahead);
                        train_info[i][TRAIN_INFO_NEXT_SENSOR] = sensors_ahead[0];
                        // get time the sensor is triggered
                        actual_time = Time();
                        train_info[i][TRAIN_INFO_TIME] = actual_time;
                        // set the sensor offset back to 0
                        train_info[i][TRAIN_INFO_SENSOR_OFFSET] = 0;

                        // if the sensor triggered is the same as the stopping sensor then go into stopped state
                        // if (train_info[i][TRAIN_INFO_STOPPING_SENSOR] == train_info[i][TRAIN_INFO_SENSOR]) {
                        //     train_info[i][TRAIN_INFO_SENSOR_OFFSET] = train_info[i][TRAIN_INFO_STOPPING_OFFSET];
                        //     train_info[i][TRAIN_INFO_STOPPED] = 1;
                        //     train_info[i][TRAIN_INFO_STOPPING_SENSOR] = -1;
                        //     train_info[i][TRAIN_INFO_STOPPING_OFFSET] = 0;
                        // }

                        // check if there were any request on the blocked sensor
                        for (j=0; j < 64; j++) {
                            if (requests[j] == train_info[i][TRAIN_INFO_ID]) {
                                // return the sensors and the expected and predicted time
                                reply_struct.value[0] = train_info[i][TRAIN_INFO_SENSOR];
                                reply_struct.value[1] = train_info[i][TRAIN_INFO_NEXT_SENSOR];
                                bwi2a(train_info[i][TRAIN_INFO_TIME_PREDICTION], reply + 2);
                                reply_struct.iValue = train_info[i][TRAIN_INFO_TIME];
                                Reply (j, (char *)&reply_struct, 30);
                                break;
                            }
                        }

                        // predict the time of arrival for the next sensor
                        expected_time = predictArrivalTime(train_info[i][TRAIN_INFO_SENSOR],
                                                                     train_info[i][TRAIN_INFO_NEXT_SENSOR],
                                                                     train_info[i][TRAIN_INFO_TIME],
                                                                     train_speed[i]);
                        train_info[i][TRAIN_INFO_TIME_PREDICTION] = expected_time;
                        break;
                    }
                }
                // give the sensor to wait on as well as the time out to the notifier
                reply_struct.iValue = train_info[i][TRAIN_INFO_TIME] + train_info[i][TRAIN_INFO_TIMEOUT];
                reply_struct.value[0] = train_info[i][TRAIN_INFO_NEXT_SENSOR];
                reply_struct.value[1] = sensors_ahead[1];
                reply_struct.value[2] = getSensorComplement(train_info[i][TRAIN_INFO_SENSOR]);
                predictSensor(reply_struct.value[2], 2, sensors_ahead);
                reply_struct.value[3] = sensors_ahead[0];
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_STOPPING_NOTIFIER:
                // handle stoping notifer, this means the train needs to stop now
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i][TRAIN_INFO_STOPPING_NOTIFIER] == sender_tid) {
                        // stop train
                        setTrainSpeed (train_info[i][TRAIN_INFO_ID], 0);
                        break;
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_TRAIN_STOPPED:
                // handle stoping notifer, this means the train needs to stop now
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i][TRAIN_INFO_STOPPING_NOTIFIER] == sender_tid) {
                        DebugPutStr("s", "DEBUG: Train Stopped!");
                        if (train_info[i][TRAIN_INFO_SENSOR] == train_info[i][TRAIN_INFO_STOPPING_SENSOR]){
                            train_info[i][TRAIN_INFO_SENSOR_OFFSET] = train_info[i][TRAIN_INFO_STOPPING_OFFSET];
                        }

                        train_info[i][TRAIN_INFO_STOPPED] = 1;
                        train_info[i][TRAIN_INFO_STOPPING_SENSOR] = -1;
                        train_info[i][TRAIN_INFO_STOPPING_OFFSET] = 0;
                        train_info[i][TRAIN_INFO_STOPPING_NOTIFIER] = -1;
                        if (train_info[i][TRAIN_INFO_DEST_SENSOR] == train_info[i][TRAIN_INFO_SENSOR] ||
                            train_info[i][TRAIN_INFO_DEST_SENSOR] == train_info[i][TRAIN_INFO_NEXT_SENSOR]) {
                            train_info[i][TRAIN_INFO_DEST_SENSOR] = -1;
                            train_info[i][TRAIN_INFO_DEST_OFFSET] = 0;
                            DebugPutStr("s", "DEBUG: Arrived at destination!");
                            break;
                        }

                        serverSetStopping(train_info[i], train_speed[i], train_info[i][TRAIN_INFO_DEST_SENSOR], train_info[i][TRAIN_INFO_DEST_OFFSET]);
                        break;
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case GET_TRAIN_LOCATION_REQUEST:
                // handle request for the trains location relative to its previous sensor
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.iValue == train_info[i][TRAIN_INFO_ID]) {
                        // if train is stopped
                        if (train_info[i][TRAIN_INFO_STOPPED] && train_info[i][TRAIN_INFO_TIME] >= 0) {
                            // if the train stopped really close the the stopping sensor
                            if (train_info[i][TRAIN_INFO_NEXT_SENSOR] == train_info[i][TRAIN_INFO_STOPPING_SENSOR]
                                && train_info[i][TRAIN_INFO_STOPPING_OFFSET] < 5) {
                                // the offset is the distance between the sensors
                                total_distance = findDistanceBetweenLandmarks( train_info[i][TRAIN_INFO_SENSOR], train_info[i][TRAIN_INFO_NEXT_SENSOR], 2);
                                train_info[i][TRAIN_INFO_SENSOR_OFFSET] = total_distance;
                                break;
                            }
                            else if (train_info[i][TRAIN_INFO_SENSOR] == train_info[i][TRAIN_INFO_STOPPING_SENSOR]) {
                                // the stopping sensor is crossed return the stopping offset
                                train_info[i][TRAIN_INFO_SENSOR_OFFSET] = train_info[i][TRAIN_INFO_STOPPING_OFFSET];
                                break;
                            }
                        }
                        // train was just initialized
                        else if (train_info[i][TRAIN_INFO_STOPPED]){
                            train_info[i][TRAIN_INFO_SENSOR_OFFSET] = 0;
                        }
                        else {
                            // if the train is moving, calculate the offset based on speed and time
                            actual_time = Time();
                            total_distance = findDistanceBetweenLandmarks( train_info[i][TRAIN_INFO_SENSOR], train_info[i][TRAIN_INFO_NEXT_SENSOR], 100);
                            train_info[i][TRAIN_INFO_SENSOR_OFFSET] = timeToDistance( train_info[i][TRAIN_INFO_SENSOR],
                                                    actual_time - train_info[i][TRAIN_INFO_TIME], train_speed[i] );
                            // cannot be greater than the max distance
                            // DebugPutStr("sdsd", "DEBUG: time:", actual_time - train_info[i][TRAIN_INFO_TIME], " distance:", total_distance);
                            if (train_info[i][TRAIN_INFO_SENSOR_OFFSET] > total_distance) {
                                train_info[i][TRAIN_INFO_SENSOR_OFFSET] = total_distance;
                            }
                        }
                        break;
                    }
                }
                reply_struct.value[0] = train_info[i][TRAIN_INFO_SENSOR];
                reply_struct.iValue = train_info[i][TRAIN_INFO_SENSOR_OFFSET];
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case INIT_TRAIN_REQUEST:
                // handle the request to start a new train
                train_info[train_count][TRAIN_INFO_SENSOR] = msg_struct.iValue;
                train_info[train_count][TRAIN_INFO_ID] = (int) msg_struct.value[0];
                train_info[train_count][TRAIN_INFO_TASK] = (int) msg_struct.value[1];

                // initialize the notifier and the courier
                notifier_tid = Create(1, (&CommandCenterNotifier));
                courier_tid = Create(2, (&CommandCenterCourier));
                train_info[train_count][TRAIN_INFO_COURIER] = courier_tid;
                msg_struct.iValue = notifier_tid;

                // pass the sensors to be waited on to the notifier, there is no time out on first wait
                predictSensor(train_info[train_count][TRAIN_INFO_SENSOR], 2, sensors_ahead);
                train_info[train_count][TRAIN_INFO_NEXT_SENSOR] = sensors_ahead[0];

                DebugPutStr("sdsd", "DEBUG: from:", train_info[train_count][TRAIN_INFO_SENSOR], " to:", train_info[train_count][TRAIN_INFO_NEXT_SENSOR]);
                msg_struct.value[0] = train_info[train_count][TRAIN_INFO_NEXT_SENSOR];
                msg_struct.value[1] = sensors_ahead[1];
                msg_struct.value[2] = getSensorComplement(train_info[train_count][TRAIN_INFO_SENSOR]);
                predictSensor(msg_struct.value[2], 2, sensors_ahead);
                msg_struct.value[3] = sensors_ahead[0];
                Send (courier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

                // initialize the train speed table
                init_train_speed(train_info[train_count][TRAIN_INFO_ID], train_speed[train_count]);
                train_count++;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case TRAIN_DESTINATION_REQUEST:
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.value[0] == train_info[i][TRAIN_INFO_ID]) {
                        train_info[i][TRAIN_INFO_DEST_SENSOR] = msg_struct.iValue;
                        train_info[i][TRAIN_INFO_DEST_OFFSET] = (int) msg_struct.value[1]*10;

                        serverSetStopping(train_info[i], train_speed[i], msg_struct.iValue, (int) msg_struct.value[1]*10);
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case GET_TRAIN_INFO_REQUEST:
                requests[sender_tid] = msg_struct.iValue;
                break;
            default:
                bwprintf(COM2, "fmlllll COMMAND CENTER SEVER %d\n", msg_struct.type);
                reply_struct.type = FAIL_TYPE;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
        }
    }
}

void serverSetStopping (int* train_info, int* train_speed, int sensor, int offset) {
    char msg[11] = {0};
    char reply[30] = {0};
    int msglen = 10, rpllen = 10;

    move_data md;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    // location = train_info[i][TRAIN_INFO_SENSOR_OFFSET] / 10;
    // initialize the stoping notifier
    int notifier_tid = Create(1, (&CommandCenterStoppingNotifier));
    int stopping_sensor_dist = 0, stopping_sensor = -1;
    int blocked_nodes[20];
    // update the train info
    train_info[TRAIN_INFO_STOPPING_NOTIFIER] = notifier_tid;
    train_info[TRAIN_INFO_TIMEOUT] = 300;

    DebugPutStr("sdsdsd", "DEBUG: routing from ", train_info[TRAIN_INFO_SENSOR], ":", train_info[TRAIN_INFO_SENSOR_OFFSET], " to ", sensor);
    // find path which will also set the switches
    pathFindDijkstra(
        &md,
        train_info[TRAIN_INFO_SENSOR],          // current node
        0,                      // offset
        sensor,                 // where it wants to go
        790,                    // stoping distance
        blocked_nodes,          // the nodes the trains can't use
        0                       // the length of the blocked nodes array
    );
    DebugPutStr("s", "DEBUG: found route ");
    stopping_sensor = md.stopping_sensor;
    stopping_sensor_dist = md.stopping_dist;

    int i;
    train_info[TRAIN_INFO_STOPPED] = 0;
    train_info[TRAIN_INFO_STOPPING_SENSOR] = sensor;
    train_info[TRAIN_INFO_STOPPING_OFFSET] = offset;
    train_info[TRAIN_INFO_STOPPING_SENSOR] = md.node_list[md.list_len-1].num;
    if (md.type == UNSAFE_REVERSE) {
        train_info[TRAIN_INFO_STOPPING_OFFSET] = 30;
    }
    for (i = 0; i < md.list_len; i++) {
        if (md.node_list[i].type == NODE_BRANCH) {
            setSwitch(md.node_list[i].branch_state, md.node_list[i].num);
        }
    }
    if (md.type == SAFE_REVERSE || md.type == UNSAFE_REVERSE){
        if (train_info[TRAIN_INFO_REVERSED]){
            train_info[TRAIN_INFO_REVERSED] = 0;
        }
        else {
            train_info[TRAIN_INFO_REVERSED] = 1;
        }
        msg_struct.value[0] = -1;
        msg_struct.iValue = 1;
        int next_sensor, prev_sensor;
        next_sensor = getSensorComplement(train_info[TRAIN_INFO_SENSOR]);
        prev_sensor = getSensorComplement(train_info[TRAIN_INFO_NEXT_SENSOR]);
        DebugPutStr("sdsd", "DEBUG: reversing: prev:", prev_sensor, " next:", next_sensor);
        pathFindDijkstra(
            &md,
            prev_sensor,          // current node
            0,                      // offset
            next_sensor,                 // where it wants to go
            790,                    // stoping distance
            blocked_nodes,          // the nodes the trains can't use
            0                       // the length of the blocked nodes array
        );
        DebugPutStr("sd", "DEBUG: edge length is: ", md.total_distance);
        train_info[TRAIN_INFO_SENSOR_OFFSET] = md.total_distance - train_info[TRAIN_INFO_SENSOR_OFFSET];
        train_info[TRAIN_INFO_SENSOR] = prev_sensor;
        train_info[TRAIN_INFO_NEXT_SENSOR] = next_sensor;
        reverseTrain(train_info[TRAIN_INFO_ID]);
        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
    else if (md.type == SHORT_MOVE) {
        stopping_sensor_dist = md.total_distance - train_info[TRAIN_INFO_SENSOR_OFFSET];
        if (stopping_sensor == train_info[TRAIN_INFO_DEST_SENSOR] ){
            stopping_sensor_dist += offset;
        }
        if (train_info[TRAIN_INFO_REVERSED]){
            stopping_sensor_dist -= TRAIN_REVERSE_OFFSET;
        }
        if (stopping_sensor_dist < 0){
            stopping_sensor_dist = 0;
        }
        msg_struct.value[0] = -1;
        msg_struct.iValue = shortMoveDistanceToDelay(stopping_sensor_dist, train_info[TRAIN_INFO_ID]);
        DebugPutStr("s", "DEBUG: Short Move");

        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
        setTrainSpeed (train_info[TRAIN_INFO_ID], 12);

    }
    else if (md.type == LONG_MOVE) {
        if (stopping_sensor == train_info[TRAIN_INFO_DEST_SENSOR] ){
            stopping_sensor_dist += offset;
        }
        if (train_info[TRAIN_INFO_REVERSED]){
            stopping_sensor_dist -= TRAIN_REVERSE_OFFSET;
        }
        if (stopping_sensor_dist < 0){
            stopping_sensor_dist = 0;
        }
        int stop_delay = distanceToDelay( stopping_sensor, stopping_sensor_dist, train_speed);

        // start the notifier
        msg_struct.value[0] = stopping_sensor;
        msg_struct.iValue = stop_delay;
        DebugPutStr("sdsd", "DEBUG: Long Move: stopping at:", stopping_sensor, " delay:", stop_delay);

        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
        // set the train speed to 12
        setTrainSpeed (train_info[TRAIN_INFO_ID], 12);
    }
}

int timeToDistance( int sensor, int delta_time, int *train_speed ) {
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;
    int distance = delta_time * velocity;
    return distance;
}

int distanceToDelay( int sensor, int distance, int *train_speed ) {
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;
    int delta_time = (int) distance / velocity;
    return delta_time;
}

int shortMoveDistanceToDelay( double distance, int train_num ) {
    double cubic;
    double square;
    double linear;
    double constant;
    double result = -1;
    if (train_num == 49) {
        cubic = 2.9053E-7 * distance * distance * distance;
        square = 0.000753205 * distance * distance;
        linear = 0.737617 * distance;
        constant = 46.55;
        result = (cubic - square + linear + constant);
    }
    else {
        cubic = 1.5E-7 * distance * distance * distance;
        square = 0.000489658 * distance * distance;
        linear = 0.611907 * distance;
        constant = 62.3535;
        result = (cubic - square + linear + constant);
    }
    return (int) result;
}

int predictArrivalTime( int sensor, int next_sensor, int init_time, int *train_speed) {
    int delta_distance = findDistanceBetweenLandmarks( sensor, next_sensor, 10);  //distance is in mm
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;

    int delta_time = (int) delta_distance / velocity;
    int ret =  init_time + delta_time;
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

int setTrainDestination( int train_id, int sensor, int offset ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10, rpllen = 10;
    int receiver_tid, train_task_id;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TRAIN_DESTINATION_REQUEST;
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
    msg_struct.value[1] = (char) offset;
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

int getTrainLocation ( int train_id, int* sensor, int* offset) {
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
    msg_struct.type = GET_TRAIN_LOCATION_REQUEST;
    reply_struct.value = reply;
    reply_struct.iValue = 0;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        *sensor = reply_struct.value[0];
        *offset = reply_struct.iValue;
        return 1;
    }
    return -1;
}
