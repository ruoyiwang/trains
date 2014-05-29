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
    int ns_tid = Create(4, CODE_OFFSET + (&NameServer));
    // check if the created nameserver tid == NAMESERVER_TID
    if (ns_tid != 1) {
        bwprintf(COM2, "WTF is happening\n\n");
        Exit();
    }
    int tid = Create(2, CODE_OFFSET + (&spawnedTask));

    // making name server test task
    Create(3, CODE_OFFSET + (&nameServerTest1));
    Create(3, CODE_OFFSET + (&nameServerTest2));

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