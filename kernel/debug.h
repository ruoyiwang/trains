#ifndef __DEBUG__
#define __DEBUG__

#include <kernel.h>
#include <queue.h>

void assert_ker (td tds[64], td_queue td_pq[16]);

void assert_ker_msg(td tds[64], td_queue td_pq[16], int error_code);

#endif
