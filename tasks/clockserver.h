#ifndef __CLOCK_SERVER__
#define __CLOCK_SERVER__

typedef struct {
	int tid;
	unsigned int delay;
} delay_element;


// clock tower consts
#define NOTIFIER			0x30
#define TIME_REQUEST		0x31
#define DELAY_REQUEST		0x32
#define DELAY_UNTIL_REQUEST	0x33
#define CLOCK_SERVER_NAME	"CLOCK SERVER"
void clockServerNotifier();
void clockServer();
int Delay( int ticks );
int DelayUntil( int ticks );
int Time();
void ClockServerTest();

#endif
