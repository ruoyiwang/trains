#include <util.h>
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

// for testing purposes
// int main() {
// 	char strA[10] = "str A";
// 	char strB[10];
// 	strcpy(strB, strA);
// 	printf("%s\n", strA);
// 	printf("%s\n", strB);
// 	int a = strlen(strB);
// 	printf("%d\n", a);
// }
