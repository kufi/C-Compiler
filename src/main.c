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

void printParseTreeItem(ParseTreeItem *item)
{
  if(item->type == TERMINAL)
  {
    printf("%i [label=\"%s\"]\n", (int)item, item->word->lexeme);
  }
  else if(item->type == NONTERMINAL)
  {
    printf("%i [label=\"%s\"]\n", (int)item, item->production->name);

    LIST_FOREACH(item->subItems, cur)
    {
      ParseTreeItem *subItem = cur->item;
      printf("%i -> %i\n", (int)item, (int)subItem);

      printParseTreeItem(subItem);
    }
  }
}

void printParserTable(ParserTable *table)
{
  for(int i = 0; i < arrayListCount(table->states); i++)
  {
    ParseState *state = arrayListGet(table->states, i);
    printf("%i: ", state->number);

    hashMapFor(state->actions, cur)
    {
      Action *action = hashMapForItem(cur);
      printf("%s -> ", action->symbol);

      if(action->type == SHIFT) printf("s %i", action->toState);
      if(action->type == REDUCE) printf("r %s", action->toProduction->name);
      if(action->type == ACCEPT) printf("acc");
      printf(",");
    }
    hashMapForEnd

    printf(" || ");
    hashMapFor(state->gotos, cur)
    {
      GoTo *goTo = hashMapForItem(cur);
      printf("%s -> %i, ", goTo->production->name, goTo->number);
    }
    hashMapForEnd

    printf("\n");
  }
}

void printParseTree(ParseTree *tree)
{
  printf("digraph {\n");
  printParseTreeItem(tree->root);
  printf("}\n");
}

int main(int argc, char **argv)
{
  ScannerConfig *config = createScannerConfig(3);
  /*addCategory(config, "num", "(-|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
  addCategory(config, "name", "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)*");
  addCategory(config, "*", "\\*");
  addCategory(config, "/", "/");
  addCategory(config, "+", "+");
  addCategory(config, "-", "-");*/

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

  Grammar *grammar = createGrammar();

  addProduction(grammar, "Goal", "List", END);
  addProduction(grammar, "List", "List Pair", "Pair", END);
  addProduction(grammar, "Pair", "( List )", "( Pair )", "( )", END);

  /*addProduction(grammar, "Goal", "Expr", END);
  addProduction(grammar, "Expr", "Expr + Term", "Expr - Term", "Term", END);
  addProduction(grammar, "Term", "Term * Factor", "Term / Factor", "Factor", END);
  addProduction(grammar, "Factor", "( Expr )", "num", "name", END);*/

  printGrammar(grammar);

  ParserTable *table = createParser(grammar, config);
  //ParseTree *tree = runParser(table, config, "(1 + 2) * ( 3 + 4)");
  ParseTree *tree = runParser(table, config, "(()((())()))");

  if(tree->success)
  {
    printParseTree(tree);
  }

  return 0;
}
