#include <Tasks.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <nameserver.h>
#include <clockserver.h>
#include <kernel.h>

void FirstUserTask () {

    // fire off the
    initTimers();

    volatile unsigned int * timer_4_low;
    timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );
    // bwprintf(COM2, "%d\n", *timer_4_low );

    char msg[20] = "I am a message";
    char reply[20];
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = 12;
    reply_struct.value = reply;

    bwsetfifo( COM2, OFF);

    // create nameserver
    int ns_tid = Create(1, CODE_OFFSET + (&NameServer));
    // check if the created nameserver tid == NAMESERVER_TID
    if (ns_tid != 1) {
        bwprintf(COM2, "WTF is happening\n\n");
        Exit();
    }
    // Create(2, CODE_OFFSET + (&spawnedTask));

    // // making name server test task
    // Create(3, CODE_OFFSET + (&nameServerTest1));
    // Create(3, CODE_OFFSET + (&nameServerTest2));

    // int tid = Create(2, CODE_OFFSET + (&spawnedTask));
    // bwprintf(COM2, "Sending message to %d: %s\n",tid, msg);
    // Send (tid, (char *)&msg_struct, 14, (char *)&reply_struct, 10);
    // bwprintf(COM2, "Got reply from %d with type %d: %s\n",tid,reply_struct.type, reply);

    // system idle task at lowest pri
    Create(15, CODE_OFFSET + (&SystemIdleTask));

    // playtRPS();
    perfTest();

    ClockServerTest();

    Exit();
}

void spawnedTask () {
    bwprintf(COM2, "SP\n");

    RegisterAs("ReceiveTask");
    int tid1, tid2;
    char msg1[20], msg2[20];
    char reply1[20] = "I am a reply1",reply2[20] = "I am a reply2";
    message msg_struct1,msg_struct2, reply_struct1,reply_struct2;
    msg_struct1.value = msg1;
    reply_struct1.value = reply1;
    reply_struct1.type = 10;

    msg_struct2.value = msg2;
    reply_struct2.value = reply2;
    reply_struct2.type = 10;

    Receive (&tid1, (char *)&msg_struct1, 14); //4
    bwprintf(COM2, "Received message from %d with type %d: %s\n",tid1, msg_struct1.type, msg1);
    Receive (&tid2, (char *)&msg_struct2, 14);//3
    bwprintf(COM2, "Received message from %d with type %d: %s\n",tid2, msg_struct2.type, msg2);

    bwprintf(COM2, "Reply message to %d: I am a reply1\n",tid1);
    Reply (tid1, (char *)&reply_struct1, 14);
    bwprintf(COM2, "Reply message to %d: I am a reply2\n",tid2);
    Reply (tid2, (char *)&reply_struct2, 14);
    Exit();
}

void nameServerTest1 () {
    RegisterAs("nameServerTest1");
    int randomtid = WhoIs("ReceiveTask");

    char msg[20] = "I am a message1";
    char reply[20];
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = 12;
    reply_struct.value = reply;

    bwprintf(COM2, "Sending message to %d: %s\n",randomtid, msg);
    Send (randomtid, (char *)&msg_struct, 14, (char *)&reply_struct, 10);//1
    bwprintf(COM2, "Got reply from %d with type %d: %s\n",randomtid,reply_struct.type, reply);

    Exit();
}

void nameServerTest2 () {
    RegisterAs("nameServerTest2");
    int randomtid = WhoIs("ReceiveTask");

    char msg[20] = "I am a message2";
    char reply[20];
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = 12;
    reply_struct.value = reply;

    bwprintf(COM2, "Sending message to %d: %s\n",randomtid, msg);
    Send (randomtid, (char *)&msg_struct, 14, (char *)&reply_struct, 10);
    bwprintf(COM2, "Got reply from %d with type %d: %s\n",randomtid,reply_struct.type, reply);

    Exit();
}

void perfTest() {
    Create(4, CODE_OFFSET + (&testSend));
    Create(4, CODE_OFFSET + (&testReceive));
}

