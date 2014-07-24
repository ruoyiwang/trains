#include <kernel.h>

#ifndef __NAME_SERVER__
#define __NAME_SERVER__

#define NAMESERVER_TID      1

#define FOREVER for(;;)

#define REGISTER    0x10
#define WHOIS       0x11

#define LOOKUP_ARRAY_SIZE   256



typedef struct {
    char task_name[64];
    int tid;
} name_server_element;

void NameServer();

void initialize_look_up_array(name_server_element lookupArray[]);

int NameServerInsert(
    int tid,
    char name[],
    name_server_element lookupArray[]
);

int NameServerLookUp(
    char name[],
    name_server_element lookupArray[]
);

int WhoIs(char* name);
int RegisterAs(char* name);

#endif
