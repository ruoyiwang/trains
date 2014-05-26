
#define BUFFER_SIZE 1000 
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

typedef enum {TRAIN, REVERSE, SWITCH, QUIT, INVALID} command_types;

int putc( int channel, char c );

int getc( int channel );

int * getFlags( int channel );

int setuptimer3();

int setspeed( int channel, int speed );

void flushScreen ( unsigned char* buffer, int *index );

void flushLine ( unsigned char* buffer, int *index );

void setCursor ( unsigned char* buffer, int *index, int row, int col );

void saveCursorPosition ( unsigned char* buffer, int *index );

void restoreCursorPosition ( unsigned char* buffer, int *index );

void writeClockBuffer ( unsigned char* buffer, int *index, int min, int sec, int tenth );

void outputPutStr ( unsigned char* buffer, int* index, unsigned char* str, int *row, int *col );

int strcompare ( unsigned char* str1, unsigned char* str2, int length );

void parseCommand (unsigned char* str, int *argc, unsigned char argv[10][10], command_types* command);

unsigned int atoi( unsigned char *str );

void startInterface( int channel );
