#include <queue.h>

void initialize_td_pq(td_queue td_pq[16]) {
    // set pq to nulls
    int i = 0;
    for (i = 0; i < 16; i++) {
        td_pq[i].front = NULL;
        td_pq[i].back = NULL;
    }
}

int pq_pop_front(td_queue td_pq[16], int pri) {
    if (td_pq[pri].front == NULL) {
        return -1;
    }
    else if (td_pq[pri].front == td_pq[pri].back) {
        int tid = td_pq[pri].front->tid;
        td_pq[pri].front = NULL;
        td_pq[pri].back = NULL;
        return tid;
    }
    else {
        int tid = td_pq[pri].front->tid;
        td_pq[pri].front = td_pq[pri].front->next;
        return tid;
    }
    return -1;
}

void pq_push_back(td_queue td_pq[16], td tds[64], int tid) {
    int pri = tds[tid].priority;
    tds[tid].state = STATE_READY;
    int i;
    for (i = 0; i<5 ; i++) {
        tds[tid].args[i] = 0;
    }
    if (td_pq[pri].front == NULL) {
        td_pq[pri].front = (tds + tid);
        td_pq[pri].back = (tds + tid);
    }
    else {
        td_pq[pri].back->next = (tds + tid);
        td_pq[pri].back = (tds + tid);
    }
}
