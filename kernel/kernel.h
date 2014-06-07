#ifndef __KERNEL__
#define __KERNEL__

#define CODE_OFFSET         0x218000

#define REQUEST_CREATE 	    2

#define NULL                0

#define STATE_READY         0
#define STATE_ACTIVE        1
#define STATE_ZOMBIE        2
#define STATE_RCV_BLK       3
#define STATE_SND_BLK       4
#define STATE_RPL_BLK       5

#define USER_STACK_SIZE     0x1000      // 0x1000 = 4096
#define USER_STACK_BEGIN    0x1000000

#define VIC1_BASE            0x800b0000
#define VIC2_BASE            0x800c0000
    #define VICxIRQStatus   0
    #define VICxIntEnable   0x10
    #define VICxIntEnClear  0x14
    #define VICxSoftInt  0x18
    #define VICxSoftIntClear  0x1c

typedef struct message_t
{
	char *value;
    int iValue;
	int type;
} message;

typedef struct mailbox_t {
	int *sender_tid;
	message *msg;
	int msg_len;
	message *rpl;
	int rpl_len;
    struct mailbox_t * next;
} mailbox;

typedef struct td_t {
    unsigned int tid;
    unsigned int pc;
    unsigned int sp;
    unsigned int spsr;
    unsigned int ret;
    unsigned int priority;
    unsigned int parent_tid;
    unsigned int state;
    unsigned int args[5];
    mailbox *sendQ;
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

int int_ker_exit ( td *active, int* args );

void int_ker_entry ();

void swi_stub();

int Create( int priority, void (* pc) ());

int MyTid();

int MyParentTid();

int Pass();

int Exit();

int Send ( int tid, char *msg, int msglen, char * reply, int replylen );

int Receive ( int *tid, char *msg, int msglen );

int Reply ( int tid, char *msg, int msglen );

int get_free_td (unsigned int* free_list_lo, unsigned int* free_list_hi);

#endif
