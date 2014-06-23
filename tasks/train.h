#ifndef __TRAIN__
#define __TRAIN__

#define TRAIN_SET_SPEED 4
#define TRAIN_REVERSE   3
#define SET_SWITCH		2
#define TRACK_TASK 		"track task"

#define COM1_PUT_DELAY 	5
#define COMMAND_DELAY 	15

void genTrainName( int train_id, char* bf);
void TrainTask ();
void TracksTask ();

#endif
