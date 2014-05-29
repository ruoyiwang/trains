#include <kernel.h>

#ifndef __TASKS__
#define __TASKS__

#define SIGNUP 	0x20
#define PLAY 	0x21
#define QUIT	0x23

#define RPS_SERVER_NAME		"RPS SERVER"

void spawnedTask ();
void FirstUserTask ();

void nameServerTest1 ();
void nameServerTest2 ();

void ReceiveTask1();
void sendTask1();

#endif
