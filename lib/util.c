#include <util.h>
#include <bwio.h>
#include <kernel.h>
#include <ts7200.h>
// #include <cstdio>

char * strcpy ( char * destination, const char * source ) {
    while ( (*(destination++) = *(source++)) ) { }
    return destination;
}

int strlen ( const char * str ) {
    int length = 0;
    while ( (*(str++)) ) {
        length++;
    }
    return length;
}

int strcmp ( const char * str1, const char * str2 ) {
    int i = 0;
    for ( i = 0; str1[i] || str2[i]; i++ ) {
        if (str1[i] != str2[i]) {
            return -1;
        }
    }
    return 0;
}

// this works, maybe? Bill says we need to write unit tests
void * memcpy ( void * destination, const void * source, unsigned int num ) {
    unsigned char * dest = (unsigned char*) destination;
    unsigned char * src = (unsigned char*) source;
    while (num-- > 0){
        *(dest++) = *(src++);
    }
    return destination;
}

void *memset(void *s, int c, unsigned int n) {
  unsigned char *p = s;
  while(n --> 0) { *p++ = (unsigned char)c; }
  return s;
}

void initTimers() {
    // clockworks
    // init load and control
    unsigned int *timer_4_enable_hi;
    timer_4_enable_hi = (unsigned int *) ( TIMER4_ENABLE_HI );

    //set the control bits
    unsigned int buf;
    buf = *timer_4_enable_hi;
    buf = buf | TIMER4_ENABLE_MASK;
    *timer_4_enable_hi = buf;


    // set the load (init value)
    int *line;
    line = (int *)( TIMER3_BASE + LDR_OFFSET );
    *line = 5080;

    // set the control
    line = (int *)( TIMER3_BASE + CRTL_OFFSET );

    buf = *line;
    // set to 508k Hz mode
    buf = buf | CLKSEL_MASK;
    // enable timer
    buf = buf | ENABLE_MASK;
    // set to preload mode
    buf = buf | MODE_MASK;
    *line = buf;

}

unsigned int rand(unsigned int seed) {
    seed = (1103515245 * seed + 12345);
    return seed;
}

void TurnCacheOn() {
    asm("stmdb sp!, {r4}");
    asm("mov r4, #0");
    //invalidate cache
    asm("mcr p15, 0, r4, c7, c7, 0");
    asm("mcr p15, 0, r4, c7, c5, 0");
    asm("MCR p15, 0, R4, c8, c7, 0");

    asm("MRC p15, 0, R4, c1, c0, 0");
    asm("ORR R4, R4, #4096");
    asm("ORR R4, R4, #4");
    // asm("ORR R4, R4, #1");
    asm("MCR p15, 0, R4, c1, c0, 0");
    asm("ldmia sp!, {r4}");
}

// // for testing purposes
// int main() {
//     char strA[10] = "str A";
//     char strB[10] = "123456789";

//     memcpy(strB, strA, 3);
//     printf("%s\n", strA);
//     printf("%s\n", strB);
//     int a = strcmp(strA, strB);
//     printf("%d\n", a);
// }

int setspeed( int channel, int speed ) {
    int *high, *low;
    switch( channel ) {
    case COM1:
        high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
        low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
            break;
    case COM2:
        high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
        low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
            break;
    default:
        bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll SETSPEED1 %d", channel);
        return -1;
        break;
    }
    switch( speed ) {
    case 115200:
        *high = 0x0;
        *low = 0x3;
        return 0;
    case 2400:
        *high = 0x0;
        *low = 0xbf;
        return 0;
    default:
        bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll SETSPEED2 %d", channel);
        return -1;
    }
}

inline void uart_noops() {
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
}
