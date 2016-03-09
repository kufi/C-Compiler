#include "ArrayList.h"

ArrayList *createArrayList(int initialSize, size_t itemSize)
{
  ArrayList *list = malloc(sizeof(ArrayList));
  list->used = 0;
  list->max = initialSize;
  list->items = malloc(itemSize * list->max);

  return list;
}

void addToArrayList(ArrayList *list, void *item)
{
  if(list->used == list->max)
  {
    list->max = list->max * 2;
    list->items = realloc(list->items, list->itemSize * list->max);
  }

  list->items[list->used++] = item;
}

bool arrayListContains(ArrayList *list, void *item)
{

}
