#ifndef __TRAIN__
#define __TRAIN__

#include <track_data.h>
#include <track_node.h>

#define PREDICT_SENSOR      0x51
#define PATH_FIND           0x52

#define TRAIN_SET_SPEED 	4
#define TRAIN_REVERSE   	3
#define SET_SWITCH			2

#define TRACK_TASK 		"track task"

#define COM1_PUT_DELAY 	5
#define COMMAND_DELAY 	15

void genTrainName( int train_id, char* bf);
void TrainTask ();
int setTrainSpeed( int num, int speed );
int reverseTrain( int num );

void TracksTask ();

void setSwitchStatus(unsigned int* switch_status, int sw, int dir);
int getSwitchStatus(unsigned int* switch_status, int sw);

void predictSensorTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int switch_status,
    int cur_sensor,         // 0 based
    int prediction_len,     // amount of predictions wanted
    char* paths              // the triggers to be triggered
);

int predictSensor( int sensor, int prediction_len, char* result );

#endif
