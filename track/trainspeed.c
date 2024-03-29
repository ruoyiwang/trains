#include <trainspeed.h>

// expecting an int array of 80
void init_train_speed(int train_num, int* train_speeds) {
	train_speeds[0] = 532;
	train_speeds[1] = 532;
	train_speeds[2] = 501;
	train_speeds[3] = 501;
	train_speeds[4] = 532;
	train_speeds[5] = 532;
	train_speeds[6] = 532;
	train_speeds[7] = 532;
	train_speeds[8] = 532;
	train_speeds[9] = 532;
	train_speeds[10] = 452;
	train_speeds[11] = 452;
	train_speeds[12] = 532;
	train_speeds[13] = 532;
	train_speeds[14] = 478;
	train_speeds[15] = 478;
	train_speeds[16] = 485;
	train_speeds[17] = 485;
	train_speeds[18] = 532;
	train_speeds[19] = 532;
	train_speeds[20] = 247;
	train_speeds[21] = 247;
	train_speeds[22] = 532;
	train_speeds[23] = 532;
	train_speeds[24] = 532;
	train_speeds[25] = 532;
	train_speeds[26] = 532;
	train_speeds[27] = 532;
	train_speeds[28] = 664;
	train_speeds[29] = 664;
	train_speeds[30] = 554;
	train_speeds[31] = 554;
	train_speeds[32] = 574;
	train_speeds[33] = 574;
	train_speeds[34] = 453;
	train_speeds[35] = 453;
	train_speeds[36] = 532;
	train_speeds[37] = 532;
	train_speeds[38] = 474;
	train_speeds[39] = 474;
	train_speeds[40] = 530;
	train_speeds[41] = 530;
	train_speeds[42] = 495;
	train_speeds[43] = 495;
	train_speeds[44] = 538;
	train_speeds[45] = 538;
	train_speeds[46] = 523;
	train_speeds[47] = 523;
	train_speeds[48] = 462;
	train_speeds[49] = 462;
	train_speeds[50] = 532;
	train_speeds[51] = 532;
	train_speeds[52] = 520;
	train_speeds[53] = 520;
	train_speeds[54] = 502;
	train_speeds[55] = 502;
	train_speeds[56] = 386;
	train_speeds[57] = 386;
	train_speeds[58] = 569;
	train_speeds[59] = 569;
	train_speeds[60] = 586;
	train_speeds[61] = 586;
	train_speeds[62] = 591;
	train_speeds[63] = 591;
	train_speeds[64] = 614;
	train_speeds[65] = 614;
	train_speeds[66] = 532;
	train_speeds[67] = 532;
	train_speeds[68] = 670;
	train_speeds[69] = 670;
	train_speeds[70] = 530;
	train_speeds[71] = 530;
	train_speeds[72] = 392;
	train_speeds[73] = 392;
	train_speeds[74] = 651;
	train_speeds[75] = 651;
	train_speeds[76] = 806;
	train_speeds[77] = 806;
	train_speeds[78] = 574;
	train_speeds[79] = 574;

	if (train_num == 49) {
		int i;
		for ( i = 0; i < 80; i++) {
			train_speeds[i] = train_speeds[i] * 1.062;
		}
	}
}

void get_train_calibrations( int train_num, int* train_stopping_dist, double* short_move_multiplier, double* speed_multiplier) {
	if (train_num == 49){
		*train_stopping_dist = 780;
		*short_move_multiplier = 110;
		*speed_multiplier = 100;
	}
	else if (train_num == 56){
		*train_stopping_dist = 670;
		*short_move_multiplier = 100;
		*speed_multiplier = 120;
	}
	else if (train_num == 54){
		*train_stopping_dist = 670;
		*short_move_multiplier = 100;
		*speed_multiplier = 120;
	}
	else {
		*train_stopping_dist = 790;
		*short_move_multiplier = 100;
		*speed_multiplier = 100;
	}
}
