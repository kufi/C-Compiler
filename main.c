#include <stdlib.h>
#include <stdio.h>
#include "Nfa.h"
#include "Dfa.h"

typedef struct PrinterState {
  int position;
  int *usedIds;
} PrinterState;

void printState(NFAState *state, PrinterState *printerState)
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
  NFAState *state = nfa->start;
  PrinterState *printerState = malloc(sizeof(PrinterState));
  printerState->usedIds = malloc(sizeof(int) * (nfa->final->id - 1));
  printerState->position = 0;

  printf("digraph {\n");
  printState(state, printerState);
  printf("}\n");
}

void printDFA(DFA *dfa)
{
  printf("digraph {\n");
  for(int i = 0; i < dfa->numberOfStates; i++)
  {
    for(int j = 0; j < dfa->numberOfStates; j++)
    {
      DFATransition *transition = dfa->transitions[i][j];

      if(transition != NULL)
      {
        printf("%i -> %i[label=\"", i, j);
        for(int c = 0; c < transition->characterSize; c++)
        {
          printf("%c", transition->characters[c]);
        }
        printf("\"]\n");
      }
    }
  }
  printf("}\n");
}

int main(int argc, char **argv)
{
  NFA *nfa = buildNFA("a(b|c)*");
  //printNFA(nfa);

  char characterSet[3] = {'a', 'b', 'c'};

  DFA *dfa = subsetConstruction(nfa, characterSet, 3);

  printDFA(dfa);

  return 0;
}
