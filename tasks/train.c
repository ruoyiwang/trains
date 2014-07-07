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
    int receiver_tid;
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

void TracksTask () {
    RegisterAs(TRACK_TASK);

    // msg shits
    char msg[10] = {0};
    char predict_result[10] = {0};
    char reply[82] = {0};
    char path[80] = {0};
    int sender_tid, msglen = 10, rpllen = 10, path_len = 0;
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
    int cur_sensor, prediction_len = 8, landmark1, landmark2, lookup_limit;
    int stop_command_sensor, stop_command_sensor_dist;

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
            case PREDICT_SENSOR:
                // predict
                cur_sensor = msg[0];
                prediction_len = msg_struct.iValue;
                // type PREDICT + value sensor (in int_8 lol) + ivalue n
                // <== next n switches after sensor, default 8
                predictSensorTrackTask(tracks, switch_status, cur_sensor, prediction_len, predict_result);
                memcpy(reply, predict_result, prediction_len);
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case FIND_DISTANCE_BETWEEN_TWO_LANDMARKS:
                // finds distance between two landmarks
                landmark1 = msg[0];
                landmark2 = msg[1];
                lookup_limit = msg_struct.iValue;
                reply_struct.iValue =
                    findDistanceBetweenLandmarksTrackTask(
                        tracks,
                        switch_status,
                        landmark1,
                        landmark2,
                        lookup_limit
                    );
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            case PATH_FIND:
                // path find
                landmark1 = msg[0];
                landmark2 = msg[1];
                path_len = pathFindTrackTask(
                    tracks,
                    &switch_status,
                    landmark1,              // cur sensor
                    landmark2,              // stopping sensor
                    msg_struct.iValue,      // stopping distance
                    &stop_command_sensor,   // the triggers to be triggered
                    &stop_command_sensor_dist,   // returning distance
                    path                    // this is the route
                );
                reply[0] = stop_command_sensor;
                reply[1] = path_len;
                reply_struct.iValue = stop_command_sensor_dist;
                memcpy(reply+2, path, path_len);
                Reply (sender_tid, (char *)&reply_struct, path_len+2);
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

void predictSensorTrackTask(
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
            else if (cur_node->type == NODE_ENTER || cur_node->type == NODE_EXIT) {
                while ( i++ < prediction_len) {
                    paths[i] = -1;
                }
                return;
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

int predictSensor( int sensor, int prediction_len, char* result ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(TRACK_TASK);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg[0] = sensor;
    msg_struct.iValue = prediction_len;
    msg_struct.type = PREDICT_SENSOR;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    memcpy(result, reply, prediction_len);
    return 0;
}

int findDistanceBetweenLandmarksTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int switch_status,
    int landmark_start,     // 0 based
    int landmark_end,       // 0 based
    int lookup_limit        // amount of predictions wanted
) {
    track_node* cur_node = &tracks[landmark_start];
    track_node* next_node;
    int cur_branch_status;
    int distance = 0;
    while (lookup_limit-- > 0) {
        // calc next_node
        if (cur_node->type == NODE_BRANCH) {
            // look up switch status, then find next_node
            cur_branch_status = getSwitchStatus(&switch_status, cur_node->num);
            if (cur_branch_status == SW_STRAIGHT) {
                next_node = cur_node->edge[DIR_STRAIGHT].dest;
                distance += cur_node->edge[DIR_STRAIGHT].dist;
            }
            else {
                next_node = cur_node->edge[DIR_CURVED].dest;
                distance += cur_node->edge[DIR_CURVED].dist;
            }
        }
        else if (cur_node->type == NODE_ENTER || cur_node->type == NODE_EXIT) {
            return -1;
        }
        else {
            next_node = cur_node->edge[DIR_AHEAD].dest;
            distance += cur_node->edge[DIR_AHEAD].dist;
        }

        if (next_node->num == landmark_end) {
            return distance;
        }

        cur_node = next_node;
    }
    return -1;
}

int findDistanceBetweenLandmarks(
    int landmark1, int landmark2, int lookup_limit
) {
    unsigned char msg[2] = {0};
    char reply[10] = {0};
    int msglen = 2, rpllen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(TRACK_TASK);
    }
    message msg_struct, reply_struct;
    msg_struct.value = (char *)msg;
    msg[0] = landmark1;
    msg[1] = landmark2;
    msg_struct.iValue = lookup_limit;
    msg_struct.type = FIND_DISTANCE_BETWEEN_TWO_LANDMARKS;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        // if succeded
        return reply_struct.iValue;
    }
    return -1;
}

