#include <debug.h>
#include <bwio.h>

// the function that the kernel calls to print all the statuses
void assert_ker(
    td tds[64],
    td_queue td_pq[16]
) {
    int i = 0;
    // print the tasks
    // tid || status
    bwprintf(COM2, "\n id || pr || st\n", tds[i].tid, tds[i].priority, tds[i].state);
    for (i = 0; i < 64; i++) {
        if (i < 10 && tds[i].priority < 10) {
            bwprintf(COM2, " %d  || %d  || %d\n", tds[i].tid, tds[i].priority, tds[i].state);
        }
        else if (i < 10) {
            bwprintf(COM2, " %d  || %d || %d\n", tds[i].tid, tds[i].priority, tds[i].state);
        }
        else if (tds[i].priority < 10) {
            bwprintf(COM2, " %d || %d  || %d\n", tds[i].tid, tds[i].priority, tds[i].state);
        }
        else {
            bwprintf(COM2, " %d || %d || %d\n", tds[i].tid, tds[i].priority, tds[i].state);
        }

    }

    FOREVER{ }
}
