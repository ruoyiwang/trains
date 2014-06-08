#include <util.h>
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
    int i = 0;
    char * dest = (char*) destination;
    char * src = (char*) source;
    for ( i = 0; i < num; i++ ) {
        dest[i] = src[i];
    }
    return destination;
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
    *line = 200;

    // set the control
    line = (int *)( TIMER3_BASE + CRTL_OFFSET );

    buf = *line;
    // set to 2k Hz mode
    buf = buf & ~CLKSEL_MASK;
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
    asm("MRC p15, 0, R4, c1, c0, 0");
    asm("ORR R4, R4, #0x800");
    asm("MCR p15, 0, R4, c1, c0, 0");
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
