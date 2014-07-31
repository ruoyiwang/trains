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
    reply_struct.value = reply;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    Reply (server_tid, (char *)&reply_struct, rpllen);

    DebugPutStr("sdsd", "DEBUG: notifier: ", msg_struct.value[0], " : ", msg_struct.iValue);

    int is_short_move = 0;
    int delay = (int) msg_struct.iValue;
    char sensor_wait[2] = {-1};
    sensor_wait[0] = msg_struct.value[0];


    if (!(msg_struct.value[0] < (char) 80)) {
        Delay(delay);
        is_short_move = 1;
        reply_struct.type = COMMAND_CENTER_STOPPING_NOTIFIER;
        reply_struct.value[0] = 1;
        reply_struct.value[1] = sensor_wait[0];
        Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    }
    else {//if (!is_short_move){
        waitForSensors(sensor_wait, 1, 1000000);
        reply_struct.type = COMMAND_CENTER_STOPPING_NOTIFIER;
        reply_struct.value[0] = 0;
        reply_struct.value[1] = sensor_wait[0];
        Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);

        while (msg_struct.iValue) {
            waitForSensors(sensor_wait, 1, 1000000);
            reply_struct.type = COMMAND_CENTER_STOPPING_NOTIFIER;
            reply_struct.value[0] = 0;
            reply_struct.value[1] = sensor_wait[0];
            Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
        }
    }


    if (is_short_move) {
        // DebugPutStr("s", "FAST STOP");
        Delay(delay+50);
    }
    else {
        Delay(670);
    }
    reply_struct.type = COMMAND_CENTER_TRAIN_STOPPED;
    Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    Exit();
}

void CommandCenterDelayNotifier() {
    int server_tid;

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    Reply (server_tid, (char *)&reply_struct, rpllen);

    DebugPutStr("sd", "DEBUG: delaying for : ", msg_struct.iValue);
    int delay = (int) msg_struct.iValue;
    Delay(delay);
    reply_struct.type = COMMAND_CENTER_DELAY_EXPIRED;
    Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    Exit();
}

void CommandCenterAdjustTask() {
    int server_tid;

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    Reply (server_tid, (char *)&reply_struct, rpllen);
    if (!isUnsafeForward((int)msg_struct.value[1])){
        DebugPutStr("sd", "DEBUG: Off target Adjusting for ", msg_struct.value[1]);
        setTrainSpeed((int)msg_struct.value[0], 3);
        waitForSensors(msg_struct.value+1, 2, 1000000);
        if (msg_struct.value[3] == false) {
            Delay(70);
        }
        if (isUnsafeReverse ((int)msg_struct.value[1])){
            Delay(70);
        }
        setTrainSpeed((int)msg_struct.value[0], 0);
        Delay(100);
    }
    reply_struct.type = COMMAND_CENTER_TRAIN_STOPPED;
    Send (server_tid, (char *)&reply_struct, rpllen, (char *)&msg_struct, msglen);
    Exit();
}

