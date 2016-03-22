#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Scanner/Nfa.h"
#include "Scanner/SubsetConstruction.h"
#include "Scanner/Hopcroft.h"
#include "Scanner/Scanner.h"
#include "Parser/Grammar.h"
#include "Parser/Parser.h"
#include "Util/Collections/HashMap.h"
#include "Util/Collections/HashSet.h"

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

  for(int i = 0; i < dfaState->transitions->used; i++)
  {
    DFATransition *trans = arrayListGet(dfaState->transitions, i);

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
  state->usedIds = malloc(sizeof(int) * arrayListCount(dfa->states));
  state->position = 0;

  printf("digraph {\n");
  printDFAState(dfa->start, state);
  printf("}\n");
}

void printGrammar(Grammar *grammar)
{
  for(int i = 0; i < arrayListCount(grammar->productions); i++)
  {
    Production *production = arrayListGet(grammar->productions, i);
    printf("%s\n", production->name);
    for(int j = 0; j < arrayListCount(production->rules); j++)
    {
      Rule *rule = arrayListGet(production->rules, j);
      printf(" |");

      if(arrayListGet(rule->symbols, 0) == EMPTY)
      {
        printf("\n");
        continue;
      }
      for(int k = 0; k < arrayListCount(rule->symbols); k++)
      {
        printf(" %s", arrayListGet(rule->symbols, k));
      }
      printf("\n");
    }
  }
}

int main(int argc, char **argv)
{
  ScannerConfig *config = createScannerConfig(3);
  /*addCategory(config, "number", "(-|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
  addCategory(config, "*", "\\*");
  addCategory(config, "/", "/");
  addCategory(config, "+", "+");
  addCategory(config, "-", "-");
  addCategory(config, " ", "( |\n)( |\n)*");*/

  addCategory(config, "(", "\\(");
  addCategory(config, ")", "\\)");

  /*addCategory(config, "num", "num");
  addCategory(config, "name", "name");
  addCategory(config, "*", "\\*");
  addCategory(config, "/", "/");
  addCategory(config, "+", "+");
  addCategory(config, "-", "-");
  addCategory(config, "(", "\\(");
  addCategory(config, ")", "\\)");*/

  Scanner *scanner = createScanner(config, "(())()(((())))");

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

  Grammar *grammar = createGrammar();

  addProduction(grammar, "Goal", "List", END);
  addProduction(grammar, "List", "List Pair", "Pair", END);
  addProduction(grammar, "Pair", "( Pair )", "( )", END);
  /*addProduction(grammar, "Expr", "Term Expr2", END);
  addProduction(grammar, "Expr2", "+ Term Expr2", "- Term Expr2", EMPTY, END);
  addProduction(grammar, "Term", "Factor Term2", END);
  addProduction(grammar, "Term2", "* Factor Term2", "/ Factor Term2", EMPTY, END);
  addProduction(grammar, "Factor", "( Expr )", "num", "name", END);*/
  printGrammar(grammar);

  createParser(grammar, config);

  return 0;
}
