#include "Printer.h"
#include <stdio.h>

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
  hashMapFor(grammar->productions, cur)
  {
    ArrayList *production = hashMapForItem(cur);
    printf("%s\n", (char *)hashMapForKey(cur));

    for(int j = 0; j < arrayListCount(production); j++)
    {
      Rule *rule = arrayListGet(production, j);
      printf(" |");

      if(arrayListGet(rule->symbols, 0) == EMPTY)
      {
        printf("\n");
        continue;
      }

      for(int k = 0; k < arrayListCount(rule->symbols); k++)
      {
        printf(" %s", (char *)arrayListGet(rule->symbols, k));
      }
      printf("\n");
    }
  }
  hashMapForEnd
}

void printLR1Item(LR1Item *item)
{
  printf("[%s -> ", item->rule->productionName);

  for(int j = 0; j < arrayListCount(item->rule->symbols); j++)
  {
    if(j == item->dotPosition) printf("\u2022");
    printf("%s ", (char *)arrayListGet(item->rule->symbols, j));
  }

  if(item->dotPosition == arrayListCount(item->rule->symbols)) printf("\u2022");

  printf(", %s]\n", item->lookahead);
}

void printLR1ItemList(LR1ItemList *list)
{
  printf("----CC%i----\n", list->number);
  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    printLR1Item(arrayListGet(list->items, i));
  }
}

void printParseTreeItem(ParseTreeItem *item)
{
  if(item->type == TERMINAL)
  {
    printf("\"%p\" [label=\"%s\"]\n", item, item->word->lexeme);
  }
  else if(item->type == NONTERMINAL)
  {
    printf("\"%p\" [label=\"%s\"]\n", item, item->rule->productionName);

    for(int i = 0; i < arrayListCount(item->subItems); i++)
    {
      ParseTreeItem *subItem = arrayListGet(item->subItems, i);
      printf("\"%p\" -> \"%p\"\n", item, subItem);

      printParseTreeItem(subItem);
    }
  }
}

void printParserTable(ScannerConfig *config, Grammar *grammar, ParserTable *table)
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
      if(action->type == REDUCE) printf("r %s %i", action->toRule->productionName, action->toRule->symbols->used);
      if(action->type == ACCEPT) printf("acc");
      printf(",");
    }
    hashMapForEnd

    printf(" || ");
    hashMapFor(state->gotos, cur)
    {
      GoTo *goTo = hashMapForItem(cur);
      printf("%s -> %i, ", goTo->productionName, goTo->number);
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

void printASTNode(ASTNode *node)
{
  printf("\"%p\" [label=\"", node);
  if(node->word != NULL)
  {
     printf(" %s ~ %s", str_replace(node->word->lexeme, "\"", "\\\""), node->word->category->name);
  }
  printf(" : ");
  if(node->rule != NULL)
  {
    printf("%s", node->rule->productionName);
  }
  printf("\"]\n");

  for(int i = 0; i < arrayListCount(node->subNodes); i++)
  {
    ASTNode *subNode = arrayListGet(node->subNodes, i);
    printf("\"%p\" -> \"%p\"\n", node, subNode);

    printASTNode(subNode);
  }
}

void printAST(ParseTree *tree)
{
  printf("digraph {\n");
  printASTNode(tree->root->astNode);
  printf("}\n");
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  int len_rep;  // length of rep (the string to remove)
  int len_with; // length of with (the string to replace rep with)
  int len_front; // distance between rep and end of last rep
  int count;    // number of replacements

  // sanity checks and initialization
  if (!orig || !rep)
      return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
      return NULL; // empty rep causes infinite loop during count
  if (!with)
      with = "";
  len_with = strlen(with);

  // count the number of replacements needed
  ins = orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count) {
      ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
      return NULL;

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  while (count--) {
      ins = strstr(orig, rep);
      len_front = ins - orig;
      tmp = strncpy(tmp, orig, len_front) + len_front;
      tmp = strcpy(tmp, with) + len_with;
      orig += len_front + len_rep; // move to next "end of rep"
  }
  strcpy(tmp, orig);
  return result;
}