#include <Tasks.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <nameserver.h>
#include <kernel.h>

void FirstUserTask () {
    // fire off the
    initTimers();
    // int * VIC2SoftInt =(int *) (VIC2_BASE + VICxSoftInt);
    // *VIC2SoftInt = *VIC2SoftInt | (1 << 19);

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

    // playtRPS();
    // perfTest();

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

void rpsClient () {
    int myTid = MyTid();

    volatile unsigned int * timer_4_low;
    timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );

    // make msg and reply
    message msg_struct, reply_struct;
    char reply[64] = {0};
    char msg[2] = {0};
    msg_struct.value = msg;
    reply_struct.value = reply;

    // test code: my RPS play will be the tid%3
    int iMyPlay = 0 ;
    // lookup who's the server
    int rpsServerTid = WhoIs(RPS_SERVER_NAME);

    int i = 0;
    for (i = 0; i < rand(*timer_4_low) % 5 + 5; i++) {
        //figure out what my play is
        iMyPlay = rand(*timer_4_low) % 3;
        char myPlay;
        switch (iMyPlay) {
            case 0:
                myPlay = 'r';
                break;
            case 1:
                myPlay = 'p';
                break;
            case 2:
                myPlay = 's';
                break;
            case 3:
                break;
        }

        // SIGNUP
        msg_struct.type = SIGNUP;
        // bwprintf(COM2, "Tid: %d | signing up\n", myTid );
        Send (rpsServerTid, (char *)&msg_struct, 2, (char *)&reply_struct, 64);
        // bwprintf(COM2, "Tid: %d | singed up\n", myTid );
        // TODO: loop if reply tells me I can't play
        //  if reply fails, try signup again
        //  if reply goes, try play

        // send PLAY
        msg_struct.type = PLAY;
        msg_struct.value[0] = myPlay;
        // bwprintf(COM2, "Tid: %d | Playes: %c\n", myTid, myPlay );
        Send (rpsServerTid, (char *)&msg_struct, 2, (char *)&reply_struct, 64);

        // display results: debug line?
        bwprintf(COM2, "Tid: %d | Player played: %c, result: %s\n", myTid, myPlay, reply_struct.value );
        // loop or quit
    }
    // // send quit
    msg_struct.type = QUIT;
    bwprintf(COM2, "Tid: %d | quitting\n" );
    Send (rpsServerTid, (char *)&msg_struct, 2, (char *)&reply_struct, 64);
    // don't care result
    // exit
    Exit();
    // stop
}

