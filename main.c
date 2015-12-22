#include <stdlib.h>
#include <stdio.h>
#include "Nfa.h"

typedef struct PrinterState {
  int position;
  int *usedIds;
} PrinterState;

void printState(State *state, PrinterState *printerState)
{
  for(int i = 0; i < printerState->position; i++)
  {
    if(printerState->usedIds[i] == state->id) return;
  }

  printerState->usedIds[printerState->position++] = state->id;

  if(state->out1 != NULL)
  {
    printf("%i -> %i[label=\"%c\"]\n", state->id, state->out1->id, state->outChar1);
    printState(state->out1, printerState);
  }

  if(state->out2 != NULL)
  {
    printf("%i -> %i[label=\"%c\"]\n", state->id, state->out2->id, state->outChar2);
    printState(state->out2, printerState);
  }
}

void printNFA(NFA *nfa)
{
  State *state = nfa->start;
  PrinterState *printerState = malloc(sizeof(PrinterState));
  printerState->usedIds = malloc(sizeof(int) * (nfa->final->id - 1));
  printerState->position = 0;

  printf("digraph {\n");
  printState(state, printerState);
  printf("}\n");
}

int main(int argc, char **argv)
{
  buildNFA("abc");

  buildNFA("a*");

  buildNFA("a|b");

  NFA *finalNFA = buildNFA("a(b|c)*");
  printNFA(finalNFA);

  DFA *dfa = buildNFA(finalNFA);

  buildNFA("(1|2|3)(1|2|3)*");

  return 0;
}
