#include <Tasks.h>
#include <bwio.h>
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

    // making name server test task
    Create(3, CODE_OFFSET + (&nameServerTest1));
    Create(3, CODE_OFFSET + (&nameServerTest2));

    int tid = Create(2, CODE_OFFSET + (&spawnedTask));
    bwprintf(COM2, "Sending message to %d: %s\n",tid, msg);
    Send (tid, (char *)&msg_struct, 14, (char *)&reply_struct, 10);
    bwprintf(COM2, "Got reply from %d with type %d: %s\n",tid,reply_struct.type, reply);

    // int tid = Create(3, CODE_OFFSET + (&spawnedTask));
    // bwprintf(COM2, "Created: %d\n", tid);

    // tid = Create(3, CODE_OFFSET + (&spawnedTask));
    // bwprintf(COM2, "Created: %d\n", tid);

    // tid = Create(1, CODE_OFFSET + (&spawnedTask));
    // bwprintf(COM2, "Created: %d\n", tid);

    // tid = Create(1, CODE_OFFSET + (&spawnedTask));
    // bwprintf(COM2, "Created: %d\n", tid);

    // bwprintf(COM2, "First: Exiting\n" );
    Exit();
}

void spawnedTask () {
    int *tid;
    char msg[20];
    char reply[20] = "I am a reply";
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    reply_struct.type = 10;
    Receive (tid, (char *)&msg_struct, 14);
    bwprintf(COM2, "Received message from %d with type %d: %s\n",*tid, msg_struct.type, msg);
    bwprintf(COM2, "Reply message to %d: I am a reply\n",*tid);

    Reply (*tid, (char *)&reply_struct, 14);
    Exit();
}

void nameServerTest1 () {
    RegisterAs("nameServerTest1");
    int randomtid = WhoIs("nameServerTest2");
    bwprintf(COM2, "nameServerTest2's tid is: %d\n", randomtid);
    Exit();
}

void nameServerTest2 () {
    RegisterAs("nameServerTest2");
    int randomtid = WhoIs("nameServerTest1");
    bwprintf(COM2, "nameServerTest1's tid is: %d\n", randomtid);
    Exit();
}