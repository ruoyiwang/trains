#include <kernel.h>

#ifndef __QUEUE__
#define __QUEUE__

typedef struct td_queue_t {
	struct td_t * front;
	struct td_t * back;
} td_queue;

void initialize_td_pq(td_queue td_pq[16]);

int pq_pop_front(td_queue td_pq[16], int pri);

void pq_push_back(td_queue td_pq[16], td tds[64], int tid);

#endif