// assuming no malicious client
void rpsServer() {
    // for(;;){}
    int myTid = MyTid();

    // register name
    RegisterAs(RPS_SERVER_NAME);

    // init array used for the matches
    int playerMatchUp[64];  // this is tid for tid
    char playerPlay[64];    // tid-ness
    int i = 0;
    for (i = 0; i < 64; i++) {
        playerMatchUp[i] = -1;
        playerPlay[i] = NULL;
    }
    int playerSignUpQueue[2];
    playerSignUpQueue[0] = -1;
    playerSignUpQueue[1] = -1;

    // make msg and reply
    message msg_struct, reply_struct1, reply_struct2;
    char reply1[64] = {0};
    char reply2[64] = {0};
    char msg[64] = {0};
    msg_struct.value = msg;
    reply_struct1.value = reply1;
    reply_struct2.value = reply2;
    int msglen = 64;

    int sender_tid;
    int opponent;
    int playcounter = 0;

    FOREVER {
        // get request
        msg_struct.value[0] = 0;
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        // bwprintf(COM2, "Sever | Received from player%d\n", sender_tid );
        // switch type
        switch( msg_struct.type ) {
            // if signup
            case SIGNUP:
                // queue player
                // bwprintf(COM2, "Tid: %d | Player %d SIGNEDUP\n", myTid, sender_tid );
                if (playerSignUpQueue[0] == -1) {
                    playerSignUpQueue[0] = sender_tid;
                }
                else {
                    playerSignUpQueue[1] = sender_tid;
                    // if queue == 2,
                    // throw them to the playerMatchUp array
                    playerMatchUp[playerSignUpQueue[0]] = playerSignUpQueue[1];
                    playerMatchUp[playerSignUpQueue[1]] = playerSignUpQueue[0];
                    // reply let them play

                    // bwprintf(COM2, "Tid: %d | replying to player%d\n", myTid, sender_tid );

                    // bwprintf(COM2, "Sever | replying to player%d\n", sender_tid );
                    Reply (sender_tid, (char *)&reply_struct1, 64);
                    // bwprintf(COM2, "Tid: %d | replying to player%d\n", myTid, playerSignUpQueue[0] );
                    // bwprintf(COM2, "Sever | replying to player%d\n", sender_tid );
                    Reply (playerSignUpQueue[0], (char *)&reply_struct1, 64);
                    // clear playerSignUpQueue
                    playerSignUpQueue[0] = -1;
                    playerSignUpQueue[1] = -1;
                }
                break;
            // if play
            case PLAY:
                // store the play into playerPlay, since the play is only 1 char, we just do 1 char
                playcounter++;
                playerPlay[sender_tid] = msg_struct.value[0];
                opponent = playerMatchUp[sender_tid];
                // check opponent has exists
                if (opponent != -1) {
                    // if opponent has quit
                    if (playerPlay[opponent] == 'q') {
                        // reply to cur dude you matchup ditch you
                        strcpy(reply_struct1.value, "Your opponent has quit");
                        reply_struct1.type = QUIT;
                        Reply (sender_tid, (char *)&reply_struct1, 64);
                        // clear match up and player play for these two players
                        playerPlay[sender_tid] = NULL;
                        playerPlay[opponent] = NULL;
                        playerMatchUp[sender_tid] = -1;
                        playerMatchUp[opponent] = -1;
                    }
                    // if opponent has played
                    else if (playerPlay[opponent] != NULL /*&& playcounter == 2*/) {
                        // it's either r, p, or s
                        // compare results, reply both
                        // bwprintf(COM2, "player: %d played %c\n", sender_tid, playerPlay[sender_tid]);
                        // bwprintf(COM2, "player: %d played %c\n", opponent, playerPlay[opponent]);
                        if (playerPlay[opponent] == playerPlay[sender_tid]) {
                            strcpy(reply_struct1.value, "TIE");
                            strcpy(reply_struct2.value, "TIE");
                        }
                        else if (playerPlay[sender_tid] == 'r' && playerPlay[opponent] == 'p') {
                            strcpy(reply_struct1.value, "\033[31mLOSE\033[37m");
                            strcpy(reply_struct2.value, "\033[32mWIN\033[37m");
                        }
                        else if (playerPlay[sender_tid] == 'r' && playerPlay[opponent] == 's') {
                            strcpy(reply_struct1.value, "\033[32mWIN\033[37m");
                            strcpy(reply_struct2.value, "\033[31mLOSE\033[37m");
                        }
                        else if (playerPlay[sender_tid] == 'p' && playerPlay[opponent] == 's') {
                            strcpy(reply_struct1.value, "\033[31mLOSE\033[37m");
                            strcpy(reply_struct2.value, "\033[32mWIN\033[37m");
                        }
                        else if (playerPlay[sender_tid] == 'p' && playerPlay[opponent] == 'r') {
                            strcpy(reply_struct1.value, "\033[32mWIN\033[37m");
                            strcpy(reply_struct2.value, "\033[31mLOSE\033[37m");
                        }
                        else if (playerPlay[sender_tid] == 's' && playerPlay[opponent] == 'r') {
                            strcpy(reply_struct1.value, "\033[31mLOSE\033[37m");
                            strcpy(reply_struct2.value, "\033[32mWIN\033[37m");
                        }
                        else if (playerPlay[sender_tid] == 's' && playerPlay[opponent] == 'p') {
                            strcpy(reply_struct1.value, "\033[32mWIN\033[37m");
                            strcpy(reply_struct2.value, "\033[31mLOSE\033[37m");
                        }
                        else {
                            bwprintf(COM2, "WTF, it shouldn't get here TT_TT\n");
                        }

                        // clear match up and player play for these two players
                        playerPlay[sender_tid] = NULL;
                        playerPlay[opponent] = NULL;
                        playerMatchUp[sender_tid] = -1;
                        playerMatchUp[opponent] = -1;

                        //bwprintf(COM2, "Server | sending play result to player %d\n", sender_tid);

                        // bwprintf(COM2, "Sever | replying to player%d\n", sender_tid );
                        Reply (sender_tid, (char *)&reply_struct1, 64);
                        // bwprintf(COM2, "Server | sending play result to player %d\n", opponent);
                        // bwprintf(COM2, "Sever | replying to player%d\n", opponent );
                        Reply (opponent, (char *)&reply_struct2, 64);
                        // bwprintf(COM2, "Server | sent play result to player %d\n", opponent);

                        bwprintf (COM2, "Press any key to see result of next match. (press 'q' to exit)\n\n");
                        char input = bwgetc(COM2);
                        if (input == 'q') Exit();
                    }
                }
                else {
                    // this means the player played before they signup, BM
                    bwprintf(COM2, "WTF, player%d played without signup\n", sender_tid);
                }
                break;
            // if quit
            case QUIT:
                // if dude is not matched, don't care
                opponent = playerMatchUp[sender_tid];
                if (opponent == -1) {
                    Reply (sender_tid, (char *)&reply_struct1, 64);
                }
                // else
                else {
                    // set quit in playerPlay
                    playerPlay[sender_tid] = 'q';
                    // reply aight
                    Reply (sender_tid, (char *)&reply_struct1, 64);

                    // if opponent has played
                    if (playerPlay[opponent] != NULL) {
                        // reply to opponentt this guy has quit
                        strcpy(reply_struct1.value, "Your opponent has quit");
                        reply_struct1.type = QUIT;
                        Reply (opponent, (char *)&reply_struct1, 64);
                        // clear err thing
                        playerPlay[sender_tid] = NULL;
                        playerPlay[opponent] = NULL;
                        playerMatchUp[sender_tid] = -1;
                        playerMatchUp[opponent] = -1;
                    }
                }
                break;
            default:
                // TODO: reply some sort of error and we are screwed
                break;
        }
    }

    bwprintf(COM2, "Server Exiting\n");
    Exit();
}