void CommandCenterTrainCheckNotifier() {
    int server_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = COMMAND_CENTER_TRAIN_CHECK;
    reply_struct.value = reply;

    FOREVER{
        Delay(10);
        Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
}

void CommandCenterTrainDeadlockNotifier() {
    int server_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = COMMAND_CENTER_DEADLOCK_CHECK;
    reply_struct.value = reply;

    FOREVER{
        Delay(200);
        Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
}

void CommandCenterDeadlockBuster() {
    int server_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);

    char msg[11] = {0};
    char reply[11] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    Receive( &server_tid, (char*)&msg_struct, msglen );
    Reply (server_tid, (char *)&reply_struct, rpllen);

    msg_struct.type = COMMAND_CENTER_DEADLOCK_CHECK;
    if (msg_struct.value[0]){
        reverseTrain(msg_struct.iValue);
    }
    setTrainSpeed(msg_struct.iValue, 3);
    Delay(1000);
    setTrainSpeed(msg_struct.iValue, 0);
    Delay(100);
    if (msg_struct.value[0]){
        Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
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
    char reserve_sensors[80] = {-1};
    int courier_tid, notifier_tid, train_count = 0, length_to_switch = 0, merge_ahead = false;
    int is_deadlock = false, look_ahead_count = 0;

    int sensors_list[80] = {0};
    int blocked_nodes[20];
    move_data md;
    // int train_info[MAX_TRAIN_COUNT][TRAIN_INFO_SIZE];
    Train_info train_info[MAX_TRAIN_COUNT];
    int train_speed[MAX_TRAIN_COUNT][80];

    int deadlock_mode = false;

    // each tid can only call one delay at a time
    int i, j = -1;
    for (i = 0; i < MAX_TRAIN_COUNT; i++) {
        train_info[i].id = -1;
        train_info[i].sensor = -1;
        train_info[i].next_sensor = -1;
        train_info[i].offset = 0;
        train_info[i].arrival_tme = 0;
        train_info[i].task_tid = -1;
        train_info[i].courier_tid = -1;
        train_info[i].time_prediction = 0;
        train_info[i].stopping_notifier = -1;
        train_info[i].stopping_sensor = -1;
        train_info[i].stopping_offset = 0;
        train_info[i].is_stopped = 1;
        train_info[i].timeout = 300;
        train_info[i].dest_sensor = -1;
        train_info[i].dest_offset = 0;
        train_info[i].is_reversed = 0;
        train_info[i].stopping_dist = 790;
        train_info[i].short_move_mult = 100;
        train_info[i].speed_mult = 100;
        train_info[i].off_course = 0;
        train_info[i].signal = -100;
        train_info[i].notifier_tid = -1;
        intQueueInit(&(train_info[i].dest_queue));
    }

    char deadlock_locations[5] = {74, 68, 3, 65, 51};

    RegisterAs(COMMAND_CENTER_SERVER_NAME);
    int deadlock_check_tid = Create(1, (&CommandCenterTrainDeadlockNotifier));

    volatile unsigned int * timer_4_low;
    timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {

            case COMMAND_CENTER_NOTIFIER:
                // handle notifer for sensors that are triggered
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    // find the right train
                    if (train_info[i].courier_tid == sender_tid) {
                        freeNodes(reserve_sensors, 0, train_info[i].id);

                        actual_time = Time();
                        train_info[i].arrival_tme = actual_time;

                        // get which sensor was triggered
                        train_info[i].sensor = msg_struct.iValue;
                        // get the next sensor
                        predictSensor(train_info[i].sensor, 4, sensors_ahead);
                        train_info[i].next_sensor = sensors_ahead[0];
                        // get time the sensor is triggered
                        actual_time = Time();
                        train_info[i].arrival_tme = actual_time;
                        // set the sensor offset back to 0
                        train_info[i].offset = 0;


                        // check if there were any request on the blocked sensor
                        for (j=0; j < 64; j++) {
                            if (requests[j] == train_info[i].id) {
                                // return the sensors and the expected and predicted time
                                requests[j] = -1;
                                reply_struct.value[0] = train_info[i].sensor;
                                reply_struct.value[1] = train_info[i].next_sensor;
                                reply_struct.value[2] = train_info[i].dest_sensor;
                                bwi2a(train_info[i].time_prediction, reply + 3);
                                reply_struct.iValue = train_info[i].arrival_tme;
                                Reply (j, (char *)&reply_struct, 30);
                                break;
                            }
                        }
                        reserve_sensors[0] = train_info[i].sensor;
                        reserve_sensors[1] = getSensorComplement(train_info[i].sensor);
                        reserveNodesRequest(reserve_sensors, 2, train_info[i].id);
                        int sensors_list_return_len;
                        nextPossibleSensorsCheck (sensors_list, 80, train_info[i].sensor, 2, train_info[i].id, &sensors_list_return_len);
                        for (j = 0; j < sensors_list_return_len; j++ ){
                            reserve_sensors[j] = getSensorComplement(sensors_list[j]);
                        }
                        reserveNodesRequest(reserve_sensors, sensors_list_return_len, train_info[i].id);

                        merge_ahead = false;
                        for (j = 0; j < train_info[i].node_list_len; j++ ){
                            // DebugPutStr("sd", "DEBUG: node: ", train_info[i].node_list[j].num);
                            if (train_info[i].node_list[j].num == train_info[i].sensor) {
                                // DebugPutStr("sdsd", "DEBUG: current index: ", j, " of ", train_info[i].node_list_len);
                                break;
                            }
                        }
                        for ( ;j < train_info[i].node_list_len; j++ ){
                            if (train_info[i].node_list[j].type == NODE_MERGE) {
                                merge_ahead = true;
                                break;
                            }
                            if (train_info[i].node_list[j].type == NODE_SENSOR) {
                                break;
                            }
                        }
                        if (merge_ahead){
                            reserve_sensors[0] = train_info[i].next_sensor;
                            reserveNodesRequest(reserve_sensors, 1, train_info[i].id);
                        }


                        // predict the time of arrival for the next sensor
                        expected_time = predictArrivalTime(train_info[i].sensor,
                                                                     train_info[i].next_sensor,
                                                                     train_info[i].arrival_tme,
                                                                     train_speed[i],
                                                                     train_info[i].speed_mult);
                        train_info[i].time_prediction = expected_time;
                        break;
                    }
                }
                // give the sensor to wait on as well as the time out to the notifier
                reply_struct.iValue = train_info[i].arrival_tme + train_info[i].timeout;
                reply_struct.value[0] = train_info[i].next_sensor;
                reply_struct.value[1] = sensors_ahead[1];
                reply_struct.value[2] = getSensorComplement(train_info[i].sensor);
                predictSensor(reply_struct.value[2], 2, sensors_ahead);
                reply_struct.value[3] = sensors_ahead[0];
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_TRAIN_CHECK:
                if (deadlock_mode) {
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    break;
                }

                reply_struct.iValue = false;
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i].check_notifier == sender_tid) {

                        look_ahead_count = 1;
                        for ( j = train_info[i].node_list_len-1 ; j>=0; j-- ){
                            if (train_info[i].node_list[j].num == train_info[i].sensor) {
                                break;
                            }
                            if (train_info[i].node_list[j].type == NODE_SENSOR) {
                                look_ahead_count++;
                            }
                        }
                        if (look_ahead_count > 4) {
                            look_ahead_count = 4;
                        }
                        int sensors_list_return_len;
                        if (train_info[i].signal != RESERVED_STOPPING && !train_info[i].is_stopped
                            && !nextPossibleSensorsCheck (sensors_list, 80, train_info[i].sensor, look_ahead_count, train_info[i].id, &sensors_list_return_len)) {
                            // Assert();
                            // somehting is blocking the way
                            setTrainSpeed (train_info[i].id, 0);

                            train_info[i].signal = RESERVED_STOPPING;
                            notifier_tid = Create(1, (&CommandCenterDelayNotifier));
                            train_info[i].stopping_notifier = notifier_tid;
                            msg_struct.iValue = 500;
                            Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
                        }
                        // check if switches need to be set
                        if (train_info[i].dest_sensor != -1) {
                            for (j = 0; j < train_info[i].node_list_len; j++ ){
                                // DebugPutStr("sd", "DEBUG: node: ", train_info[i].node_list[j].num);
                                if (train_info[i].node_list[j].num == train_info[i].sensor) {
                                    // DebugPutStr("sdsd", "DEBUG: current index: ", j, " of ", train_info[i].node_list_len);
                                    break;
                                }
                            }
                            length_to_switch = 0;
                            for (;j < train_info[i].node_list_len; j++ ){
                                if (train_info[i].node_list[j].type == NODE_BRANCH) {
                                    if ( (length_to_switch - train_info[i].offset) < 1000) {
                                        setSwitch(train_info[i].node_list[j].branch_state, train_info[i].node_list[j].num);

                                        predictSensor(train_info[i].sensor, 2, sensors_ahead);
                                        train_info[i].next_sensor = sensors_ahead[0];
                                        changeWaitForSensors(train_info[i].notifier_tid, sensors_ahead, 2 );

                                    }
                                }
                                length_to_switch += train_info[i].node_list[j].dist_to_next;
                            }
                            // DebugPutStr("sd", "DEBUG: length to switch: ", length_to_switch);
                        }
                        break;
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_STOPPING_NOTIFIER:
                // handle stoping notifer, this means the train needs to stop now
                reply_struct.iValue = false;
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i].stopping_notifier == sender_tid) {
                        // stop train
                        DebugPutStr("sd", "DEBUG: Train Stopped for move type : ", msg_struct.value[0]);
                        if (msg_struct.value[0] == 0) {
                            if (msg_struct.value[1] == train_info[i].sensor || msg_struct.value[1] == train_info[i].sensor){
                                setTrainSpeed (train_info[i].id, 0);
                            }
                            else {
                                reply_struct.iValue = true;
                            }
                        }
                        else {
                            setTrainSpeed (train_info[i].id, 0);
                        }
                        break;
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_TRAIN_STOPPED:
                // handle stoping notifer, this means the train needs to stop now
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i].stopping_notifier == sender_tid) {
                        if (train_info[i].sensor == train_info[i].stopping_sensor || train_info[i].sensor == train_info[i].dest_sensor
                            || (isUnsafeForward(train_info[i].stopping_sensor) && train_info[i].next_sensor == train_info[i].stopping_sensor)){// || train_info[i].next_sensor == train_info[i].stopping_sensor
                            // || train_info[i].signal == UNSAFE_REVERSE || train_info[i].signal == SAFE_REVERSE) {
                            DebugPutStr("sd", "DEBUG: Train Stopped at sensor : ", train_info[i].sensor);
                            if (train_info[i].sensor == train_info[i].stopping_sensor) {
                                train_info[i].sensor = train_info[i].stopping_sensor;
                                train_info[i].offset = train_info[i].stopping_offset;
                            }
                            else {
                                train_info[i].sensor = train_info[i].dest_sensor;
                                train_info[i].offset = train_info[i].dest_offset;
                            }


                            train_info[i].is_stopped = 1;
                            changeWaitForSensors(train_info[i].notifier_tid, sensors_ahead, 0 );

                            train_info[i].stopping_sensor = -1;
                            train_info[i].stopping_offset = 0;
                            train_info[i].stopping_notifier = -1;
                            if (train_info[i].dest_sensor == train_info[i].sensor ||
                                train_info[i].dest_sensor == train_info[i].next_sensor) {
                                train_info[i].dest_sensor = -1;
                                train_info[i].dest_offset = 0;
                                DebugPutStr("s", "DEBUG: Arrived at destination!");

                                // check if the queue is empty if not, we set dest again
                                if (intQueuePeek(&(train_info[i].dest_queue)) >= 0) {
                                    train_info[i].dest_sensor = intQueuePop(&(train_info[i].dest_queue));
                                    serverSetStopping(&(train_info[i]), train_speed[i], train_info[i].dest_sensor, 0, requests);
                                    DebugPutStr("sd", "DEBUG: Routing TO: ", train_info[i].dest_sensor);
                                }
                                else {
                                    while ( true ) {
                                        train_info[i].dest_sensor = rand(*timer_4_low) % 80;
                                        if (train_info[i].dest_sensor < 16 ||
                                            train_info[i].dest_sensor > 27) {
                                            break;
                                        }
                                    }
                                    serverSetStopping(&(train_info[i]), train_speed[i], train_info[i].dest_sensor, 0, requests);
                                    DebugPutStr("sd", "DEBUG: Going to Random Location: ", train_info[i].dest_sensor);
                                }

                                break;
                            }

                            train_info[i].signal = -100;
                            serverSetStopping(&(train_info[i]), train_speed[i], train_info[i].dest_sensor, train_info[i].dest_offset, requests);
                            break;
                        }
                        else if (train_info[i].next_sensor == train_info[i].stopping_sensor && train_info[i].signal != UNSAFE_FORWORD){
                            train_info[i].signal = -100;
                            //try to reach target
                            notifier_tid = Create(1, (&CommandCenterAdjustTask));
                            train_info[i].stopping_notifier = notifier_tid;
                            msg_struct.value[0] = train_info[i].id;
                            msg_struct.value[1] = train_info[i].stopping_sensor;
                            predictSensor(train_info[i].stopping_sensor, 2, sensors_ahead);
                            msg_struct.value[2] = sensors_ahead[0];
                            msg_struct.value[3] = train_info[i].is_reversed;
                            Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
                            train_info[i].off_course = 1;
                            break;
                        }
                        else {
                            DebugPutStr("sd", "DEBUG: Train is Off course! heading to ",train_info[i].dest_sensor);
                            train_info[i].is_stopped = 1;
                            changeWaitForSensors(train_info[i].notifier_tid, sensors_ahead, 0 );
                            train_info[i].offset = 0;
                            train_info[i].stopping_sensor = -1;
                            train_info[i].stopping_offset = 0;
                            train_info[i].stopping_notifier = -1;
                            train_info[i].signal = -100;
                            sensors_ahead[0] = ANY_SENSOR_REQUEST;
                            changeWaitForSensors(train_info[i].notifier_tid, sensors_ahead, 1);
                            serverSetStopping(&(train_info[i]), train_speed[i], train_info[i].dest_sensor, train_info[i].dest_offset, requests);
                        }
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_DELAY_EXPIRED:
                // handle stoping notifer, this means the train needs to stop now
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i].stopping_notifier == sender_tid) {
                        if (train_info[i].dest_sensor > 0) {
                            DebugPutStr("sd", "DEBUG: Trying to reroute to ",train_info[i].dest_sensor);
                            train_info[i].is_stopped = 1;
                            changeWaitForSensors(train_info[i].notifier_tid, sensors_ahead, 0 );

                            train_info[i].offset = 0;
                            train_info[i].stopping_sensor = -1;
                            train_info[i].stopping_offset = 0;
                            train_info[i].stopping_notifier = -1;
                            train_info[i].signal = -100;
                            serverSetStopping(&(train_info[i]), train_speed[i], train_info[i].dest_sensor, train_info[i].dest_offset, requests);
                        }
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case GET_TRAIN_LOCATION_REQUEST:
                // handle request for the trains location relative to its previous sensor
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.iValue == train_info[i].id) {
                        // if train is stopped
                        if (train_info[i].is_stopped && train_info[i].arrival_tme >= 0) {
                            // if the train stopped really close the the stopping sensor
                            if (train_info[i].next_sensor == train_info[i].stopping_sensor
                                && train_info[i].stopping_offset < 5) {
                                // the offset is the distance between the sensors
                                total_distance = findDistanceBetweenLandmarks( train_info[i].sensor, train_info[i].next_sensor, 2);
                                train_info[i].offset = total_distance;
                                break;
                            }
                            else if (train_info[i].sensor == train_info[i].stopping_sensor) {
                                // the stopping sensor is crossed return the stopping offset
                                train_info[i].offset = train_info[i].stopping_offset;
                                break;
                            }
                        }
                        // train was just initialized
                        else if (train_info[i].is_stopped){
                            train_info[i].offset = 0;
                        }
                        else {
                            // if the train is moving, calculate the offset based on speed and time
                            actual_time = Time();
                            total_distance = findDistanceBetweenLandmarks( train_info[i].sensor, train_info[i].next_sensor, 100);
                            train_info[i].offset = timeToDistance( train_info[i].sensor,
                                                    actual_time - train_info[i].arrival_tme, train_speed[i], train_info[i].speed_mult );
                            // cannot be greater than the max distance
                            // DebugPutStr("sdsd", "DEBUG: time:", actual_time - train_info[i].arrival_tme, " distance:", total_distance);
                            if (train_info[i].offset > total_distance) {
                                train_info[i].offset = total_distance;
                            }
                        }
                        break;
                    }
                }
                reply_struct.value[0] = train_info[i].sensor;
                reply_struct.iValue = train_info[i].offset;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case INIT_TRAIN_REQUEST:
                j = train_count;
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.value[0] == train_info[i].id) {
                        j = i;
                        break;
                    }
                }
                // handle the request to start a new train
                train_info[j].sensor = msg_struct.iValue;
                train_info[j].id = (int) msg_struct.value[0];
                train_info[j].task_tid = (int) msg_struct.value[1];

                // initialize the notifier and the courier
                notifier_tid = Create(1, (&CommandCenterNotifier));
                courier_tid = Create(2, (&CommandCenterCourier));
                train_info[j].courier_tid = courier_tid;
                train_info[j].notifier_tid = notifier_tid;
                msg_struct.iValue = notifier_tid;

                // pass the sensors to be waited on to the notifier, there is no time out on first wait
                predictSensor(train_info[j].sensor, 2, sensors_ahead);
                train_info[j].next_sensor = sensors_ahead[0];

                reserve_sensors[0] = train_info[i].sensor;
                reserve_sensors[1] = getSensorComplement(train_info[j].next_sensor);
                reserve_sensors[2] = getSensorComplement(train_info[j].sensor);
                reserveNodesRequest(reserve_sensors, 3, train_info[j].id);

                train_info[j].check_notifier = Create(1, (&CommandCenterTrainCheckNotifier));

                DebugPutStr("sdsd", "DEBUG: from:", train_info[j].sensor, " to:", train_info[train_count].next_sensor);
                msg_struct.value[0] = train_info[j].next_sensor;
                msg_struct.value[1] = sensors_ahead[1];
                msg_struct.value[2] = getSensorComplement(train_info[j].sensor);
                predictSensor(msg_struct.value[2], 2, sensors_ahead);
                msg_struct.value[3] = sensors_ahead[0];
                Send (courier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

                // initialize the train speed table
                init_train_speed(train_info[j].id, train_speed[j]);
                get_train_calibrations(train_info[j].id, &(train_info[j].stopping_dist), &(train_info[j].short_move_mult), &(train_info[j].speed_mult));
                if (j == train_count){
                    train_count++;
                }
                for (i=0; i < 64; i++) {
                    if (requests[i] == train_info[j].id) {
                        // return the sensors and the expected and predicted time
                        reply_struct.value[0] = train_info[j].sensor;
                        reply_struct.value[1] = train_info[j].next_sensor;
                        reply_struct.value[2] = train_info[j].dest_sensor;
                        bwi2a(train_info[j].time_prediction, reply + 3);
                        reply_struct.iValue = train_info[j].arrival_tme;
                        Reply (i, (char *)&reply_struct, 30);
                        break;
                    }
                }
                j = -1;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case COMMAND_CENTER_DEADLOCK_CHECK:
                if (deadlock_mode && sender_tid == deadlock_check_tid){
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    break;
                }
                if (deadlock_mode) {
                    for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                        if (train_info[i].id != -1 && train_info[i].id != msg_struct.iValue) {
                            DebugPutStr("sds", "DEADLOCK: second train ", train_info[i].id, " looking for sensor");
                            notifier_tid = Create(1, (&CommandCenterDeadlockBuster));
                            msg_struct.iValue = train_info[i].id;
                            msg_struct.value[0] = false;
                            Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
                        }
                    }
                    deadlock_mode = false;
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    break;
                }

                is_deadlock = false;
                // DebugPutStr("s", "DEBUG: CHECKING FOR DEADLOCKS");
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (train_info[i].id != -1) {
                        is_deadlock = true;

                        if (!train_info[i].is_stopped || train_info[i].dest_sensor == -1){
                            is_deadlock = false;
                            break;
                        }
                        if (pathFindDijkstra(
                            &md,
                            train_info[i].sensor,          // current node
                            train_info[i].offset,                      // offset
                            train_info[i].dest_sensor,                 // where it wants to go
                            train_info[i].stopping_dist,                    // stoping distance
                            blocked_nodes,          // the nodes the trains can't use
                            0,                       // the length of the blocked nodes array
                            train_info[i].id
                        )){
                            is_deadlock = false;
                            break;
                        }
                        if (pathFindDijkstra(
                            &md,
                            getSensorComplement(train_info->sensor),          // current node
                            train_info[i].offset,                      // offset
                            train_info[i].dest_sensor,                 // where it wants to go
                            train_info[i].stopping_dist,                    // stoping distance
                            blocked_nodes,          // the nodes the trains can't use
                            0,                       // the length of the blocked nodes array
                            train_info[i].id
                        )) {
                            is_deadlock = false;
                            break;
                        }

                    }
                }
                if (is_deadlock) {
                    DebugPutStr("s", "DEBUG: ENTERING DEADLOCK MODE!");

                    // enter deadlock mode
                    deadlock_mode = true;
                    int last_arrival = -1, train_id;
                    for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                        if (train_info[i].id != -1) {
                            if (train_info[i].arrival_tme > last_arrival) {
                                last_arrival = train_info[i].arrival_tme;
                                train_id = train_info[i].id;
                            }
                            train_info[i].offset = 0;
                            train_info[i].stopping_sensor = -1;
                            train_info[i].stopping_offset = 0;
                            train_info[i].stopping_notifier = -1;
                            train_info[i].signal = -100;
                            sensors_ahead[0] = ANY_SENSOR_REQUEST;
                            setTrainSpeed(train_info[i].id, 0);
                            changeWaitForSensors(train_info[i].notifier_tid, sensors_ahead, 1);
                        }
                    }
                    DebugPutStr("sds", "DEADLOCK: train ", train_id, " looking for sensor");
                    notifier_tid = Create(1, (&CommandCenterDeadlockBuster));
                    msg_struct.iValue = train_id;
                    msg_struct.value[0] = true;
                    Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
                }

                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case TRAIN_DESTINATION_REQUEST:
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.value[0] == train_info[i].id) {
                        // if the train is not currently enroute to anywhere
                        if (train_info[i].dest_sensor == -1) {
                            train_info[i].dest_sensor = msg_struct.iValue;
                            train_info[i].dest_offset = (int) msg_struct.value[1]*10;

                            serverSetStopping(&(train_info[i]), train_speed[i], msg_struct.iValue, (int) msg_struct.value[1]*10, requests);
                        }
                        // else we queue the node
                        else {
                            intQueuePush(&(train_info[i].dest_queue), msg_struct.iValue);
                        }
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case GET_TRAIN_INFO_REQUEST:
                requests[sender_tid] = msg_struct.iValue;
                break;

            case SET_TRAIN_STOPPING_DIST:
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.value[0] == train_info[i].id) {
                        train_info[i].stopping_dist = msg_struct.iValue;
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;

            case SET_TRAIN_SHORT_MULT:
                for (i = 0; i < MAX_TRAIN_COUNT; i++) {
                    if (msg_struct.value[0] == train_info[i].id) {
                        train_info[i].short_move_mult = msg_struct.iValue;
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            default:
                bwprintf(COM2, "fmlllll COMMAND CENTER SEVER %d\n", msg_struct.type);
                reply_struct.type = FAIL_TYPE;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
        }
    }
}

