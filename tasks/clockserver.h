#ifndef __CLOCK_SERVER__
#define __CLOCK_SERVER__

typedef struct {
	int tid;
	unsigned int delay;
} delay_element;


// clock tower consts
#define CLOCK_NOTIFIER		0
#define TIME_REQUEST		1
#define DELAY_REQUEST		2
#define DELAY_UNTIL_REQUEST	3
#define CLOCK_SERVER_NAME	"CLOCK SERVER"
void clockServerNotifier();
void clockServer();
int Delay( int ticks );
int DelayUntil( int ticks );
int Time();
void ClockServerTest();

#endif