void playtRPS() {
    Create(4, CODE_OFFSET + (&rpsServer));
    int i;
    for (i = 0; i< 50; i++) {
        Create(4, CODE_OFFSET + (&rpsClient));
    }
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

    int msglen = 4;
    char msg[4] = {0};
    char reply[4] = {0};
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

    for (i = 0; i < 100000; i++) {
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
    int msglen = 4;
    int sender_tid;
    char msg[4] = {0};
    char reply[4] = {0};

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
            }
        }
    }
}

void queueDelay(delay_element delays[64], int tid, int delay) {
    int i;
    for (i = 0; i < 64; i++) {
        if (delays[i].tid == -1) {
            delays[i].tid = tid;
            delays[i].delay = delay;
        }
    }
}

void clockServer() {
    int curTime = 0;
    // Create Notifier and send any initialization data
    Create(2, CODE_OFFSET + (&clockServerNotifier));
    // Initialize self
    RegisterAs(CLOCK_SERVER_NAME);
    // msg shits
    char msg[8] = {0};
    char reply[8] = {0};
    int sender_tid, msglen = 8;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    // the array that takes in requests
    delay_element delays[64];   // 64 of them
    // each tid can only call one delay at a time
    int i;
    for (i = 0; i < 64; i++) {
        delays[i].tid = -1;
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
                Reply (sender_tid, (char *)&reply_struct, msglen);
                // reply what the time is
                break;
            case DELAY_REQUEST:
                // add request to list of suspended tasks
                queueDelay(delays, sender_tid, msg_struct.iValue);
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
    int receiver_tid = WhoIs(CLOCK_SERVER_NAME);
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
    int receiver_tid = WhoIs(CLOCK_SERVER_NAME);
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = TIME_REQUEST;
    reply_struct.value = reply;

    int curTime = 0;
    Send (receiver_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    if (strcmp(msg_struct.value, "FAIL") != 0) {
        // if succeded
        curTime = msg_struct.iValue;
        return curTime;
    }
    return -1;
}

int DelayUntil( int ticks ) {

    return -1;
}

void ClockServerTest() {
    Create(2, CODE_OFFSET + (&clockServer));
    int t = Time();
    bwprintf(COM2, "curTime%d\n", t);
}