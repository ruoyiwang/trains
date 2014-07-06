#ifndef __COMMANDCENTER__
#define __COMMANDCENTER__

#define INITIAL_SENSOR 44
#define MAX_TRAIN_COUNT 3

#define TRAIN_INFO_ID 0
#define TRAIN_INFO_SENSOR 1
#define TRAIN_INFO_NEXT_SENSOR 2
#define TRAIN_INFO_TIME 3
#define TRAIN_INFO_TASK 4
#define TRAIN_INFO_COURIER 5

#define COMMAND_CENTER_NOTIFIER 0
#define INIT_TRAIN_REQUEST 1
#define GET_TRAIN_INFO_REQUEST 2
#define COMMAND_CENTER_SERVER_NAME "Command Center Server"

void CommandCenterNotifier();
void CommandCenterCourier();
void CommandCenterServer();
int initTrainLocation( int train_id, int sensor );
int waitTrainInfo ( int train_id, char *train_info);

#endif