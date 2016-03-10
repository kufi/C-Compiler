#ifndef QUEUE_HEADER
#define QUEUE_HEADER

#include "LinkedList.h"

typedef LinkedList Queue;

Queue *queueCreate();

void queueEnqueue(Queue *queue, void *item);

void *queueDequeue(Queue *queue);

#endif
