#include <stdlib.h>
#include "LinkedList.h"
#include "Queue.h"

Queue *queueCreate()
{
  return linkedListCreate();
}

void queueEnqueue(Queue *queue, void *item)
{
  linkedListPush(queue, item);
}

void *queueDequeue(Queue *queue)
{
  return linkedListShift(queue);
}
