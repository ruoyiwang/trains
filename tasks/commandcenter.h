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
#define TRAIN_INFO_TIME_PREDICTION 6
#define TRAIN_INFO_STOPPING_NOTIFIER 7
#define TRAIN_INFO_STOPPING_SENSOR 8
#define TRAIN_INFO_STOPPING_OFFSET 9
#define TRAIN_INFO_STOPPED 10
#define TRAIN_INFO_TIMEOUT 11
#define TRAIN_INFO_SENSOR_OFFSET 12
#define TRAIN_INFO_DEST_SENSOR 13
#define TRAIN_INFO_DEST_OFFSET 14
#define TRAIN_INFO_REVERSED 15
#define TRAIN_INFO_SIZE 16

#define COMMAND_CENTER_NOTIFIER 0
#define INIT_TRAIN_REQUEST 1
#define TRAIN_DESTINATION_REQUEST 2
#define GET_TRAIN_INFO_REQUEST 3
#define COMMAND_CENTER_STOPPING_NOTIFIER 4
#define TRAIN_REVERSE_REQUEST 5
#define GET_TRAIN_LOCATION_REQUEST 6
#define COMMAND_CENTER_TRAIN_STOPPED 7

#define TRAIN_REVERSE_OFFSET 120
#define COMMAND_CENTER_SERVER_NAME "CCS"

void CommandCenterNotifier();
void CommandCenterCourier();
void CommandCenterServer();
void CommandCenterStoppingNotifier();
int initTrainLocation( int train_id, int sensor );
int waitTrainInfo ( int train_id, int *train_info);
int distanceToDelay( int sensor, int distance, int *train_speed );
int predictArrivalTime( int sensor, int next_sensor, int init_time, int *train_speed);
int setTrainDestination( int train_id, int sensor, int offset );
int shortMoveDistanceToDelay( double distance, int train_num );
void serverSetStopping (int* train_info, int* train_speed, int sensor, int offset);
int timeToDistance( int sensor, int delta_time, int *train_speed );
int getTrainLocation ( int train_id, int* sensor, int* offset);

#endif