void serverSetStopping (Train_info* train_info, int* train_speed, int stop_sensor, int stop_offset, int* requests) {
    char msg[11] = {0};
    char reply[30] = {0};
    int msglen = 10, rpllen = 10;

    char sensors_ahead[5] = {-1};
    char reserve_sensors[5] = {-1};
    move_data md, temp;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    // location = train_info[i].offset / 10;
    // initialize the stoping notifier
    int stopping_sensor_dist = 0, stopping_sensor = -1, need_reverse = false;
    int blocked_nodes[20];
    // update the train info
    int notifier_tid;
    DebugPutStr("sdsdsd", "DEBUG: routing from ", train_info->sensor, ":", train_info->offset, " to ", stop_sensor);
    // find path which will also set the switches
    if (train_info->sensor == stop_sensor){
        return;
    }
    if (!pathFindDijkstra(
        &md,
        train_info->sensor,          // current node
        train_info->offset,                      // offset
        stop_sensor,                 // where it wants to go
        train_info->stopping_dist,                    // stoping distance
        blocked_nodes,          // the nodes the trains can't use
        0,                       // the length of the blocked nodes array
        train_info->id
    )){
        if (!pathFindDijkstra(
            &md,
            getSensorComplement(train_info->sensor),          // current node
            train_info->offset,                      // offset
            stop_sensor,                 // where it wants to go
            train_info->stopping_dist,                    // stoping distance
            blocked_nodes,          // the nodes the trains can't use
            0,                       // the length of the blocked nodes array
            train_info->id
        )) {
            DebugPutStr("sds", "DEBUG: train ", train_info->id,": No route found, retrying in 1 second");
            setTrainSpeed (train_info->id, 0);
            // no route found, try again in 1 second;
            notifier_tid = Create(1, (&CommandCenterDelayNotifier));
            train_info->stopping_notifier = notifier_tid;
            msg_struct.iValue = 100;
            Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
            return;
        }
        else {
            need_reverse = true;
        }
    }


    notifier_tid = Create(1, (&CommandCenterStoppingNotifier));
    train_info->stopping_notifier = notifier_tid;
    train_info->timeout = 300;
    // need to addjust more if unsafe
    // if ( md.type == UNSAFE_REVERSE && train_info->off_course){
    //     pathFindDijkstra(
    //         &md,
    //         getSensorComplement(train_info->sensor),          // current node
    //         30,                      // offset
    //         stop_sensor,                 // where it wants to go
    //         train_info->stopping_dist,                    // stoping distance
    //         blocked_nodes,          // the nodes the trains can't use
    //         0,                       // the length of the blocked nodes array
    //         train_info->id
    //     );

    //     int i;
    //     for (i = 0; i < md.list_len; i++) {
    //         if (md.node_list[i].type == NODE_BRANCH) {
    //             setSwitch(md.node_list[i].branch_state, md.node_list[i].num);
    //         }
    //     }
    //     train_info->signal = UNSAFE_OFF_COURSE_SIGNAL;

    //     train_info->is_stopped = 0;
    //     train_info->stopping_sensor = train_info->sensor;
    //     if (train_info->is_reversed){
    //         train_info->stopping_offset = 5;
    //     }
    //     else {
    //         train_info->stopping_offset = 15;
    //     }
    //     train_info->stopping_offset = 30;
    //     msg_struct.value[0] = -1;
    //     msg_struct.iValue = shortMoveDistanceToDelay(train_info->stopping_offset, train_info->id, train_info->short_move_mult);
    //     DebugPutStr("s", "DEBUG: Short Move");

    //     Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    //     setTrainSpeed (train_info->id, 12);
    //     train_info->off_course = 0;
    //     return;
    // }
    int i;
    memcpy(train_info->node_list, md.node_list, md.list_len * sizeof(move_node));

    train_info->node_list_len = md.list_len;
    DebugPutStr("sd", "DEBUG: found route :", md.type);
    stopping_sensor = md.stopping_sensor;
    stopping_sensor_dist = md.stopping_dist;

    train_info->is_stopped = 0;
    train_info->stopping_sensor = stop_sensor;
    train_info->stopping_offset = stop_offset;
    train_info->stopping_sensor = md.node_list[md.list_len-1].num;

    train_info->signal = -100;
    if (md.unsafe_forward){
        train_info->signal = UNSAFE_FORWORD;
    }
    if (md.reverse_first || need_reverse){
        // set first 2 switches if reverse
        for (i = 0; i < md.list_len; i++) {
            if (md.node_list[i].type == NODE_BRANCH) {
                setSwitch(md.node_list[i].branch_state, md.node_list[i].num);
                break;
            }
        }
        i++;
        for (; i < md.list_len; i++) {
            if (md.node_list[i].type == NODE_BRANCH) {
                setSwitch(md.node_list[i].branch_state, md.node_list[i].num);
                break;
            }
        }
        if (train_info->is_reversed){
            train_info->is_reversed = 0;
        }
        else {
            train_info->is_reversed = 1;
        }
        // msg_struct.value[0] = -1;
        // msg_struct.iValue = 1;
        int next_sensor, prev_sensor;
        prev_sensor = getSensorComplement(train_info->next_sensor);
        next_sensor = getSensorComplement(train_info->sensor);
        predictSensor(next_sensor, 1, sensors_ahead);
        sensors_ahead[1] = sensors_ahead[0];
        sensors_ahead[0] = next_sensor;
        changeWaitForSensors(train_info->notifier_tid, sensors_ahead, 2 );

        DebugPutStr("sdsd", "DEBUG: reversing: prev:", prev_sensor, " next:", next_sensor);
        pathFindDijkstra(
            &temp,
            prev_sensor,          // current node
            0,                      // offset
            next_sensor,                 // where it wants to go
            train_info->stopping_dist,                    // stoping distance
            blocked_nodes,          // the nodes the trains can't use
            0,                       // the length of the blocked nodes array
            train_info->id
        );
        DebugPutStr("sd", "DEBUG: edge length is: ", temp.total_distance);
        train_info->offset = temp.total_distance - train_info->offset;
        train_info->sensor = prev_sensor;
        train_info->next_sensor = next_sensor;
        reverseTrain(train_info->id);
        // Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
    if (md.type == SHORT_MOVE) {
        int sensors_count = 0;
        for (i = 0; i < md.list_len; i++) {
            if (md.node_list[i].type == NODE_BRANCH) {
                setSwitch(md.node_list[i].branch_state, md.node_list[i].num);
            }
            else if (md.node_list[i].type == NODE_SENSOR) {
                reserve_sensors[sensors_count++] = getSensorComplement(md.node_list[i].num);
            }
        }

        // reserveNodesRequest(reserve_sensors, sensors_count, train_info->id);
        predictSensor(train_info->next_sensor, 1, sensors_ahead);
        sensors_ahead[1] = sensors_ahead[0];
        sensors_ahead[0] = train_info->next_sensor;
        changeWaitForSensors(train_info->notifier_tid, sensors_ahead, 2 );

        stopping_sensor_dist = md.total_distance;// - train_info->offset;
        if (stopping_sensor == train_info->dest_sensor ){
            stopping_sensor_dist += stop_offset;
        }
        if (train_info->is_reversed){
            stopping_sensor_dist -= TRAIN_REVERSE_OFFSET;
        }
        if (stopping_sensor_dist < 0){
            stopping_sensor_dist = 0;
        }
        // if (isUnsafeReverse(train_info->stopping_sensor)) {
        //     stopping_sensor_dist += TRAIN_LENGTH;
        // }
        msg_struct.value[0] = -1;
        msg_struct.iValue = shortMoveDistanceToDelay(stopping_sensor_dist, train_info->id, train_info->short_move_mult);
        DebugPutStr("sd", "DEBUG: Short Move for: ", msg_struct.iValue);
        setTrainSpeed (train_info->id, 12);

        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    }
    else if (md.type == LONG_MOVE) {

        predictSensor(train_info->next_sensor, 1, sensors_ahead);
        sensors_ahead[1] = sensors_ahead[0];
        sensors_ahead[0] = train_info->next_sensor;
        changeWaitForSensors(train_info->notifier_tid, sensors_ahead, 2 );

        if (stopping_sensor == train_info->dest_sensor ){
            stopping_sensor_dist += stop_offset;
        }
        if (train_info->is_reversed){
            stopping_sensor_dist -= TRAIN_REVERSE_OFFSET;
        }
        if (stopping_sensor_dist < 0){
            stopping_sensor_dist = 0;
        }
        int stop_delay = distanceToDelay( stopping_sensor, stopping_sensor_dist, train_speed, train_info->speed_mult);

        // start the notifier
        msg_struct.value[0] = stopping_sensor;
        msg_struct.iValue = stop_delay;
        DebugPutStr("sdsd", "DEBUG: Long Move: stopping at:", stopping_sensor, " delay:", stop_delay);

        setTrainSpeed (train_info->id, 12);
        Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
        // set the train speed to 12
    }
    train_info->off_course = 0;

    // check if there were any request on the blocked sensor
    for (i=0; i < 64; i++) {
        if (requests[i] == train_info->id) {
            // return the sensors and the expected and predicted time
            reply_struct.value[0] = train_info->sensor;
            reply_struct.value[1] = train_info->next_sensor;
            reply_struct.value[2] = train_info->dest_sensor;
            bwi2a(train_info->time_prediction, reply + 3);
            reply_struct.iValue = train_info->arrival_tme;
            Reply (i, (char *)&reply_struct, 30);
            break;
       }
   }
}

int timeToDistance( int sensor, int delta_time, int *train_speed, double speed_mult ) {
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;
    velocity = velocity * speed_mult / 100;
    int distance = delta_time * velocity;
    return distance;
}

int distanceToDelay( int sensor, int distance, int *train_speed, double speed_mult  ) {
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;
    velocity = velocity * speed_mult / 100;
    int delta_time = (int) distance / velocity;
    return delta_time;
}

int shortMoveDistanceToDelay( double distance, int train_num, double short_move_mult) {
    double cubic;
    double square;
    double linear;
    double constant;
    double result = -1;
    distance = distance * short_move_mult / 100;
    if (train_num == 49) {
        cubic = 2.9053E-7 * distance * distance * distance;
        square = 0.000753205 * distance * distance;
        linear = 0.737617 * distance;
        constant = 46.55;
        result = (cubic - square + linear + constant);
    }
    else if (train_num == 56) {
        cubic = 1.7E-7 * distance * distance * distance;
        square = 5.6E-4 * distance * distance;
        linear = (0.68 + 0.02) * distance;
        constant = 61;
        result = (cubic - square + linear + constant);
    }
    else if (train_num == 54) {
        cubic = 2.4E-7 * distance * distance * distance;
        square = 7.2E-4 * distance * distance;
        linear = (0.78 + 0.02) * distance;
        constant = 52;
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

int predictArrivalTime( int sensor, int next_sensor, int init_time, int *train_speed, double speed_mult) {
    int delta_distance = findDistanceBetweenLandmarks( sensor, next_sensor, 10);  //distance is in mm
    float velocity = train_speed[sensor] / 100; // velocity is in mm/tick;
    velocity = velocity * speed_mult / 100;

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
        train_info[0] = -1;
        train_info[1] = -1;
        train_info[4] = -1;
        if (reply_struct.value[0] < 80) {
            train_info[0] = reply_struct.value[0];
        }
        if (reply_struct.value[1] < 80) {
            train_info[1] = reply_struct.value[1];
        }
        train_info[2] = (int) atoi(reply_struct.value + 3);
        // Assert();
        train_info[3] = reply_struct.iValue;
        if (reply_struct.value[2] < 80) {
            train_info[4] = reply_struct.value[2];
        }
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

int setTrainShortMult ( int train_id, int short_mult) {
    char msg[10] = {0};
    char reply[30] = {0};
    int msglen = 10, rpllen = 30;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.iValue = short_mult;
    msg_struct.type = SET_TRAIN_SHORT_MULT;
    msg_struct.value[0] = train_id;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        return 0;
    }
    return -1;
}

int setTrainStopDist ( int train_id, int stop_dist) {
    char msg[10] = {0};
    char reply[30] = {0};
    int msglen = 10, rpllen = 30;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(COMMAND_CENTER_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.iValue = stop_dist;
    msg_struct.type = SET_TRAIN_STOPPING_DIST;
    msg_struct.value[0] = train_id;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        return 0;
    }
    return -1;
}
