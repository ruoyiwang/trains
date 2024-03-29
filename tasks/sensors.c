#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <track.h>
#include <train.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <sensors.h>

void SensorNotifier() {
    int i;
    char c;
    char sensors_bytes[10];
    int courier_tid;

    char msg[10] = {0};
    // int receiver_tid;
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = SENSOR_NOTIFIER;
    reply_struct.value = sensors_bytes;

    Receive( &courier_tid, (char*)&msg_struct, msglen );
    Reply (courier_tid, (char *)&reply_struct, rpllen);

    FOREVER {
        Receive( &courier_tid, (char*)&msg_struct, msglen );
        putc(COM1, DUMP_ALL_SENSORS);
        for ( i=0; i < 10; i++) {
            c = getc(COM1);
            sensors_bytes[i] = c;
        }
        Reply (courier_tid, (char *)&reply_struct, rpllen);
    }
}

void SensorNotifierNoCourier() {
    int i;
    char c;
    char sensors_bytes[10];

    int receiver_tid = WhoIs(SENSOR_SERVER_NAME);

    char rpl[10] = {0};
    // int receiver_tid;
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    reply_struct.value = rpl;
    msg_struct.type = SENSOR_NOTIFIER;
    msg_struct.value = sensors_bytes;

    FOREVER {
        memset(&msg_struct, 0, sizeof(message));
        msg_struct.type = SENSOR_NOTIFIER;
        msg_struct.value = sensors_bytes;
        Delay(5);
        putc(COM1, DUMP_ALL_SENSORS);
        for ( i=0; i < 10; i++) {
            // Delay(2);
            c = getc(COM1);
            sensors_bytes[i] = c;
        }
        // sensors_bytes[0] = Time();
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
}

void SensorCourier() {
    int notifier_tid, server_tid;

    char msg[10] = {0};
    char rpl[10] = {0};
    // int receiver_tid;
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.type = SENSOR_NOTIFIER;
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

void SensorServer() {
    // Create Notifier and send any initialization data
    // msg shits
    char msg[10] = {0};
    char reply[10] = {0};
    int sender_tid, msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    char current_sensor_state[10] = {0};
    // the array that takes in requests
    int requests[64][6];   // 64 of them
    // each tid can only call one delay at a time
    int request_met = false;
    int i, j;
    for (i = 0; i < 64; i++) {
        requests[i][0] = -1;
        requests[i][1] = -1;
        requests[i][2] = -1;
        requests[i][3] = -1;
        requests[i][4] = -1;
        requests[i][5] = -1;
    }
    // int notifier_tid = Create(1, (&SensorNotifier));
    // int courier_tid = Create(2, (&SensorCourier));
    // msg_struct.iValue = notifier_tid;
    // Send (courier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    RegisterAs(SENSOR_SERVER_NAME);
    Create(1, (&SensorNotifierNoCourier));

    int sensorNum;
    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case SENSOR_NOTIFIER:
                // reply to notifier I got ur time (don't really care)
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                memcpy(current_sensor_state, msg_struct.value, 10);
                for (i = 0; i < 64; i++) {
                    if (requests[i][1] >= 0 && requests[i][1] < 80 && (current_sensor_state[requests[i][1]/8] & (1 << (7 - (requests[i][1] % 8))))) {
                        // msg_struct.value[requests[i][1]/8] = msg_struct.value[requests[i][1]/8] & ~(1 << (7 - (requests[i][1] % 8)));
                        // DebugPutStr("sdsd", "DEBUG: unblocked sensor: ",requests[i][1], " for task: ", i);
                        reply_struct.iValue = requests[i][1];
                        requests[i][0] = -1;
                        requests[i][1] = -1;
                        requests[i][2] = -1;
                        requests[i][3] = -1;
                        requests[i][4] = -1;
                        requests[i][5] = -1;
                        Reply (i, (char *)&reply_struct, rpllen);
                        // break;
                    }
                }
                for (i = 0; i < 64; i++) {
                    if (requests[i][1] >= 0 && requests[i][1] < 80 && (msg_struct.value[requests[i][1]/8] & (1 << (7 - (requests[i][1] % 8))))) {
                        msg_struct.value[requests[i][1]/8] = msg_struct.value[requests[i][1]/8] & ~(1 << (7 - (requests[i][1] % 8)));
                        // break;
                    }
                }
                for (i = 0; i < 64; i++) {
                    //if element exists
                    // if ( requests[i][0] != -1) {
                    //     cur_time = Time();
                    //     if (requests[i][0] > cur_time) { // sensor timeout occured
                    //         reply_struct.iValue = -1;
                    //         requests[i][0] = -1;
                    //         requests[i][1] = -1;
                    //         requests[i][2] = -1;
                    //         requests[i][3] = -1;
                    //         requests[i][4] = -1;
                    //         requests[i][5] = -1;
                    //         Reply (i, (char *)&reply_struct, rpllen);
                    //         continue;
                    //     }
                    // }
                    for (j=1 ; j< 6; j ++){
                        if (requests[i][j] >= 0 && requests[i][1] < 80 && (msg_struct.value[requests[i][j]/8] & (1 << (7 - (requests[i][j] % 8))))) {
                            msg_struct.value[requests[i][j]/8] = msg_struct.value[requests[i][j]/8] & ~(1 << (7 - (requests[i][1] % 8)));
                            reply_struct.iValue = requests[i][j];
                            requests[i][0] = -1;
                            requests[i][1] = -1;
                            requests[i][2] = -1;
                            requests[i][3] = -1;
                            requests[i][4] = -1;
                            requests[i][5] = -1;
                            Reply (i, (char *)&reply_struct, rpllen);
                            break;
                        }
                    }
                }
                sensorNum = -1;
                for (i = 0; i<10 ; i ++){
                    if (msg_struct.value[i] == 0) {
                        continue;
                    }
                    for (j = 0; j< 8 ; j++) {
                        if ( msg_struct.value[i] & ( 1 << j ) ){
                            if ( (i % 2) ) {
                                sensorNum = (i / 2)*16 + 15 - j;
                            }
                            else {
                                sensorNum =(i / 2)*16 + 7 - j;
                            }
                        }
                    }

                }
                if (sensorNum != -1){
                    for (i = 0; i < 64; i++) {
                        for (j=1 ; j< 6; j ++){
                            if (requests[i][j] == ANY_SENSOR_REQUEST) {
                                DebugPutStr("sdsd", "DEBUG: unblocked sensor: ",sensorNum , " for task: ", i);
                                reply_struct.iValue = sensorNum;
                                requests[i][0] = -1;
                                requests[i][1] = -1;
                                requests[i][2] = -1;
                                requests[i][3] = -1;
                                requests[i][4] = -1;
                                requests[i][5] = -1;
                                Reply (i, (char *)&reply_struct, rpllen);
                                break;
                            }
                        }
                    }
                }
                break;
            case SENSORS_DUMP_REQUEST:
                memcpy(reply_struct.value, current_sensor_state, 10);
                Reply (sender_tid, (char *)&reply_struct, 10);
                // reply what the time is
                break;
            case SENSORS_CREATE_NOTIFIER:
                Create(1, (&SensorNotifierNoCourier));
                Reply (sender_tid, (char *)&reply_struct, 10);
                // reply what the time is
                break;
            case WAIT_REQUEST:
                // add request to list of suspended tasks
                requests[sender_tid][0] = msg_struct.iValue;
                for (i= 0; i < (int)msg_struct.value[0]; i ++) {
                    requests[sender_tid][1+i] = msg_struct.value[i+1];
                }
                // memcpy(requests[sender_tid]+1, msg_struct.value, msg_struct.value[0]);
                break;
            case CHANGE_WAIT_REQUEST:
                // add request to list of suspended tasks
                for (i= 0; i < 6; i ++) {
                    requests[msg_struct.iValue][i] = -1;
                }
                requests[msg_struct.iValue][0] = 1000000;
                for (i= 0; i < (int)msg_struct.value[0]; i ++) {
                    requests[msg_struct.iValue][1+i] = msg_struct.value[i+1];
                }
                // memcpy(requests[sender_tid]+1, msg_struct.value, msg_struct.value[0]);
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
            default:
                // wtf
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll SENSORSSERVER %d", msg_struct.type);
                reply_struct.type = FAIL_TYPE;
                Reply (sender_tid, (char *)&reply_struct, 10);
                break;
        }
    }
}

