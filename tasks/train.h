#ifndef __TRAIN__
#define __TRAIN__

#include <track_data.h>
#include <track_node.h>

#define PREDICT_SENSOR      0x51
#define PATH_FIND           0x52
#define FIND_DISTANCE_BETWEEN_TWO_LANDMARKS	0x53

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

int findDistanceBetweenLandmarksTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int switch_status,
    int landmark_start,     // 0 based
    int landmark_end,       // 0 based
    int lookup_limit        // amount of predictions wanted
);
int findDistanceBetweenLandmarks(
    int landmark1, int landmark2, int lookup_limit
);
int predictSensor( int sensor, int prediction_len, char* result );


// path shits
int pathFindTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int* switch_status,
    int cur_sensor,         // 0 based
    int stopping_node,      // amount of predictions wanted
    int* stopping_sensor,   // the triggers to be triggered
    int* stoppong_sensor_dist,   // returning distance
    char* sensor_route
);

int bfsPathFind(        // pretty shit path find lol
    track_node *tracks,
    track_node* begin_node,
    track_node* end_node,
    track_node** path   // the path the train's gonna take
);

int pathFind(
    int cur_sensor,             // current node
    int dest_node,              // where it wants to go
    int stopping_dist,          // stoping distance
    int* stopping_sensor,       // returning node
    int* stoppong_sensor_dist,  // returning distance
    char* sensor_route          // the sensors the train's gonna pass
);

void makePath(track_node* node, track_node* init_node, track_node** path);

void setSwitchTrackTask(int switch_num, char switch_dir, unsigned int* switch_status);

#endif
