#include <Tasks.h>
#include <bwio.h>
#include <util.h>
#include <nameserver.h>

void FirstUserTask () {
    char msg[20] = "I am a message";
    char reply[20];
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = 12;
    reply_struct.value = reply;

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

    Create(4, CODE_OFFSET + (&rpsServer));

    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));
    Create(4, CODE_OFFSET + (&rpsClient));


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

    // make msg and reply
    message msg_struct, reply_struct;
    char reply[64] = {0};
    char msg[2] = {0};
    msg_struct.value = msg;
    reply_struct.value = reply;

    // test code: my RPS play will be the tid%3
    int iMyPlay = myTid % 3;
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

    // lookup who's the server
    int rpsServerTid = WhoIs(RPS_SERVER_NAME);

    int i = 0;
    for (i = 0; i < myTid; i++) {
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
