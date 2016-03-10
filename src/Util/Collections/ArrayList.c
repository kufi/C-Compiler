#include "ArrayList.h"

ArrayList *arrayListCreate(int initialSize, size_t itemSize)
{
  ArrayList *list = malloc(sizeof(ArrayList));
  list->used = 0;
  list->max = initialSize;
  list->itemSize = itemSize;
  list->items = calloc(initialSize, itemSize);

  return list;
}

void arrayListPush(ArrayList *list, void *item)
{
  if(list->used == list->max)
  {
    list->max = list->max * 2;
    list->items = realloc(list->items, list->itemSize * list->max);
  }

  list->items[list->used++] = item;
}

bool arrayListEmpty(ArrayList *list)
{
  return list->used == 0;
}

void *arrayListPop(ArrayList *list)
{
  return list->items[--list->used];
}

void arrayListFree(ArrayList *list)
{
  free(list);
}
