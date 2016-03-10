#include <stdlib.h>
#include "LinkedList.h"

LinkedList *linkedListCreate()
{
  return calloc(1, sizeof(LinkedList));
}

LinkedListNode *createNode(void *item)
{
  LinkedListNode *node = calloc(1, sizeof(LinkedListNode));
  node->item = item;
  return node;
}

void linkedListPush(LinkedList *list, void *item)
{
  LinkedListNode *node = createNode(item);

  if(list->end == NULL)
  {
    list->start = node;
    list->end = node;
  }
  else
  {
    list->end->next = node;
    node->prev = list->end;
    list->end = node;
  }

  list->count++;
}

void *linkedListPop(LinkedList *list)
{
  if(list->end == NULL) return NULL;

  LinkedListNode *lastItem = list->end;

  if(list->start == lastItem)
  {
    list->start = NULL;
    list->end = NULL;
  }
  else
  {
    list->end = lastItem->prev;
    list->end->next = NULL;
  }

  void *item = lastItem->item;
  free(lastItem);

  list->count--;

  return item;
}

void linkedListUnshift(LinkedList *list, void *item)
{
  LinkedListNode *node = createNode(item);

  if(list->start == NULL)
  {
    list->start = node;
    list->end = node;
  }
  else
  {
    node->next = list->start;
    list->start->prev = node;
    list->start = node;
  }

  list->count++;
}

void *linkedListShift(LinkedList *list)
{
  if(list->start == NULL) return NULL;

  LinkedListNode *firstItem = list->start;
  void *item = firstItem->item;

  if(list->end == firstItem)
  {
    list->start = NULL;
    list->end = NULL;
  }
  else
  {
    list->start = firstItem->next;
    list->start->prev = NULL;
  }

  list->count--;
  free(firstItem);

  return item;
}

void linkedListRemove(LinkedList *list, LinkedListNode *node)
{
  if(node->prev == NULL)
  {
    linkedListShift(list);
  }
  else if(node->next == NULL)
  {
    linkedListPop(list);
  }
  else
  {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
  }
}

void linkedListFree(LinkedList *list)
{
  LIST_FOREACH(list, node)
  {
    free(node);
  }

  free(list);
}
