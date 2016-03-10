#ifndef STACK_HEADER
#define STACK_HEADER

#include "LinkedList.h"

typedef LinkedList Stack;

Stack *stackCreate();

void stackPush(Stack *stack, void *item);

void *stackPop(Stack *stack);

void *stackPeek(Stack *stack);

#endif
