#ifndef ARRAYLIST_HEADER
#define ARRAYLIST_HEADER

#include <stdlib.h>
#include <stdbool.h>

typedef struct ArrayList {
  int used;
  int max;
  size_t itemSize;
  void **items;
} ArrayList;

ArrayList *createArrayList(int initialSize, size_t itemSize);

void addToArrayList(ArrayList *list, void *item);

bool arrayListContains(ArrayList *list, void *item);

#endif
