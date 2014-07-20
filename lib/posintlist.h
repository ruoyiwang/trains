#ifndef __POSINTLIST__
#define __POSINTLIST__

int posintlistInit(int list[], int array_len);

int posintlistIsEmpty(int list[], int array_len);

int posintlistIsInList(int needle, int haystack[], int array_len);

int posintlistInsert(int value, int list[], int array_len);

int posintlistErase(int value, int list[], int array_len);

int posintlistFindMin(int list[], int array_len);

#endif

