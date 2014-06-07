#include <kernel.h>

#ifndef __TASKS__
#define __TASKS__

// RPS consts
#define SIGNUP 	0x20
#define PLAY 	0x21
#define QUIT	0x23
#define RPS_SERVER_NAME		"RPS SERVER"

// clock tower consts
#define NOTIFIER			0x30
#define TIME_REQUEST		0x31
#define DELAY_REQUEST		0x32
#define CLOCK_SERVER_NAME	"CLOCK SERVER"

void spawnedTask ();
void FirstUserTask ();

void nameServerTest1 ();
void nameServerTest2 ();

void rpsClient ();
void rpsServer();
void playtRPS();

void perfTest();
void testSend();
void testReceive();

void clockServer();

#endif
