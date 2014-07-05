#ifndef __UTIL__
#define __UTIL__

char * strcpy ( char * destination, const char * source );

int strlen ( const char * str );

int strcmp ( const char * str1, const char * str2 );

void * memcpy ( void * destination, const void * source, unsigned int num );

void *memset(void *s, int c, unsigned int n);

void initTimers();

unsigned int rand(unsigned int seed);

void TurnCacheOn();

int setspeed( int channel, int speed );

inline void uart_noops();
#endif