int waitForSensors( char *sensors, int len, int timeOut ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(SENSOR_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = WAIT_REQUEST;
    reply_struct.value = reply;

    msg_struct.iValue = timeOut;
    msg[0] = (char)len;

    // int i;
    // for (i = 0; i < len; i++) {
    //     msg[1+i] = sensors[i];
    // }
    memcpy(msg+1, sensors, len);
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return reply_struct.iValue;
    }
    return -1;
}

int changeWaitForSensors(int tid, char *sensors, int len ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(SENSOR_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = CHANGE_WAIT_REQUEST;
    reply_struct.value = reply;

    msg_struct.iValue = tid;
    msg[0] = (char)len;

    // int i;
    // for (i = 0; i < len; i++) {
    //     msg[1+i] = sensors[i];
    // }
    memcpy(msg+1, sensors, len);
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return reply_struct.iValue;
    }
    return -1;
}

int getLatestSensors ( char current_sensor_state[10]) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(SENSOR_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = SENSORS_DUMP_REQUEST;
    reply_struct.value = reply;
    reply_struct.iValue = 0;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        // if succeded
        memcpy(current_sensor_state, reply_struct.value, 10);
        return 0;
    }
    return -1;
}

int sensorToInt (char module, int num) {
    return (module - 'A') * 16 + num - 1;
}

int getSensorComplement (int num) {
    if (num % 2 == 1){
        return num - 1;
    }
    else{
        return num + 1;
    }
}
