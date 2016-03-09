#ifndef QUEUE_HEADER
#define QUEUE_HEADER

#include "LinkedList.h"

typedef struct Queue {
  LinkedList *list;
} Queue;

Queue *createQueue();

void pushQueue(Queue *queue, void *item);

void *popQueue(Queue *queue);

#endif
