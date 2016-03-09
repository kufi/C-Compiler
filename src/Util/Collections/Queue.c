#include <stdlib.h>
#include "LinkedList.h"
#include "Queue.h"

Queue *createQueue()
{
  Queue *queue = malloc(sizeof(Queue));
  queue->list = createLinkedList();

  return queue;
}

void pushQueue(Queue *queue, void *item)
{
  pushList(queue->list, item);
}

void *popQueue(Queue *queue)
{
  return shiftList(queue->list);
}
