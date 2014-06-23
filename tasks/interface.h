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

#define SW_STRAIGHT 33
#define SW_CURVE 34

#define SENSORS_POSITION_X 8
#define SENSORS_DISPLAY_WIDTH 8

#define CMD_TRAIN 1
#define CMD_REVERSE 2
#define CMD_SWITCH 3
#define CMD_QUIT 4
#define CMD_INVALID 5

void restoreCursorPosition ();
void saveCursorPosition ();
void setCursor ( int row, int col );
void flushLine ();
void flushScreen ();
void cursorCommand( char * cmd );
void clockDisplayTask();
void initInterface();
void drawSwitchesTable ( int *row, int *col );
void outputPutStr ( char* str, int *row, int *col );
void outputPutStrLn ( char* str, int *row, int *col );
int getSwCursor ( int sw, int *row, int *col );
void setSwitch ( int state, int address );
void SensorsTask();
void handleCommandTask();
void parseCommand ( char* str, int *argc, char argv[10][10], int* command);
#endif