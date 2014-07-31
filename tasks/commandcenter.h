#ifndef __COMMANDCENTER__
#define __COMMANDCENTER__

#include <intqueue.h>

#define INITIAL_SENSOR 44
#define MAX_TRAIN_COUNT 3
#define UNSAFE_OFF_COURSE_SIGNAL -2
#define REVERSE_SIGNAL -3
#define UNSAFE_REVERSE_SIGNAL -4
#define RESERVED_STOPPING -5
#define UNSAFE_FORWORD -6

typedef struct train_info_t {
	int id;
	int sensor;
	int next_sensor;
	int arrival_tme;
	int task_tid;
	int courier_tid;
	int notifier_tid;
	int time_prediction;
	int stopping_notifier;
	int stopping_sensor;
	int stopping_offset;
	int is_stopped;
	int timeout;
	int offset;
	int dest_offset;
	int dest_sensor;
	int is_reversed;
	int stopping_dist;
	int off_course;
	int signal;
	int check_notifier;
	int_queue dest_queue;
	double short_move_mult;
	double speed_mult;
	int node_list_len;
	move_node node_list[TRACK_MAX];
	int cur_mail_index;
} Train_info;

typedef struct mail_t {
	int from;
	int to;
	int status;
	int mail_type;
} mail;

#define MAIL_LIST_SIZE			20
#define MAIL_STATUS_EMPTY		0
#define MAIL_STATUS_NEW			1
#define MAIL_STATUS_PICKUP		2
#define MAIL_STATUS_IN_FLIGHT	3
#define MAIL_STATUS_DELIVERED	4
#define MAIL_TYPE_RANDOM		0
#define MAIL_TYPE_REAL			1

#define COMMAND_CENTER_NOTIFIER 0
#define INIT_TRAIN_REQUEST 1
#define TRAIN_DESTINATION_REQUEST 2
#define GET_TRAIN_INFO_REQUEST 3
#define COMMAND_CENTER_STOPPING_NOTIFIER 4
#define TRAIN_REVERSE_REQUEST 5
#define GET_TRAIN_LOCATION_REQUEST 6
#define COMMAND_CENTER_TRAIN_STOPPED 7
#define SET_TRAIN_STOPPING_DIST 8
#define SET_TRAIN_SHORT_MULT 9
#define COMMAND_CENTER_DELAY_EXPIRED 10
#define COMMAND_CENTER_TRAIN_CHECK 11
#define COMMAND_CENTER_DEADLOCK_CHECK 12
#define GET_FIVE_MAIL 13
#define ADD_NEW_MAIL 14

#define TRAIN_REVERSE_OFFSET 120
#define COMMAND_CENTER_SERVER_NAME "CCS"

void CommandCenterNotifier();
void CommandCenterCourier();
void CommandCenterServer();
void CommandCenterStoppingNotifier();
void CommandCenterAdjustTask();
int initTrainLocation( int train_id, int sensor );
int waitTrainInfo ( int train_id, int *train_info);
int distanceToDelay( int sensor, int distance, int *train_speed , double short_move_mult );
int predictArrivalTime( int sensor, int next_sensor, int init_time, int *train_speed, double short_move_mult );
int setTrainDestination( int train_id, int sensor, int offset );
int shortMoveDistanceToDelay( double distance, int train_num, double mult );
void serverSetStopping (Train_info* train_info, int* train_speed, int sensor, int offset, int* requests);
int timeToDistance( int sensor, int delta_time, int *train_speed , double short_move_mult );
int getTrainLocation ( int train_id, int* sensor, int* offset);
int setTrainShortMult ( int train_id, int short_mult);
int setTrainStopDist ( int train_id, int stop_dist);


// mailing
void queueNewMailCommandCenter (mail mail_list[MAIL_LIST_SIZE], int from, int to);
int getNextNewMail (mail mail_list[MAIL_LIST_SIZE], int* mail_index);
int deliverMail( mail mail_list[MAIL_LIST_SIZE], int mail_index );
void getFiveMailCommandCenter (mail mail_list[MAIL_LIST_SIZE], mail ret_list[5]);
void getFiveMail(mail mail_list[5]);
void initMailList(mail mail_list[MAIL_LIST_SIZE]);
void queueNewMail (int from, int to);
#endif
