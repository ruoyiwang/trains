#ifndef __INTQUEUE__
#define __INTQUEUE__

#define INT_QUEUE_SIZE  20

typedef struct int_queue_t {
    int elements[INT_QUEUE_SIZE];
    int front;
    int length;
    int id;         // used by caller
    int metadata;   // used by caller
} int_queue;

int intQueuePeek(int_queue *q);

int intQueuePop(int_queue *q);

int intQueuePush(int_queue *q, int e);

void intQueueInit(int_queue *q);

#endif
