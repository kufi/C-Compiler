#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

typedef struct LinkedListNode {
  struct LinkedListNode *prev;
  struct LinkedListNode *next;
  void *item;
} LinkedListNode;

typedef struct LinkedList {
  int count;
  LinkedListNode *start;
  LinkedListNode *end;
} LinkedList;

LinkedList *createLinkedList();

void pushList(LinkedList *list, void *item);

void *popList(LinkedList *list);

void unshiftList(LinkedList *list, void *item);

void *shiftList(LinkedList *list);

static inline int countList(LinkedList *list)
{
  return list->count;
}

#endif
