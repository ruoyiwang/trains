#include <kernel.h>

#ifndef __TASKS__
#define __TASKS__

// RPS consts
#define SIGNUP 	0x20
#define PLAY 	0x21
#define QUIT	0x23
#define RPS_SERVER_NAME		"RPS SERVER"

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

void SystemIdleTask();

#endif
