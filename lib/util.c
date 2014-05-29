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
