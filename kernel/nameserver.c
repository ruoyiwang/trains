#include <nameserver.h>
#include <util.h>
#include <bwio.h>

void initialize_look_up_array(name_server_element lookupArray[]) {
    int i = 0;
    for (i = 0; i < LOOKUP_ARRAY_SIZE; i++) {
        lookupArray[i].task_name[0] = NULL;
        lookupArray[i].tid = -1;
    }
}

/*
    @name:      NameServerInsert
    @auth:      Roy
    @param:     tid, name, lookupArray
    @return:    0   - shit worked
                -1  - didn't work
    @desc:      inserts tid, name into the lookup array
*/
int NameServerInsert(
    int tid,
    char name[],
    name_server_element lookupArray[]
) {
    // TODO: implement overwrite
    int i = 0;
    for ( i = 0; i < LOOKUP_ARRAY_SIZE; i++ ) {
        // if repeated task name, overwrite
        if (strcmp(lookupArray[i].task_name, name) == 0) {
            // the current implementation is overwrite
            lookupArray[i].tid = tid;
            return 0;
        }
        // if tid == -1, which means the spot is not taken, take it
        if (lookupArray[i].tid == -1) {
            strcpy(lookupArray[i].task_name, name);
            lookupArray[i].tid = tid;
            return 0;
        }
    }
    // if it gets here it means the array is full, return neg 1
    return -1;
}


/*
    @name:      NameServerLookUp
    @auth:      Roy
    @param:     name, lookupArray
    @return:    0-63:   the tid of the found task
                -1:     couldn't find the task
    @desc:      find the task id with the name in the lookup array
*/
int NameServerLookUp(
    char name[],
    name_server_element lookupArray[]
) {
    int i = 0;
    for ( i = 0; i < LOOKUP_ARRAY_SIZE; i++ ) {
        // if repeated found task name
        if (strcmp(lookupArray[i].task_name, name) == 0) {
            // return the tid;
            return lookupArray[i].tid;
        }
    }
    // Assert();
    return -1;
}

void NameServer() {
    // make a fixed size array as the lookup table lol;
    name_server_element lookupArray[LOOKUP_ARRAY_SIZE];
    initialize_look_up_array(lookupArray);

    int sender_tid; // used for receive
    int tid;        // used for whois
    int msglen = 64;
    char msg[64] = {0};
    char task_name[64] = {0};
    char reply[64] = {0};
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;
    // initialize server internals
    FOREVER {
        memset(msg, 0, 64);
        msglen = 64;
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        memset(task_name, 0, 64);
        switch( msg_struct.type ) {
            case REGISTER:
                strcpy(task_name, msg_struct.value);
                NameServerInsert(sender_tid, task_name, lookupArray);
                // TODO: assert the return of insert is success
                Reply (sender_tid, (char *)&reply_struct, 64);
                break;
            case WHOIS:
                strcpy(task_name, msg_struct.value);
                tid = NameServerLookUp(task_name, lookupArray);
                // putting tid as a first char because tid is a number 0-63
                if (tid < 0) {
                    strcpy(reply_struct.value, "fail");
                }
                else {
                    reply_struct.value[0] = tid;
                }
                Reply (sender_tid, (char *)&reply_struct, 64);
                break;
            default:
                // This should never happen
                // TODO: reply false or some shit
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll NAMESEVER %d", msg_struct.type);
                reply_struct.type = FAIL_TYPE;
                Reply (sender_tid, (char *)&reply_struct, 64);
                break;
        }
    }
}

int RegisterAs(char* name) {
    if (strcmp(name, "") == 0) {
        return -3;
    }
    message msg_struct, reply_struct;
    char reply[64] = {0};
    char msg[64] = {0};

    // the msg is the name;
    strcpy(msg, name);

    // form the struct
    msg_struct.value = msg;
    msg_struct.type = REGISTER;

    reply_struct.value = reply;
    int tid = NAMESERVER_TID;

    Send (tid, (char *)&msg_struct, 64, (char *)&reply_struct, 64);

    // TODO: check the reply struct for errs
    if (strcmp(reply_struct.value, "fail") == 0) {
        return -1;
    }
    return 0;
}

int WhoIs(char* name) {
    message msg_struct, reply_struct;
    char reply[64] = {0};
    char msg[64] = {0};

    // the msg is the name;
    strcpy(msg, name);

    // form the struct
    msg_struct.value = msg;
    msg_struct.type = WHOIS;

    reply_struct.value = reply;
    int tid = NAMESERVER_TID;

    Send (tid, (char *)&msg_struct, 64, (char *)&reply_struct, 64);

    // TODO: check the reply struct for errs
    if (strcmp(reply_struct.value, "fail") == 0) {
        return -1;
    }
    else {
        tid = reply_struct.value[0];
        return tid;
    }
    // yeah ==
    return -1;
}
