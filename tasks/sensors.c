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
        Delay(10);
        putc(COM1, DUMP_ALL_SENSORS);
        for ( i=0; i < 10; i++) {
            c = getc(COM1);
            sensors_bytes[i] = c;
        }
        Reply (courier_tid, (char *)&reply_struct, rpllen);
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
    int requests[64];   // 64 of them
    // each tid can only call one delay at a time
    int i;
    for (i = 0; i < 64; i++) {
        requests[i] = -1;
    }
    int notifier_tid = Create(1, (&SensorNotifier));
    int courier_tid = Create(2, (&SensorCourier));
    msg_struct.iValue = notifier_tid;
    Send (courier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    RegisterAs(SENSOR_SERVER_NAME);
    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case SENSOR_NOTIFIER:
                // reply to notifier I got ur time (don't really care)
                memcpy(current_sensor_state, msg_struct.value, msglen);
                for (i = 0; i < 64; i++) {
                    //if element exists
                    if (requests[i] != -1 && (current_sensor_state[requests[i]/8] & (1 << (7 - (requests[i] % 8))))) {
                        requests[i] = -1;
                        Reply (i, (char *)&reply_struct, msglen);
                    }
                }
                Reply (sender_tid, (char *)&reply_struct, msglen);
                break;
            case SENSORS_DUMP_REQUEST:
                memcpy(reply_struct.value, current_sensor_state, rpllen);
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                // reply what the time is
                break;
            case WAIT_REQUEST:
                // add request to list of suspended tasks
                requests[sender_tid] = msg_struct.iValue;
                break;
            default:
                // wtf
                break;
        }
    }
}

int waitForSensor( int sensor ) {
    char msg[10] = {0};
    char reply[10] = {0};
    int msglen = 10;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(SENSOR_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.iValue = sensor;
    msg_struct.type = WAIT_REQUEST;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return 0;
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
