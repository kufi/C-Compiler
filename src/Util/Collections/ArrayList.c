#include "ArrayList.h"

ArrayList *createArrayList(int initialSize, size_t itemSize)
{
  ArrayList *list = malloc(sizeof(ArrayList));
  list->used = 0;
  list->max = initialSize;
  list->itemSize = itemSize;
  list->items = malloc(itemSize * list->max);

  return list;
}

void pushToArrayList(ArrayList *list, void *item)
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

void *popFromArrayList(ArrayList *list)
{
  return list->items[--list->used];
}
