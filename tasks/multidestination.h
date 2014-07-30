#ifndef __MULTI_DESTINATION__
#define __MULTI_DESTINATION__

#include <intqueue.h>

#define MULTI_DESTIATION_NOTIFIER	70
#define QUEUE_DESTINATION			71

#define QUEUE_SUCCESSFUL			80
#define QUEUE_FAILED				81

#define MULTI_DESTIATION_SERVER_NAME	"mds"

void MultiDestinationNotifier();

void MultiDestinationServer();

int findCurTrainDestinationQueue(int_queue destination_queues[6], int train_id);

int queueDestination(int train_id, int destination);

#endif
