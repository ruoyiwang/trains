#include <posintlist.h>

#include <interface.h>

// -1 makes the slot "free"
int posintlistInit(int list[], int array_len) {
    int i = 0;
    for ( i = 0; i < array_len; i++) {
        list[i] = -1;
    }
    return true;
}

int posintlistIsInList(int needle, int haystack[], int array_len) {
    int i = 0;
    for (i = 0; i < array_len; i++) {
        if (haystack[i] == -1) {
            return false;
        }
        if (needle == haystack[i]) {
            return true;
        }
    }
    return false;
}

int posintlistInsert(int value, int list[], int array_len) {
    int i = 0;
    for ( i = 0; i < array_len; i++) {
        if (list[i] == -1) {
            list[i] = value;
            return true;
        }
    }

    // insersion failed
    return false;
}

int posintlistErase(int value, int list[], int array_len) {
    int i = 0;
    for ( i = 0; i < array_len; i++) {
        if (list[i] == value) {
            list[i] = -1;
            return true;
        }
    }

    // insersion failed
    return false;
}

int posintlistIsEmpty(int list[], int array_len) {
    int i = 0;
    for ( i = 0; i < array_len; i++) {
        if (list[i] != -1) {
            return false;
        }
    }
    return true;
}

int posintlistFindMin(int list[], int array_len) {
    int i = 0, min = -1;
    for (i = 0; i < array_len; i++) {
        min = min < list[i] ? min : list[i];
    }
    return min;
}
