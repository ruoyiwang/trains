#ifndef __KERNEL__
#define __KERNEL__

#define CODE_OFFSET         0x218000

#define REQUEST_CREATE 	    2

#define NULL                0

#define STATE_READY         0
#define STATE_ACTIVE        1
#define STATE_ZOMBIE        2

#define USER_STACK_SIZE     0x1000      // 0x1000 = 4096
#define USER_STACK_BEGIN    0x1000000

typedef struct td_t {
    unsigned int tid;
    unsigned int pc;
    unsigned int sp;
    unsigned int spsr;
    unsigned int ret;
    unsigned int priority;
    unsigned int parent_tid;
    unsigned int state;
    struct td_t * next;
} td;

void first();

void SVC_HANDLER();

void initialize ( td tds[64] );

void testContextNOP();

int testContext();

int main( int argc, char* argv[] );

int ker_exit ( td *active, int* args );

void ker_entry ();

void swi_stub();

int Create( int priority, void (* pc) ());

int MyTid();

int MyParentTid();

int Pass();

int Exit();

void spawnedTask ();

int get_free_td (unsigned int* free_list_lo, unsigned int* free_list_hi);

#endif
