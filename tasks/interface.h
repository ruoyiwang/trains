#ifndef __INTERFACE__
#define __INTERFACE__

#define false 0
#define true 1

#define CLOCK_POSITION_X 1
#define CLOCK_POSITION_Y 1

#define CMD_POSITION_X 40
#define CMD_POSITION_Y 6

#define SW_POSITION_X 3
#define SW_POSITION_Y 1

#define TRACK_POSITION_X 5
#define TRACK_POSITION_Y 75

#define SW_STRAIGHT 33
#define SW_CURVE 34

#define SENSORS_POSITION_X 8
#define SENSORS_DISPLAY_WIDTH 8

#define IDLE_POSITION_X 2
#define IDLE_POSITION_Y 20

#define TRAIN_TABLE_X	13
#define NEXT_POSITION_Y 6
#define PREV_POSITION_Y 13
#define OFFSET_POSITION_Y 20

#define EXPECTED_POSITION_Y 31
#define ACTUAL_POSITION_Y 50
#define DEST_POSITION_Y 67

#define MAIL_POSITION_X 42
#define MAIL_OFFSET_Y 12

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
#define CMD_SHORT_MOVE_TIME	12
#define CMD_PF_DIJKSTRA		13	// path find dijkstra
#define CMD_INIT_TRACK   	14
#define CMD_CREATE_SENSOR_NOTIFIER 	15
#define CMD_SET_STOPPING_DIST 		16
#define CMD_SET_SHORT_MULT 	17
#define CMD_RESERVE_NODE	18
#define CMD_FREE_NODE		19
#define CMD_LIST_RESERVED_NODE		20
#define CMD_TRAIN_DEST_MULT	21
#define CMD_NEXT_SENSORS_CHECK		22
#define CMD_GET_FIVE_MAIL	23
#define CMD_ADD_NEW_MAIL	24

void restoreCursorPosition (char* buffer, int* index);
void saveCursorPosition (char* buffer, int* index);
void setCursor ( int row, int col, char* buffer, int* index );
void flushLine (char* buffer, int* index);
void flushScreen (char* buffer, int* index);
void DebugPutStr ( char* fmt, ... );
void cursorCommand( char * cmd, char* buffer, int* index );
void clockDisplayTask();
void initInterface();
void drawSwitchesTable ( int *row, int *col, char* buffer, int* index );
void outputPutStr ( char* str, int *row, int *col, char* buffer, int* index );
void outputPutStrLn ( char* str, int *row, int *col, char* buffer, int* index );
int getSwCursor ( int sw, int *row, int *col );
void setSwitch ( int state, int address );
void setSwitchForce ( int state, int address, int force );
void SensorsTask();
void handleCommandTask();
void parseCommand ( char* str, int *argc, char argv[10][10], int* command);
void IdleDisplayTask();
void MailDisplayTask();
unsigned int atoi ( char *str );
#endif
