#include <stdlib.h>
#include <stdio.h>
#include "Nfa.h"
#include "SubsetConstruction.h"
#include "Hopcroft.h"
#include "Scanner.h"
#include <string.h>

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
    printf("%i -> %i[label=\"%c\"]\n", state->id, state->out1->id, state->outChar1 == '\0' ? ' ' : state->outChar1);
    printState(state->out1, printerState);
  }

  if(state->out2 != NULL)
  {
    printf("%i -> %i[label=\"%c\"]\n", state->id, state->out2->id, state->outChar2 == '\0' ? ' ' : state->outChar2);
    printState(state->out2, printerState);
  }
}

void printNFA(NFA *nfa, int stateSize)
{
  NFAState *state = nfa->start;
  PrinterState *printerState = malloc(sizeof(PrinterState));
  printerState->usedIds = malloc(sizeof(int) * (stateSize - 1));
  printerState->position = 0;

  printf("digraph {\n");
  printState(state, printerState);
  printf("}\n");
}

void printDFAState(DFAState *dfaState, PrinterState *printerState)
{
  for(int i = 0; i < printerState->position; i++)
  {
    if(printerState->usedIds[i] == dfaState->id) return;
  }

  printerState->usedIds[printerState->position++] = dfaState->id;

  if(dfaState->categoryId != -1)
  {
    printf("%i [shape = doublecircle]\n", dfaState->id);
    printf("%i [label=\"%i (C: %i)\"]\n", dfaState->id, dfaState->id, dfaState->categoryId);
  }

  for(int i = 0; i < dfaState->usedTransitions; i++)
  {
    DFATransition *trans = dfaState->transitions[i];

    printf("%i -> %i[label=\"%c", dfaState->id, trans->toState->id, trans->characters[0]);
    for(int i = 1; i < strlen(trans->characters); i++)
    {
      printf(", %c", trans->characters[i]);
    }
    printf("\"]\n");

    printDFAState(trans->toState, printerState);
  }
}

void printDFA(DFA *dfa)
{
  PrinterState *state = malloc(sizeof(PrinterState));
  state->usedIds = malloc(sizeof(int) * dfa->stateSize);
  state->position = 0;

  printf("digraph {\n");
  printDFAState(dfa->start, state);
  printf("}\n");
}

int main(int argc, char **argv)
{
  ScannerConfig *config = createScannerConfig(3);
  addCategory(config, "number", "(-|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
  addCategory(config, "*", "\\*");
  addCategory(config, "/", "/");
  addCategory(config, "+", "+");
  addCategory(config, "-", "-");
  addCategory(config, " ", "( |\n)( |\n)*");

  Scanner *scanner = createScanner(config, "123 + 438 * -44");

  while(hasMoreWords(scanner))
  {
    Word word = nextWord(scanner);

    if(word.category == NULL)
    {
      printf("Could not get category for input\n");
      break;
    }

    printf("'%s' Category: %s\n", word.lexeme, word.category->name);
  }

  return 0;
}
