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

void pushToArrayList(ArrayList *list, void *item);

void *popFromArrayList(ArrayList *list);

bool arrayListEmpty(ArrayList *list);

static inline void *arrayListGet(ArrayList *list, int i)
{
  return list->items[i];
}

static inline void arrayListSet(ArrayList *list, int i, void *item)
{
  list->items[i] = item;
}

#endif
