#ifndef __INTERFACE__
#define __INTERFACE__

#define false 0
#define true 1

#define CLOCK_POSITION_X 1
#define CLOCK_POSITION_Y 1

#define CMD_POSITION_X 19
#define CMD_POSITION_Y 6

#define SW_POSITION_X 3
#define SW_POSITION_Y 1

#define TRACK_POSITION_X 28
#define TRACK_POSITION_Y 1

#define SW_STRAIGHT 33
#define SW_CURVE 34

#define SENSORS_POSITION_X 8
#define SENSORS_DISPLAY_WIDTH 8

#define IDLE_POSITION_X 2
#define IDLE_POSITION_Y 20

#define NEXT_POSITION_X 23
#define PREV_POSITION_X 22

#define EXPECTED_POSITION_X 25
#define ACTUAL_POSITION_X 26

#define CMD_TRAIN           1
#define CMD_REVERSE         2
#define CMD_SWITCH          3
#define CMD_QUIT            4
#define CMD_INVALID         5
#define CMD_ASSERT          6
#define CMD_PREDICT_SENSOR  7
#define CMD_FIND_DISTANCE   8
#define CMD_PATH_FIND       9
#define CMD_INIT_TRAIN   	10
#define CMD_TRAIN_DEST      11

void restoreCursorPosition (char* buffer, int* index);
void saveCursorPosition (char* buffer, int* index);
void setCursor ( int row, int col, char* buffer, int* index );
void flushLine (char* buffer, int* index);
void flushScreen (char* buffer, int* index);
void cursorCommand( char * cmd, char* buffer, int* index );
void clockDisplayTask();
void initInterface();
void drawSwitchesTable ( int *row, int *col, char* buffer, int* index );
void outputPutStr ( char* str, int *row, int *col, char* buffer, int* index );
void outputPutStrLn ( char* str, int *row, int *col, char* buffer, int* index );
int getSwCursor ( int sw, int *row, int *col );
void setSwitch ( int state, int address );
void SensorsTask();
void handleCommandTask();
void parseCommand ( char* str, int *argc, char argv[10][10], int* command);
void IdleDisplayTask();

#endif
