#ifndef __TRAIN__
#define __TRAIN__

#include <track_data.h>
#include <track_node.h>

#define TRAIN_SET_SPEED     4
#define TRAIN_REVERSE       3

#define COM1_PUT_DELAY  5
#define COMMAND_DELAY   15

void genTrainName( int train_id, char* bf);
void TrainTask ();
int setTrainSpeed( int num, int speed );
int reverseTrain( int num );

#endif
