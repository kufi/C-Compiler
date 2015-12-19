#include <stdlib.h>
#include <stdio.h>
#include "Nfa.h"

typedef struct PrintState {
  int position;
  int *usedIds;
} PrintState;

void printNFA(State *state, PrintState *printState)
{
  for(int i = 0; i < printState->position; i++)
  {
    if(printState->usedIds[i] == state->id) return;
  }

  printState->usedIds[printState->position++] = state->id;

  if(state->out1 != NULL)
  {
    printf("%i -> %i[label=\"%c\"]\n", state->id, state->out1->id, state->outChar1);
    printNFA(state->out1, printState);
  }

  if(state->out2 != NULL)
  {
    printf("%i -> %i[label=\"%c\"]\n", state->id, state->out2->id, state->outChar2);
    printNFA(state->out2, printState);
  }
}

int main(int argc, char **argv)
{
  createNFA("abc");

  createNFA("a*");

  createNFA("a|b");

  NFA *finalNFA = createNFA("a(b|c)*");

  createNFA("(1|2|3)(1|2|3)*");

  State *state = finalNFA->start;
  PrintState *printState = malloc(sizeof(PrintState));
  printState->usedIds = malloc(sizeof(int) * (finalNFA->final->id - 1));
  printState->position = 0;

  printf("digraph {\n");
  printNFA(state, printState);
  printf("}\n");

  return 0;
}
