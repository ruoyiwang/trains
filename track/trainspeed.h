#ifndef __TRAIN_SPEED__
#define __TRAIN_SPEED__

void init_train_speed(int train_num, int* train_speeds);
void get_train_calibrations( int train_num, int* train_stopping_dist, double* short_move_multiplier, double* speed_multiplier);

#endif
