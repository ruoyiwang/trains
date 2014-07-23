#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <track.h>
#include <posintlist.h>

#include <track_data.h>
#include <track_node.h>

void TracksTask () {
    RegisterAs(TRACK_TASK);

    // pathfind shits
    volatile path_find_requirements pfr;

    char* msg;
    msg = &pfr;
    // so msg and pft share the same address space
    // holy shit this is ugly =__=

    // more msg shits
    char predict_result[10] = {0};
    char reply[182] = {0};
    char path[80] = {0};
    int sender_tid, rpllen = 10, path_len = 0;
    int msglen = sizeof(path_find_requirements);

    message msg_struct, reply_struct;
    msg_struct.value = (char*)&pfr;
    reply_struct.value = reply;
    char commandstr[10] = {0};
    int i = 0;

    // switch status, bitwise 32-0,
    // think we only 22-1?
    // 0: curve
    // 1: stright
    unsigned int switch_status = 0;

    // init the tracks
    track_node tracks[TRACK_MAX];
    // and we are using track b
    // init_tracka(tracks);
    init_trackb(tracks);
    int cur_sensor, prediction_len = 8, landmark1, landmark2, lookup_limit;
    int stop_command_sensor, stop_command_sensor_dist;

    // move data from path find

    FOREVER {
        rpllen = 10;
        reply_struct.value = reply;
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
            case INIT_TRACK:
                if (msg_struct.value[0] == 'a') {
                    init_tracka(tracks);
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                else {
                    init_trackb(tracks);
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
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
                    &path                    // this is the route
                );
                reply[0] = stop_command_sensor;
                reply[1] = path_len;
                reply_struct.iValue = stop_command_sensor_dist;
                // memcpy(reply+2, path, path_len);
                Reply (sender_tid, (char *)&reply_struct, 2);
                break;
            case PATH_FIND_DIJKSTRA:
                // path find
                landmark1 = msg[0];
                landmark2 = msg[1];
                move_data md = pathFindDijkstraTrackTask(
                    tracks,     // the initialized array of tracks
                    &switch_status,
                    pfr.src,         // 0 based
                    pfr.src_node_offfset,
                    pfr.dest,
                    pfr.stopping_dist,      // stopping distance
                    pfr.blocked_nodes,
                    pfr.blocked_nodes_len
                );
                // debug line
                // for (i = 0; i < md.list_len; i++) {
                //     if (md.type == SAFE_REVERSE) {
                //         bwprintf(COM2, "SAFE REVERSE    ");
                //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[0].num);
                //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[1].num);
                //         break;
                //     }
                //     else if (md.type == UNSAFE_REVERSE) {
                //         bwprintf(COM2, "UNSAFE REVERSE  ");
                //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[0].num);
                //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[1].num);
                //         break;
                //     }
                //     bwprintf(COM2, "\n");
                //     if (md.node_list[i].type == NODE_SENSOR) {
                //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[i].num);
                //     }
                //     else if (md.node_list[i].type == NODE_BRANCH) {
                //         bwprintf(COM2, "%d|Branch: %d   ", md.list_len, md.node_list[i].num);
                //     }
                //     else if (md.node_list[i].type == NODE_MERGE) {
                //         bwprintf(COM2, "%d|Merge: %d    ", md.list_len, md.node_list[i].num);
                //     }
                //     bwprintf(COM2, "\n");
                // }
                // end of debugline
                reply_struct.iValue = 1;
                // memcpy(reply_struct.value, md, sizeof(move_data));
                reply_struct.value = (char*)&md;
                Reply (sender_tid, (char *)&reply_struct, sizeof(move_data));
                break;
            default:
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll TRACKSERVER %d", msg_struct.type);
                Assert();
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

// return succss:path len, fail:-1
struct move_data_t pathFindDijkstraTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int* switch_status,
    int cur_sensor,         // 0 based
    int src_node_offfset,
    int stopping_node,
    int stopping_dist,
    int blocked_nodes[],         // the landmarks the train cannot use
    int blocked_nodes_len
) {
    int i = 0, unsafe_reverses_list_size = 10;
    int unsafe_reverses[10];
    unsafe_reverses[i++] = 3;     // A4
    unsafe_reverses[i++] = 30;    // B15
    unsafe_reverses[i++] = 37;     // C6
    unsafe_reverses[i++] = 38;     // C7
    unsafe_reverses[i++] = 40;     // C9
    unsafe_reverses[i++] = 42;     // C11
    unsafe_reverses[i++] = 44;     // C13
    unsafe_reverses[i++] = 67;     // E4
    unsafe_reverses[i++] = 68;     // E5
    unsafe_reverses[i++] = 74;     // E11

    int distances[TRACK_MAX] = {0};
    int route[TRACK_MAX] = {0};

    int previous[TRACK_MAX];
    int Q[TRACK_MAX];
    posintlistInit(Q, TRACK_MAX);
    distances[cur_sensor] = 0;
    for (i = 0; i < TRACK_MAX; i++) {
        if (i != cur_sensor) {
            distances[i] = 1000000000;    // infinity
            previous[i] = -1;           // undefined
        }
        Q[i] = i;   // adding the item to Q
    }

    int u, alt, min_dist, min_u, v, found_path = 0;
    // main loop
    while (!posintlistIsEmpty(Q, TRACK_MAX)) {
        // u := vertex in Q with min dist[u]
        min_dist = 1000000000;
        for (i = 0; i < TRACK_MAX; i++) {
            u = Q[i];
            if (u != -1) {
                if (distances[u] < min_dist) {
                    min_dist = distances[u];
                    min_u = u;
                }
            }
        }
        u = min_u;

        // remove u from Q
        posintlistErase(u, Q, TRACK_MAX);

        if (posintlistIsInList(u, blocked_nodes, blocked_nodes_len) &&
            u == stopping_node) {
            break;
        }
        else if (posintlistIsInList(u, blocked_nodes, blocked_nodes_len)) {
            // we avoid the items that are not available
            continue;
        }
        else if (u == stopping_node) {
            found_path = 1;
            break;
        }

        // for each neighbor v of u that's not in Q
        // find shortest path
        if (tracks[u].type == NODE_ENTER || tracks[u].type == NODE_EXIT) {
            continue;
        }
        else if (tracks[u].type == NODE_BRANCH) {
            // here we can do straight, curve, reverse
            v = tracks[u].edge[DIR_STRAIGHT].dest->index;
            if (posintlistIsInList(v, Q, TRACK_MAX)) {
                alt = distances[u] + tracks[u].edge[DIR_STRAIGHT].dist;
                if (alt < distances[v]) {
                    distances[v] = alt;
                    previous[v] = u;
                }
            }
            v = tracks[u].edge[DIR_CURVED].dest->index;
            if (posintlistIsInList(v, Q, TRACK_MAX)) {
                alt = distances[u] + tracks[u].edge[DIR_CURVED].dist;
                if (alt < distances[v]) {
                    distances[v] = alt;
                    previous[v] = u;
                }
            }
            v = tracks[u].reverse->index;
            if (posintlistIsInList(v, Q, TRACK_MAX)) {
                alt = distances[u] + REVERSING_WEIGHT;
                if (alt < distances[v]) {
                    distances[v] = alt;
                    previous[v] = u;
                }
            }
        }
        else {
            // this is either sensor or merge
            // here we can only do ahead or reverse
            v = tracks[u].edge[DIR_AHEAD].dest->index;
            if (posintlistIsInList(v, Q, TRACK_MAX)) {
                alt = distances[u] + tracks[u].edge[DIR_AHEAD].dist;
                if (alt < distances[v]) {
                    distances[v] = alt;
                    previous[v] = u;
                }
            }
            v = tracks[u].reverse->index;
            if (posintlistIsInList(v, Q, TRACK_MAX)) {
                alt = distances[u] + REVERSING_WEIGHT;
                if (alt < distances[v]) {
                    distances[v] = alt;
                    previous[v] = u;
                }
            }
        }
    }

    move_data md;
    md.total_distance = 0;

    if (!found_path) {
        md.type = PATH_NOT_FOUND;
        return md;
    }


    // make Dijkstra Path
    // we should able to back trace (stopping node)
    // first find pathlength
    // TODO: make this shit better
    int path_length = 0;
    int temp_node = stopping_node;
    while (temp_node != cur_sensor) {
        path_length++;
        temp_node = previous[temp_node];
    }

    temp_node = stopping_node;
    for (i = path_length; temp_node != cur_sensor; i--) {
        route[i] = temp_node;
        temp_node = previous[temp_node];
        // bwprintf(COM2, "\n%d nodes: %d    \n", path_length, route[i]);
    }
    route[0] = cur_sensor;

    // including the ending node
    path_length = path_length+1;

    // this is the first move the train needs to perform
    // go through the list of nodes, identify shits and return them.
    // first identify if the first move is a reverse
    track_node first_node = tracks[(int)route[0]];
    track_node second_node = tracks[(int)route[1]];
    // int cur_branch_status;
    if (first_node.reverse->index == second_node.index) {
        // bwprintf(COM2, "\nswag %d|", first_node.reverse->index);
        // bwprintf(COM2, "%d\n", second_node.index);
        // need to reverse first
        // check if not safe

        md.node_list[0].type = first_node.type;
        md.node_list[0].id = first_node.index;
        md.node_list[0].num = first_node.num;
        md.node_list[1].type = second_node.type;
        md.node_list[1].id = second_node.index;
        md.node_list[1].num = second_node.num;
        md.list_len = 2;
        md.total_distance = 2;

        if (posintlistIsInList(first_node.index, unsafe_reverses, unsafe_reverses_list_size)) {
            // if unsafe, command center needs to handle it by shifting it train's len
            md.type = UNSAFE_REVERSE;
        }
        else {
            // if safe, cmd center just send the reverse command
            md.type = SAFE_REVERSE;
        }
        return md;
    }

    // second, construct the mode up to the reverse
    int j = 0, cur_node_num, next_node_num;
    md.total_distance = 0 - src_node_offfset;
    md.type = SHORT_MOVE;
    for (i = 0; i < path_length; i++, j++) {
        cur_node_num = route[i];
        next_node_num = route[i+1];
        md.node_list[j].type = tracks[cur_node_num].type;
        md.node_list[j].id = tracks[cur_node_num].index;
        md.node_list[j].num = tracks[cur_node_num].num;
        md.list_len = j+1;
        if (md.total_distance > stopping_dist * 2.5) {
            md.type = LONG_MOVE;
        }

        // if stopping node, return
        if (cur_node_num == stopping_node) {
            break;
        }
        // if sensor reverse, return
        else if (tracks[cur_node_num].type == NODE_SENSOR &&
            tracks[cur_node_num].reverse->index == tracks[next_node_num].index) {
            break;
        }
        // else if merge reverse, goto next sensor
        else if (tracks[cur_node_num].type == NODE_MERGE &&
            tracks[cur_node_num].reverse->index == tracks[next_node_num].index) {
            // just greedy find the closest sensor
            cur_node_num = tracks[cur_node_num].edge[DIR_AHEAD].dest->index;
            while (true) {   // can only be max chain 4 though
                j++;
                md.list_len = j+1;
                md.node_list[j].type = tracks[cur_node_num].type;
                md.node_list[j].id = tracks[cur_node_num].index;
                md.node_list[j].num = tracks[cur_node_num].num;
                if (tracks[cur_node_num].type == NODE_MERGE) {
                    // the one to look at next
                    cur_node_num = tracks[cur_node_num].edge[DIR_AHEAD].dest->index;
                    md.total_distance += tracks[cur_node_num].edge[DIR_AHEAD].dist;
                }
                else if (tracks[cur_node_num].type == NODE_SENSOR) {
                    // break_outer_loop = 1;
                    break;
                }
                else if (tracks[cur_node_num].type == NODE_BRANCH) {                    // if straight is a sensor, then go straight
                    if (tracks[cur_node_num].edge[DIR_STRAIGHT].dest->type == NODE_SENSOR) {
                        md.node_list[j].branch_state = SW_STRAIGHT;
                        cur_node_num = tracks[cur_node_num].edge[DIR_STRAIGHT].dest->index;
                        md.total_distance += tracks[cur_node_num].edge[DIR_STRAIGHT].dist;
                    }
                    else {
                        md.node_list[j].branch_state = SW_CURVE;
                        cur_node_num = tracks[cur_node_num].edge[DIR_CURVED].dest->index;
                        md.total_distance += tracks[cur_node_num].edge[DIR_CURVED].dist;
                    }
                }
            }
            break;
        }

        // else if branch + reverse, set br dir, goto next closest sensor, return
        // actually this is not logically possible =____= just tehcnically possible

        // else if normal branch, find branch dir and set it
        else if (tracks[cur_node_num].type == NODE_BRANCH) {
            // find the correct branch
            if (tracks[cur_node_num].edge[DIR_STRAIGHT].dest->index == tracks[next_node_num].index) {
                md.node_list[j].branch_state = SW_STRAIGHT;
                md.total_distance += tracks[cur_node_num].edge[DIR_STRAIGHT].dist;
            }
            else {
                md.node_list[j].branch_state = SW_CURVE;
                md.total_distance += tracks[cur_node_num].edge[DIR_CURVED].dist;
            }
        }
        else {
            md.total_distance += tracks[cur_node_num].edge[DIR_AHEAD].dist;
        }

    }


    if (md.type != LONG_MOVE) {
        // just return on short move or reverse
        return md;
    }

    // have to tell the caller where to stop
    int cur_dist = src_node_offfset;
    // if the ending node is in unsafe_reverses, we need to make the train go train leng more
    if (posintlistIsInList(md.node_list[md.list_len-1].id, unsafe_reverses, unsafe_reverses_list_size)) {
        cur_dist -= TRAIN_LENGTH;
    }
    for (i = md.list_len-2; i > 0; i--) {   // start with the second last one
        if (md.node_list[i].type == NODE_BRANCH) {
            if (md.node_list[i].branch_state == SW_STRAIGHT) {
                cur_dist += tracks[md.node_list[i].id].edge[DIR_STRAIGHT].dist;
            }
            else {
                cur_dist += tracks[md.node_list[i].id].edge[DIR_CURVED].dist;
            }
        }
        else {
            // straight or merge
            cur_dist += tracks[md.node_list[i].id].edge[DIR_AHEAD].dist;
        }

        // found
        if (cur_dist > stopping_dist && md.node_list[i].type == NODE_SENSOR) {
            md.stopping_sensor = md.node_list[i].num;
            md.stopping_dist = cur_dist - stopping_dist;
            break;
        }
    }

    return md;
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
    // if (reply_struct.iValue < 0) {
    //     return -1;
    // }
    return 1;

}

// expecte sensor_route to be as long as 80 char
int pathFindDijkstra(
    move_data* md,
    int cur_sensor,             // current node
    int src_node_offfset,
    int dest_node,              // where it wants to go
    int stopping_dist,          // stoping distance
    int blocked_nodes[TRACK_MAX],         // the landmarks the train cannot use
    int blocked_nodes_len
) {
    int i = 0;
    // int sensor_path_len = 0;

    // char reply[182] = {0};
    int msglen = sizeof(path_find_requirements);
    int rpllen = sizeof(move_data);

    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(TRACK_TASK);
    }
    message msg_struct, reply_struct;

    path_find_requirements pfr;
    pfr.src = cur_sensor;
    pfr.dest = dest_node;
    pfr.stopping_dist = stopping_dist;
    pfr.src_node_offfset = src_node_offfset;
    pfr.blocked_nodes_len = blocked_nodes_len;
    // apparently I can't assign an int array to an int array, so deep cpy it is
    for (i = 0; i < blocked_nodes_len; i++) {
        pfr.blocked_nodes[i] = blocked_nodes[i];
    }
    for (i = blocked_nodes_len; i < TRACK_MAX; i++) {
        pfr.blocked_nodes[i] = -1;
    }

    msg_struct.value = (char*)&pfr;

    msg_struct.iValue = stopping_dist;
    msg_struct.type = PATH_FIND_DIJKSTRA;
    reply_struct.value = (char*)md;

    // // bwprintf(COM2, "\nHITTTTTTTTTTTTTTTTTTTTT\n");
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    // *stoppong_sensor_dist = reply_struct.iValue;
    // *stopping_sensor = reply_struct.value[0];
    // sensor_path_len = reply_struct.value[1];
    // memcpy(sensor_route, reply_struct.value+2, sensor_path_len);
    // debug line
    // for (i = 0; i < md.list_len; i++) {
    //     if (md.type == SAFE_REVERSE) {
    //         bwprintf(COM2, "SAFE REVERSE    \n");
    //         bwprintf(COM2, "%d|%d           \n", md.list_len, md.node_list[0].num);
    //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[1].num);
    //         break;
    //     }
    //     else if (md.type == UNSAFE_REVERSE) {
    //         bwprintf(COM2, "UNSAFE REVERSE  \n");
    //         bwprintf(COM2, "%d|%d           \n", md.list_len, md.node_list[0].num);
    //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[1].num);
    //         break;
    //     }
    //     bwprintf(COM2, "\n");
    //     bwprintf(COM2, "%d\n", md.type);
    //     if (md.node_list[i].type == NODE_SENSOR) {
    //         bwprintf(COM2, "%d|%d           ", md.list_len, md.node_list[i].num);
    //     }
    //     else if (md.node_list[i].type == NODE_BRANCH) {
    //         bwprintf(COM2, "%d|Branch: %d   ", md.list_len, md.node_list[i].num);
    //     }
    //     else if (md.node_list[i].type == NODE_MERGE) {
    //         bwprintf(COM2, "%d|Merge: %d    ", md.list_len, md.node_list[i].num);
    //     }
    //     bwprintf(COM2, "\n");
    // }
    // end of debugline
    if (md->type == PATH_NOT_FOUND) {
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

void initTrack(char track) {
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
    msg_struct.type = INIT_TRACK;
    reply_struct.value = reply;

    if (track == 'a') {
        msg[0] = 'a';
    }
    else {
        // init track b
        msg[0] = 'b';
    }
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
}
