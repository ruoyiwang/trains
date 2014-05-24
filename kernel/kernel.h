// #define SYSTEM_MODE     0xdf
// #define ACTIVE_TASK     0x1FDCFFC
// #define TDS_BEGIN       0x1FDCAFC   // 0x1FDCFFC - 4 * 5 * 0X40
//     #define TD_SIZE         0x14
//     #define TD_TID_OFFSET   0x0
//     #define TD_PC_OFFSET    0x4
//     #define TD_SP_OFFSET    0x8
//     #define TD_STATE_OFFSET 0xc
//     #define TD_NEXT_OFFSET  0x10
// #define PQS_BEGIN       0x1FDCABC   // 0x1FDCAFC - 4 * 2 * 8
//     #define PQ_SIZE         0x8
//     #define PQ_BEGIN_OFFSET 0x0
//     #define PQ_END_OFFSET   0x4
// #define TDS_FREELIST_LO        0x1FDCAB0   // 0X1FDCABC - 4 * 2, tids 0 - 31
// #define TDS_FREELIST_HI        0x1FDCAB4   // 0X1FDCABC - 4 * 2, tids 32 - 63
// #define KERNEL_SP_START 0x1FDC9B4   // 0x1FDCAB4−10×4

#define CODE_OFFSET     0x218000

#define REQUEST_CREATE 0

typedef struct td_t {
    unsigned int tid;
    unsigned int pc;
    unsigned int sp;
    unsigned int spsr;
    unsigned int ret;
    unsigned int state;
    struct td_t * next;
} td;

void first();

void SVC_HANDLER();

int initialize ( );

void testContextNOP();

int testContext();

int main( int argc, char* argv[] );

void ker_exit ( td *active, int req );

void ker_entry ();