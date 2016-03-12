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

ArrayList *arrayListCreate(int initialSize, size_t itemSize);

void arrayListPush(ArrayList *list, void *item);

void *arrayListPop(ArrayList *list);

bool arrayListEmpty(ArrayList *list);

static inline void *arrayListGet(ArrayList *list, int i)
{
  return list->items[i];
}

static inline void arrayListSet(ArrayList *list, int i, void *item)
{
  list->items[i] = item;
}

static inline int arrayListCount(ArrayList *list)
{
  return list->used;
}

void arrayListFree(ArrayList *list);

void arrayListQSort(ArrayList *list, int (*compare)(const void *a, const void *b));

#endif
