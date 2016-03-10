#include <stdlib.h>
#include "Stack.h"

Stack *stackCreate()
{
  return linkedListCreate();
}

void stackPush(Stack *stack, void *item)
{
  linkedListPush(stack, item);
}

void *stackPop(Stack *stack)
{
  return linkedListPop(stack);
}

void *stackPeek(Stack *stack)
{
  return stack->end != NULL ? stack->end->item : NULL;
}
