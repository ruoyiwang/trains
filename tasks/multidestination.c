#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <commandcenter.h>
#include <train.h>
#include <track.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>
#include <sensors.h>
#include <commandcenter.h>
#include <multidestination.h>

void MultiDestinationNotifier() {
    // msg shits
    char msg[80] = {0};
    char reply[10] = {0};
    int msglen = 80, rpllen = 10;

    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;


    // find out who command center is
    int command_center_id = WhoIs(COMMAND_CENTER_SERVER_NAME);
    int md_server_id = WhoIs(MULTI_DESTIATION_SERVER_NAME);
    int sender_tid, train_id, destination;

    // receive train number from server
    Receive( &sender_tid, (char*)&msg_struct, msglen );
    if (sender_tid != md_server_id) {
        bwprintf(COM2, "\n\n\n\n\n\n\nImpossibleeeeeeeeee MultiDestinationNotifier %d", msg_struct.type);
    }
    Reply (sender_tid, (char *)&reply_struct, rpllen);
    train_id = msg_struct.iValue;

    msglen = 2;
    FOREVER {
        // send msg to server asking for next item in queue
        msg_struct.iValue = train_id;
        msg_struct.type = MULTI_DESTIATION_NOTIFIER;
        Send (md_server_id, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
        destination = reply_struct.iValue;

        // send msg to command center telling to do to that location
        // command center can only reply if it is fulfilled
        msg_struct.value[0] = (char) train_id;
        msg_struct.value[1] = (char) destination;
        msg_struct.type = MULTI_DESTIATION_NOTIFIER;
        Send (command_center_id, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
    }
}

void MultiDestinationServer() {
    RegisterAs(MULTI_DESTIATION_SERVER_NAME);
    int i = 0;

    char msg[80] = {0};
    char reply[10] = {0};
    int msglen = 80, rpllen = 10;

    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    int sender_tid, queue_num;

    // using queues for keep track of the destinations for each train
    int_queue destination_queues[6];    // we only have 4 trains left right?
    for (i = 0; i < 6; i++) {
        intQueueInit(&(destination_queues[i]));
    }

    // blocked notifiers will be stored in int_queue.metadata
    // the train id is stored in int_queue.id

    int train_id, destination;
    FOREVER {
        Receive( &sender_tid, (char*)&msg_struct, msglen );
        switch(msg_struct.type) {
            case MULTI_DESTIATION_NOTIFIER:
                // must have an queue set up already, it's impossible to get here otherwise
                // check which train it is
                train_id = msg_struct.iValue;
                for (i = 0; i < 6; i++) {
                    if (train_id == destination_queues[i].id) {
                        break;
                    }
                }
                // if there isn't an item in the queue
                if (intQueuePeek(&(destination_queues[i])) < 0) {
                    destination_queues[i].metadata = sender_tid;
                }
                // else we pop and reply
                else {
                    destination = intQueuePop(&(destination_queues[i]));
                    reply_struct.iValue = destination;
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                break;
            case QUEUE_DESTINATION:
                train_id = msg[0];
                destination = msg[1];
                queue_num = findCurTrainDestinationQueue(destination_queues, train_id);
                if (queue_num < 0) {
                    reply_struct.type = QUEUE_FAILED;
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    // early break
                    break;
                }
                else {
                    intQueuePush(&(destination_queues[queue_num]), destination);
                    reply_struct.type = QUEUE_SUCCESSFUL;
                    // reply to the queuer
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                }
                if (destination_queues[queue_num].metadata >= 0) {
                    // if a notifier was ever blocked on it
                    sender_tid = destination_queues[queue_num].metadata;
                    destination = intQueuePop(&(destination_queues[i]));
                    reply_struct.iValue = destination;
                    // reply to the blocked notifier
                    Reply (sender_tid, (char *)&reply_struct, rpllen);
                    destination_queues[queue_num].metadata = -1;
                }
                break;
            default:
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll MultiDestinationServer %d", msg_struct.type);
                reply_struct.type = FAIL_TYPE;
                Reply (sender_tid, (char *)&reply_struct, rpllen);
                break;
        }
    }
}

int findCurTrainDestinationQueue(int_queue destination_queues[6], int train_id) {
    int queues_full = true;
    int i = 0;
    // check if we already allocated the queue for it
    for (i = 0; i < 6; i++) {
        if (destination_queues[i].id == train_id) {
            return i;
        }
        if (destination_queues[i].id < 0) {
            queues_full = false;
        }
    }

    if (queues_full) {
        return -1;
    }

    // if we never allocated a queue for it, do it now.
    for (i = 0; i < 6; i++) {
        if (destination_queues[i].id < 0) {
            destination_queues[i].id = train_id;

            // make a notifier for it

            char msg[1] = {0};
            char reply[10] = {0};
            int msglen = 1, rpllen = 10;

            message msg_struct, reply_struct;
            msg_struct.value = msg;
            reply_struct.value = reply;

            int notifier_tid = Create(1, (&MultiDestinationNotifier));
            msg_struct.iValue = train_id;

            Send (notifier_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
            return i;
        }
    }

    return -1;  // it really shouldn't get here...
}

int queueDestination(int train_id, int destination) {
    static int receiver_id = -1;
    if (receiver_id == -1) {
        receiver_id = WhoIs(MULTI_DESTIATION_SERVER_NAME);
    }
    char msg[2] = {0};
    char reply[10] = {0};
    int msglen = 1, rpllen = 10;

    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    msg[0] = train_id;
    msg[1] = destination;
    msg_struct.type = QUEUE_DESTINATION;
    Send (receiver_id, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    if (reply_struct.type == QUEUE_SUCCESSFUL) {
        return true;
    }

    return false;
}
