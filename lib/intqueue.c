#include <intqueue.h>

int intQueuePeek(int_queue *q) {
    if ( q->length == 0 ) {
        return -1;
    }
    return q->elements[q->front];
}

int intQueuePop(int_queue *q) {
    if ( q->length == 0 ) {
        return -1;
    }

    int ret = q->elements[q->front];

    q->elements[q->front] = -1;
    q->front = ( q->front + 1 ) % INT_QUEUE_SIZE;
    q->length--;

    return ret;
}

int intQueuePush(int_queue *q, int e) {
    if ( q->length >= INT_QUEUE_SIZE ) {
        return -1;
    }

    int insertion_point = ( q->front + q->length ) % INT_QUEUE_SIZE;
    q->elements[insertion_point] = e;
    q->length++;

    return 1;
}

void intQueueInit(int_queue *q) {
    int i = 0;
    for (i = 0; i < INT_QUEUE_SIZE; i++) {
        q->elements[i] = -1;
    }
    q->front = 0;
    q->length = 0;
    q->id = -1;
    q->metadata = -1;
}
