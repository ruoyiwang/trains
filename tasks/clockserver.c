#include <clockserver.h>
#include <nameserver.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>

void clockServerNotifier() {
    // msg shits
    char msg[8] = {0};
    char reply[8] = {0};
    int receiver_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = NOTIFIER;
    reply_struct.value = reply;
    receiver_tid = WhoIs(CLOCK_SERVER_NAME);
    int data;

    FOREVER {
        data = AwaitEvent( EVENT_CLOCK );
        msg_struct.iValue = data;
        // send evt to data
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
}

void clockServerCountDownAndNotify(delay_element delays[64], message reply_struct, int msglen) {
    int i;
    for (i = 0; i < 64; i++) {
        //if element exists
        if (delays[i].tid != -1) {
            delays[i].delay--;
            if (delays[i].delay <= 0) {
                Reply (delays[i].tid, (char *)&reply_struct, msglen);
                delays[i].tid = -1;
                delays[i].delay = -1;
            }
        }
    }
}

void queueDelay(delay_element delays[64], int tid, int delay) {
    // int i =;

    delays[tid].tid = tid;
    delays[tid].delay = delay;
    // if (i>60){
    //     bwprintf(COM2, "delay queue overflow \n");
    //     Assert();
    // }
}

void clockServer() {
    // Initialize self
    RegisterAs(CLOCK_SERVER_NAME);
    int curTime = 0;
    // Create Notifier and send any initialization data
    Create(0, (&clockServerNotifier));
    // msg shits
    char msg[8] = {0};
    char reply[8] = {0};
    int sender_tid, msglen = 8, rpllen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    // the array that takes in requests
    delay_element delays[64];   // 64 of them
    // each tid can only call one delay at a time
    int i;
    for (i = 0; i < 64; i++) {
        delays[i].tid = -1;
        delays[i].delay = -1;
    }

    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case NOTIFIER:
                // reply to notifier I got ur time (don't really care)
                Reply (sender_tid, (char *)&reply_struct, msglen);
                clockServerCountDownAndNotify(delays, reply_struct, msglen);
                // update time
                curTime++;
                break;
            case TIME_REQUEST:
                reply_struct.iValue = curTime;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                // reply what the time is
                break;
            case DELAY_REQUEST:
                // add request to list of suspended tasks
                if (msg_struct.iValue > 0) {
                    queueDelay(delays, sender_tid, msg_struct.iValue);
                }
                else {
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                }
                break;
            case DELAY_UNTIL_REQUEST:
                // add request to list of suspended tasks
                if (msg_struct.iValue > curTime) {
                    queueDelay(delays, sender_tid, msg_struct.iValue - curTime);
                }
                else {
                    Reply (sender_tid, (char *)&reply_struct, msglen);
                }
                break;
            default:
                // wtf
                break;
        }
    }

}

int Delay( int ticks ) {
    char msg[8] = {0};
    char reply[8] = {0};
    int msglen = 8;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(CLOCK_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.iValue = ticks;
    msg_struct.type = DELAY_REQUEST;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return 0;
    }
    return -1;
}

int Time () {
    char msg[8] = {0};
    char reply[8] = {0};
    int msglen = 8;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(CLOCK_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TIME_REQUEST;
    reply_struct.value = reply;
    reply_struct.iValue = 0;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(reply_struct.value, "FAIL") != 0) {
        // if succeded
        return reply_struct.iValue;
    }
    return -1;
}

int DelayUntil( int ticks ) {
    char msg[8] = {0};
    char reply[8] = {0};
    int msglen = 8;
    static int receiver_tid = -1;
    if (receiver_tid < 0) {
        receiver_tid = WhoIs(CLOCK_SERVER_NAME);
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.iValue = ticks;
    msg_struct.type = DELAY_UNTIL_REQUEST;
    reply_struct.value = reply;

    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        return 0;
    }
    return -1;
}
