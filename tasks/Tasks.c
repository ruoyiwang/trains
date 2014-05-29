#include <Tasks.h>
#include <bwio.h>

void FirstUserTask () {
    char msg[20] = "I am a message";
    char reply[20] ;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = 12;
    reply_struct.value = reply;
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