void testSend() {
    RegisterAs("testSend");
    int receiver_tid = -1;
    while (receiver_tid < 0) {
        receiver_tid = WhoIs("testReceive");
    }

    int msglen = 64;
    char msg[64] = {0};
    char reply[64] = {0};
    int i = 0;
    for ( i = 0; i < msglen - 1; i++) {
        msg[i] = 'a';
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    bwprintf(COM2, "initiating Send\n");

    // start timing
    volatile unsigned int * timer_4_low;
    volatile unsigned int * timer_4_hi;
    timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );
    timer_4_hi = (unsigned int *) ( TIMER4_ENABLE_HI );
    int hi = (*timer_4_hi) & TIMER4_HI_MASK ;
    bwprintf(COM2, "hi: %u\n", hi);
    bwprintf(COM2, "lo: %u\n", (*timer_4_low));

    for (i = 0; i < 1000; i++) {
        Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
    }
    // end timing
    hi = (*timer_4_hi) & TIMER4_HI_MASK ;
    bwprintf(COM2, "hi: %u\n", hi);
    bwprintf(COM2, "lo: %u\n", (*timer_4_low));

    Exit();
}

void testReceive() {
    RegisterAs("testReceive");
    int sender_id = -1;
    while (sender_id < 0) {
        sender_id = WhoIs("testSend");
    }
    int msglen = 64;
    int sender_tid;
    char msg[64] = {0};
    char reply[64] = {0};

    int i = 0;
    for ( i = 0; i < msglen - 1; i++) {
        reply[i] = 'a';
    }
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    bwprintf(COM2, "initiating Receive\n");

    while(1) {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        Reply (sender_tid, (char *)&reply_struct, msglen);
    }

    Exit();
}

void ClockServerTest() {
    // make the server
    Create(1, CODE_OFFSET + (&clockServer));

    // make the test task
    Create(3, CODE_OFFSET + (&ClockServerTestTask));
    Create(4, CODE_OFFSET + (&ClockServerTestTask));
    Create(5, CODE_OFFSET + (&ClockServerTestTask));
    Create(6, CODE_OFFSET + (&ClockServerTestTask));

    // msg boilerplate
    int sender_tid1, sender_tid2, sender_tid3, sender_tid4;
    char msg[8] = {0};
    char reply[8] = {0};
    int msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    Receive( &sender_tid1, (char*)&msg_struct, msglen );
    Receive( &sender_tid2, (char*)&msg_struct, msglen );
    Receive( &sender_tid3, (char*)&msg_struct, msglen );
    Receive( &sender_tid4, (char*)&msg_struct, msglen );

    reply[0] = 10;
    reply[1] = 20;
    Reply ( sender_tid1, (char *)&reply_struct, msglen );
    reply[0] = 23;
    reply[1] = 9;
    Reply ( sender_tid2, (char *)&reply_struct, msglen );
    reply[0] = 33;
    reply[1] = 6;
    Reply ( sender_tid3, (char *)&reply_struct, msglen );
    reply[0] = 71;
    reply[1] = 3;
    Reply ( sender_tid4, (char *)&reply_struct, msglen );

}

void SystemIdleTask() {
    char c;
    FOREVER{
        c = bwgetc(COM2);
        if (c == 'q') {
            Exit();
        }
    }
}

void ClockServerTestTask() {
    int delay_times, number_of_delays, parent_tid, tid;

    // get my parent tid;
    parent_tid = MyParentTid();
    tid = MyTid();

    char msg[8] = {0};
    char reply[8] = {0};
    int msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    Send (parent_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    delay_times = (int) reply[0];
    number_of_delays = (int) reply[1];

    int i;
    for (i = 0; i < number_of_delays; i++) {
        Delay(delay_times);
        bwprintf(COM2, "Task: %d | ticks delayed: %d | %d | delays completed: %d\n", tid, delay_times, Time(), i);
    }

    Exit();
}
