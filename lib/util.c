#include <util.h>

char * strcpy ( char * destination, const char * source ) {
	char tempChar;
	while ( (*(destination++) = *(source++)) ) { }
	return destination;
}