// return succss:path len, fail:-1
int pathFindTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int* switch_status,
    int cur_sensor,         // 0 based
    int stopping_node,      // amount of predictions wanted
    int stopping_dist,
    int* stopping_sensor,   // the triggers to be triggered
    int* stoppong_sensor_dist,   // returning distance
    char* sensor_route
) {
    int i = 0, j = 0, path_landmark_count, pf_result;
    // need to BFS, use another func
    // to find the path through all the nodes
    track_node* path[TRACK_MAX];
    for (i = 0; i < TRACK_MAX; i++) {
        path[i] = -1;
    }

    pf_result = bfsPathFind(tracks, &tracks[cur_sensor], &tracks[stopping_node], path);
    if (pf_result < 0) {
        return pf_result;
    }

    // then set all the switches
    for (i = 0; i < TRACK_MAX; i++) {
        if (path[i] == -1 ){
            path_landmark_count = i;
            break;
        }
        else if (path[i]->type == NODE_BRANCH) {
            if (path[i]->edge[DIR_STRAIGHT].dest == path[i+1] ) {
                setSwitchTrackTask(path[i]->num, SW_STRAIGHT, switch_status);
            }
            else {
                setSwitchTrackTask(path[i]->num, SW_CURVE, switch_status);
            }
        }
    }

    // find the dist to end, can use find distance
    int temp_dist;
    *stopping_sensor = -1;
    *stoppong_sensor_dist = -1;
    for (i = path_landmark_count-2; i >= 0; i--) {
        if (path[i]->type == NODE_SENSOR) {
            temp_dist = findDistanceBetweenLandmarksTrackTask(
                tracks, *switch_status,
                path[i]->num, stopping_node, 80
            );
            // bwprintf(COM2, "\n%d\n", temp_dist);
            // Assert();
            if (temp_dist > stopping_dist) {
                // bwprintf(COM2, "\n%d\n", temp_dist);
                // Assert();
                *stopping_sensor = path[i]->num;
                *stoppong_sensor_dist = temp_dist - stopping_dist;
                break;
            }
        }
    }

    // find the sensor list
    j = 0;
    for (i = 0; i < path_landmark_count; i++) {
        if (path[i]->type == NODE_SENSOR) {
            sensor_route[j++] = path[i]->num;
        }
    }
    return j;
}

int bfsPathFind(        // pretty shit path find lol
    track_node* tracks,
    track_node* begin_node,
    track_node* end_node,
    track_node** path   // the path the train's gonna take
) {
    // init the parents and visisted flags
    int i = 0;
    for (i = 0; i < TRACK_MAX; i++) {
        tracks[i].parent = (track_node*)-1;
        tracks[i].visisted = 0;
    }
    // shitty queue v
    int buffer_size = 80;
    track_node *circ_buff[80];
    int cb_begin = 0, cb_end = 0;

    // init the buff (queue)
    track_node* cur_node = begin_node;
    circ_buff[cb_end++] = cur_node;
    cb_end = cb_end % buffer_size;

    while (cb_begin != cb_end) {
        cur_node = circ_buff[cb_begin++];
        cb_begin = cb_begin % buffer_size;
        if (cur_node->visisted == 1) {
            continue;
        }
        cur_node->visisted = 1;
        if (cur_node == end_node) {
            // track the parents and store in paths
            makePath( cur_node, begin_node, path);
            return 1;
        }
        else if (cur_node->type == NODE_ENTER || cur_node->type == NODE_EXIT) {
            continue;
        }
        else if (cur_node->type == NODE_BRANCH) {
            // straight
            if (cur_node->edge[DIR_STRAIGHT].dest->visisted != 1) {
                cur_node->edge[DIR_STRAIGHT].dest->parent = cur_node;
                circ_buff[cb_end++] = cur_node->edge[DIR_STRAIGHT].dest;
                cb_end = cb_end % buffer_size;
            }
            // curve
            if (cur_node->edge[DIR_CURVED].dest->visisted != 1) {
                cur_node->edge[DIR_CURVED].dest->parent = cur_node;
                circ_buff[cb_end++] = cur_node->edge[DIR_CURVED].dest;
                cb_end = cb_end % buffer_size;
            }
        }
        else {  // either sensor or merge
            // go forward
            if (cur_node->edge[DIR_AHEAD].dest->visisted != 1) {
                cur_node->edge[DIR_AHEAD].dest->parent = cur_node;
                circ_buff[cb_end++] = cur_node->edge[DIR_AHEAD].dest;
                cb_end = cb_end % buffer_size;
            }
            // // try reverse too, especially for merge
            // cur_node->reverse->parent = cur_node;
            // circ_buff[cb_end++] = cur_node->reverse;
            // cb_end = cb_end % buffer_size;
        }
    }
    return -1;
}

void makePath(track_node* node, track_node* init_node, track_node** path) {
    // find how long the path is
    track_node* cur_node = node;
    int path_length = 1;
    while ( (cur_node = cur_node->parent) != -1 ){
        path_length++;
    }
    // add shits to path backwards
    int i = 0;
    cur_node = node;
    for (i = path_length-1; i >=0; i--) {
        path[i] = cur_node;
        cur_node = cur_node->parent;
    }
}

// expecte sensor_route to be as long as 80 char
int pathFind(
    int cur_sensor,             // current node
    int dest_node,              // where it wants to go
    int stopping_dist,          // stoping distance
    int* stopping_sensor,       // returning node
    int* stoppong_sensor_dist,  // returning distance
    char* sensor_route          // the sensors the train's gonna pass
) {
    int sensor_path_len = 0;

    // msg shits
    char msg[10] = {0};
    char reply[83] = {0};
    int msglen = 10;
    int rpllen = 83;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(TRACK_TASK);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg[0] = cur_sensor;
    msg[1] = dest_node;
    msg_struct.iValue = stopping_dist;
    msg_struct.type = PATH_FIND;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);


    *stoppong_sensor_dist = reply_struct.iValue;
    *stopping_sensor = reply[0];
    sensor_path_len = reply[1];
    // memcpy(sensor_route, reply+2, sensor_path_len);
    if (reply_struct.iValue < 0) {
        return -1;
    }
    return 1;

}

void setSwitchTrackTask(int switch_num, char switch_dir, unsigned int* switch_status) {
    char commandstr[3];
    commandstr[0] = switch_dir;
    setSwitchStatus(switch_status, switch_num, switch_dir);
    commandstr[1] = switch_num;
    commandstr[2] = 32;
    putstr_len(COM1, commandstr, 3);
}
