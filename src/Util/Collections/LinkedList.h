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

#define LIST_FOREACH(L, V) LinkedListNode *_node = NULL;\
    LinkedListNode *V = NULL;\
    for(V = _node = L->start; _node != NULL; V = _node = _node->next)

LinkedList *linkedListCreate();

void linkedListPush(LinkedList *list, void *item);

void *linkedListPop(LinkedList *list);

void linkedListUnshift(LinkedList *list, void *item);

void *linkedListShift(LinkedList *list);

void linkedListFree(LinkedList *list);

void linkedListRemove(LinkedList *list, LinkedListNode *node);

static inline int linkedListCount(LinkedList *list)
{
  return list->count;
}

#endif
