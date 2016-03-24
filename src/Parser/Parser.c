#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"
#include "Parser.h"
#include "../Scanner/Scanner.h"
#include "../Util/Collections/ArrayList.h"
#include "../Util/Collections/Queue.h"
#include "../Util/Collections/HashSet.h"
#include "../Util/Collections/HashMap.h"
#include "../Scanner/StringBuilder.h"

typedef struct LR1Item {
  char *production;
  Rule *rule;
  int dotPosition;
  char *lookahead;
} LR1Item;

typedef struct LR1ItemList {
  int number;
  ArrayList *items;
} LR1ItemList;

typedef struct FirstSet {
  char *name;
  ArrayList *terminals;
} FirstSet;

typedef struct Transition {
  ArrayList *from;
  ArrayList *to;
  char *symbol;
} Transition;

LR1Item *createLR1Item(char *name, Rule *rule, char *lookahead)
{
  LR1Item *item = calloc(1, sizeof(LR1Item));
  item->production = strdup(name);
  item->rule = rule;
  item->dotPosition = 0;
  item->lookahead = lookahead;

  return item;
}

LR1ItemList *createLR1Items(Production *production)
{
  LR1ItemList *list = calloc(1, sizeof(LR1ItemList));
  list->items = arrayListCreate(10, sizeof(LR1Item *));

  for(int i =0; i < arrayListCount(production->rules); i++)
  {
    arrayListPush(list->items, createLR1Item(production->name, arrayListGet(production->rules, i), END));
  }

  return list;
}

void addSet(HashMap *sets, FirstSet *set)
{
  hashMapSet(sets, set->name, set);
}

FirstSet *createFirstSet(char *name)
{
  FirstSet *set = calloc(1, sizeof(FirstSet));
  set->terminals = arrayListCreate(5, sizeof(char *));
  set->name = name;
  return set;
}

void addTerminal(FirstSet *set, char *terminal)
{
  arrayListPush(set->terminals, terminal);
}

FirstSet *findSet(HashSet *sets, char *name)
{
  return hashMapGet(sets, name);
}

FirstSet *copySet(FirstSet *set)
{
  FirstSet *copy = calloc(1, sizeof(FirstSet));
  *copy = *set;
  copy->terminals = arrayListCreate(arrayListCount(set->terminals), sizeof(char *));
  for(int i = 0; i < arrayListCount(set->terminals); i++)
  {
    arrayListPush(copy->terminals, strdup(arrayListGet(set->terminals, i)));
  }

  return copy;
}

bool setContainsEmpty(FirstSet *set)
{
  if(arrayListCount(set->terminals) == 0) return false;
  return arrayListGet(set->terminals, arrayListCount(set->terminals)) == EMPTY;
}

void removeEmptyFromSet(FirstSet *set)
{
  if(setContainsEmpty(set))
  {
    arrayListPop(set->terminals);
  }
}

bool setContainsTerminal(FirstSet *set, char *search)
{
  for(int i = 0; i < arrayListCount(set->terminals); i++)
  {
    char *terminal = arrayListGet(set->terminals, i);

    if(terminal == EMPTY && search == EMPTY) return true;
    if(terminal != EMPTY && strcmp(terminal, search) == 0) return true;
  }

  return false;
}

void combineSets(FirstSet *first, FirstSet *second)
{
  if(second == NULL) return;

  for(int i = 0; i < arrayListCount(second->terminals); i++)
  {
    char *terminal = arrayListGet(second->terminals, i);
    if(!setContainsTerminal(first, terminal)) addTerminal(first, terminal);
  }
}

HashMap *createFirstSets(Grammar *grammar, ScannerConfig *config)
{
  HashMap *sets = hashMapCreate(10);

  FirstSet *empty = createFirstSet(EMPTY);
  addTerminal(empty, EMPTY);
  addSet(sets, empty);

  FirstSet *end = createFirstSet(END);
  addTerminal(end, END);
  addSet(sets, end);

  for(int i = 0; i < config->categories->used; i++)
  {
    Category *cat = arrayListGet(config->categories, i);
    FirstSet *set = createFirstSet(cat->name);
    addTerminal(set, cat->name);
    addSet(sets, set);
  }

  for(int i = 0; i < arrayListCount(grammar->productions); i++)
  {
    Production *prod = arrayListGet(grammar->productions, i);
    addSet(sets, createFirstSet(prod->name));
  }

  bool changing;

  do
  {
    changing = false;

    for(int i = 0; i < arrayListCount(grammar->productions); i++)
    {
      Production *prod = arrayListGet(grammar->productions, i);

      FirstSet *prodSet = findSet(sets, prod->name);

      for(int j = 0; j < arrayListCount(prod->rules); j++)
      {
        Rule *rule = arrayListGet(prod->rules, j);

        FirstSet *rhs = copySet(findSet(sets, arrayListGet(rule->symbols, 0)));
        bool containsEmpty = setContainsEmpty(rhs);
        removeEmptyFromSet(rhs);

        int k = 1;
        for(; k < arrayListCount(rule->symbols) && containsEmpty; k++)
        {
          char *symbol = arrayListGet(rule->symbols, k);

          FirstSet *symbolSet = copySet(findSet(sets, symbol));
          containsEmpty = setContainsEmpty(symbolSet);
          removeEmptyFromSet(symbolSet);
          combineSets(rhs, symbolSet);
        }

        if(k == arrayListCount(rule->symbols) && containsEmpty)
        {
          addTerminal(rhs, EMPTY);
        }

        int oldSize = arrayListCount(prodSet->terminals);
        combineSets(prodSet, rhs);

        changing |= oldSize != arrayListCount(prodSet->terminals);
      }
    }
  } while(changing);

  return sets;
}

char *getNextSymbol(LR1Item *item)
{
  if(item->dotPosition < arrayListCount(item->rule->symbols))
  {
    return arrayListGet(item->rule->symbols, item->dotPosition);
  }

  return NULL;
}

Production *getProductionForSymbol(Grammar *grammar, char *symbol)
{
  if(symbol == NULL) return NULL;

  for(int i = 0; i < arrayListCount(grammar->productions); i++)
  {
    Production *production = arrayListGet(grammar->productions, i);
    if(strcmp(production->name, symbol) == 0) return production;
  }

  return NULL;
}

int getLookaheadSymbolsSize(LR1Item *item)
{
  return arrayListCount(item->rule->symbols) - item->dotPosition;
}

char **getLookaheadSymbols(LR1Item *item, char *lookahead)
{
  int remainingSymbols = getLookaheadSymbolsSize(item);
  char **lookaheadsSymbols = malloc(sizeof(char *) * remainingSymbols);

  int lookaheadPosition = 0;
  for(int i = item->dotPosition + 1; i < arrayListCount(item->rule->symbols); i++)
  {
    lookaheadsSymbols[lookaheadPosition++] = arrayListGet(item->rule->symbols, i);
  }

  lookaheadsSymbols[lookaheadPosition] = lookahead;

  return lookaheadsSymbols;
}

FirstSet *getFirstSetForLookaheads(char **lookaheads, int lookaheadSize, HashMap *sets)
{
  FirstSet *resultingSet = createFirstSet("");

  for(int i = 0; i < lookaheadSize; i++)
  {
    char *lookahead = lookaheads[i];

    FirstSet *set = findSet(sets, lookahead);
    combineSets(resultingSet, set);

    if(!setContainsEmpty(set)) break;
  }

  return resultingSet;
}

bool areLR1ItemsSame(LR1Item *first, LR1Item *second)
{
  if(first->dotPosition != second->dotPosition) return false;
  if(first->rule != second->rule) return false;
  if(strcmp(first->lookahead, second->lookahead) != 0) return false;
  return true;
}

bool itemListContainsItem(ArrayList *list, LR1Item *searchItem)
{
  for(int i = 0; i < arrayListCount(list); i++)
  {
    LR1Item *item = arrayListGet(list, i);

    if(areLR1ItemsSame(item, searchItem)) return true;
  }

  return false;
}

bool addLR1ItemToList(LR1ItemList *list, LR1Item *item)
{
  if(itemListContainsItem(list->items, item)) return false;
  arrayListPush(list->items, item);
  return true;
}

LR1Item *copyLR1Item(LR1Item *item)
{
  LR1Item *newItem = malloc(sizeof(LR1Item));
  newItem->production = strdup(item->production);
  newItem->dotPosition = item->dotPosition;
  newItem->rule = item->rule;
  newItem->lookahead = strdup(item->lookahead);
  return newItem;
}

int nextItemListNumber()
{
  static int number = 0;
  return number++;
}

LR1ItemList *copyLR1ItemList(LR1ItemList *list)
{
  LR1ItemList *newList = calloc(1, sizeof(LR1ItemList));
  newList->items = arrayListCreate(arrayListCount(list->items), sizeof(LR1Item *));

  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    arrayListPush(newList->items, copyLR1Item(arrayListGet(list->items, i)));
  }

  return newList;
}

LR1ItemList *closure(LR1ItemList *list, Grammar *grammar, HashMap *sets)
{
  LR1ItemList *newList = copyLR1ItemList(list);
  Queue *worklist = queueCreate();

  for(int i = 0; i < arrayListCount(newList->items); i++) queueEnqueue(worklist, arrayListGet(newList->items, i));

  LR1Item *item;
  while((item = queueDequeue(worklist)) != NULL)
  {
    Production *production = getProductionForSymbol(grammar, getNextSymbol(item));

    if(production == NULL) continue;

    FirstSet *set = getFirstSetForLookaheads(getLookaheadSymbols(item, item->lookahead), getLookaheadSymbolsSize(item), sets);

    for(int j = 0; j < arrayListCount(production->rules); j++)
    {
      Rule *rule = arrayListGet(production->rules, j);

      for(int k = 0; k < arrayListCount(set->terminals); k++)
      {
        LR1Item *ruleItem = createLR1Item(production->name, rule, arrayListGet(set->terminals, k));
        if(addLR1ItemToList(newList, ruleItem)) queueEnqueue(worklist, ruleItem);
      }
    }
  }

  return newList;
}

char *getDotSymbol(LR1Item *item)
{
  if(item->dotPosition == arrayListCount(item->rule->symbols)) return NULL;

  return arrayListGet(item->rule->symbols, item->dotPosition);
}

LR1ItemList *goTo(LR1ItemList *list, char *symbol, Grammar *grammar, HashMap *firstSets)
{
  LR1ItemList *moved = calloc(1, sizeof(LR1ItemList));
  moved->items = arrayListCreate(10, sizeof(LR1Item *));

  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    LR1Item *item = arrayListGet(list->items, i);
    char *nextSymbol = getDotSymbol(item);

    if(nextSymbol != NULL && strcmp(nextSymbol, symbol) == 0)
    {
      LR1Item *movedDotItem = copyLR1Item(item);
      movedDotItem->dotPosition++;
      addLR1ItemToList(moved, movedDotItem);
    }
  }

  return closure(moved, grammar, firstSets);
}

void printLR1ItemList(LR1ItemList *list)
{
  printf("----CC%i----\n", list->number);
  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    LR1Item *item = arrayListGet(list->items, i);

    printf("[%s -> ", item->production);

    for(int j = 0; j < arrayListCount(item->rule->symbols); j++)
    {
      if(j == item->dotPosition) printf("\u2022");
      printf("%s ", (char *)arrayListGet(item->rule->symbols, j));
    }

    if(item->dotPosition == arrayListCount(item->rule->symbols)) printf("\u2022");

    printf(", %s]\n", item->lookahead);
  }
}

bool containsSymbol(ArrayList *symbols, char *symbol)
{
  for(int i = 0; i < symbols->used; i++)
  {
    if(strcmp(symbols->items[i], symbol) == 0) return true;
  }

  return false;
}

ArrayList *getSymbolsFollowingDot(LR1ItemList *list)
{
  ArrayList *following = arrayListCreate(10, sizeof(char *));

  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    LR1Item *item = arrayListGet(list->items, i);
    char *symbol = getDotSymbol(item);

    if(symbol != NULL && !containsSymbol(following, symbol))
    {
      arrayListPush(following, symbol);
    }
  }

  return following;
}

bool areLR1ItemListSame(LR1ItemList *first, LR1ItemList *second)
{
  if(arrayListCount(first->items) != arrayListCount(second->items)) return false;

  for(int i = 0; i < arrayListCount(first->items); i++)
  {
    if(!itemListContainsItem(second->items, arrayListGet(first->items, i))) return false;
  }

  return true;
}

LR1ItemList *tryGetExistingItemList(HashSet *collection, LR1ItemList *search)
{
  return hashSetGetExisting(collection, search);
}

bool ccCompare(void *a, void *b)
{
  return areLR1ItemListSame(a, b);
}

char *createStringFromLR1Item(LR1Item *item)
{
  StringBuilder s = stringBuilderCreateFull(50);

  appendChars(&s, item->production);
  appendChars(&s, item->lookahead);

  char buffer[12];
  snprintf(buffer, 12, "%i", item->dotPosition);
  appendChars(&s, buffer);

  for(int i = 0; i < arrayListCount(item->rule->symbols); i++)
  {
    appendChars(&s, arrayListGet(item->rule->symbols, i));
  }

  return s.string;
}

uint32_t ccHash(void *hashable)
{
  LR1ItemList *list = (LR1ItemList *)hashable;
  StringBuilder builder = createStringBuilder();

  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    LR1Item *item = arrayListGet(list->items, i);
    appendChars(&builder, createStringFromLR1Item(item));
  }

  char *key = builder.string;
  size_t len = builder.used;
  uint32_t hash = 0;
  uint32_t i = 0;

  for(hash = i = 0; i < len; ++i)
  {
      hash += key[i];
      hash += (hash << 10);
      hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}

int itemListCompare(const void *a, const void *b)
{
  LR1Item *first = *(void **)a;
  LR1Item *second = *(void **)b;
  return strcmp(createStringFromLR1Item(first), createStringFromLR1Item(second));
}

enum ActionType { SHIFT, REDUCE, ACCEPT };

typedef struct Action {
  enum ActionType type;
  char *symbol;
  union {
    int toState;
    Production *toProduction;
  };
} Action;

typedef struct GoTo {
  int number;
} GoTo;

typedef struct ParseState {
  int number;
  ArrayList *actions;
  ArrayList *gotos;
} ParseState;

typedef struct ParserTable {
  ArrayList *states;
} ParserTable;

static uint32_t transitionsHash(void *a)
{
  LR1ItemList *list = a;
  return list->number;
}

bool transitionsCompare(void *a, void *b)
{
  HashMap *first = a;
  HashMap *second = b;
  return first == second;
}

Parser *createParser(Grammar *grammar, ScannerConfig *config)
{
  Production *goalProduction = arrayListGet(grammar->productions, 0);

  HashMap *sets = createFirstSets(grammar, config);
  LR1ItemList *cc0 = closure(createLR1Items(goalProduction), grammar, sets);
  cc0->number = nextItemListNumber();

  HashSet *cc = hashSetCreateFull(0, 0.0f, ccCompare, ccHash);
  hashSetPut(cc, cc0);

  Queue *worklist = queueCreate();
  queueEnqueue(worklist, cc0);

  HashMap *transitions = hashMapCreateFull(0, 0.0f, transitionsCompare, transitionsHash);

  LR1ItemList *itemList = NULL;

  while((itemList = queueDequeue(worklist)) != NULL)
  {
    ArrayList *following = getSymbolsFollowingDot(itemList);

    for(int i = 0; i < following->used; i++)
    {
      char *symbol = arrayListGet(following, i);
      LR1ItemList *temp = goTo(itemList, symbol, grammar, sets);
      arrayListQSort(temp->items, itemListCompare);
      LR1ItemList *existing = tryGetExistingItemList(cc, temp);

      if(existing == NULL)
      {
        hashSetPut(cc, temp);
        queueEnqueue(worklist, temp);
        existing = temp;
        temp->number = nextItemListNumber();
      }

      HashMap *transitionsForItemList = hashMapGet(transitions, itemList);
      if(transitionsForItemList == NULL) transitionsForItemList = hashMapCreate();
      hashMapSet(transitionsForItemList, symbol, existing);
      hashMapSet(transitions, itemList, transitionsForItemList);
    }
  }

  ParserTable *table = calloc(1, sizeof(ParserTable));
  table->states = arrayListCreate(cc->count, sizeof(ParseState *));

  int stateNumber = 0;
  hashSetFor(cc, cur)
  {
    LR1ItemList *list = hashSetForItem(cur);

    ParseState *state = calloc(1, sizeof(ParseState));
    state->number = stateNumber++;
    state->actions = arrayListCreate(arrayListCount(list->items), sizeof(Action *));
    state->gotos = arrayListCreate(arrayListCount(list->items), sizeof(GoTo *));

    for(int i = 0; i < arrayListCount(list->items); i++)
    {
      LR1Item *item = arrayListGet(list->items, i);
      bool isAtEnd = item->dotPosition == arrayListCount(item->rule->symbols);

      if(isAtEnd)
      {
        Action *action = calloc(1, sizeof(Action));
        action->symbol = item->lookahead;

        if(strcmp(item->production, goalProduction->name) == 0 && item->lookahead == END)
        {
          action->type = ACCEPT;
        }
        else
        {
          action->type = REDUCE;
          action->toProduction = getProductionForSymbol(grammar, item->production);
        }

        arrayListPush(state->actions, action);
        continue;
      }

      char *nextSymbol = getDotSymbol(item);
      Production *nextProduction = nextSymbol != NULL ? getProductionForSymbol(grammar, nextSymbol) : NULL;

      //nextSymbol is a terminal symbol
      if(nextProduction == NULL)
      {
        HashMap *transitionsForList = hashMapGet(transitions, list);
        LR1ItemList *toTransition = transitionsForList != NULL ? hashMapGet(transitionsForList, nextSymbol) : NULL;

        if(toTransition != NULL)
        {
          Action *shiftAction = calloc(1, sizeof(Action));
          shiftAction->type = SHIFT;
          shiftAction->toState = toTransition->number;
          shiftAction->symbol = nextSymbol;
          arrayListPush(state->actions, shiftAction);
        }
      }
    }

    HashMap *transitionsForList = hashMapGet(transitions, list);

    for(int i = 0; i < arrayListCount(grammar->productions); i++)
    {
      Production *production = arrayListGet(grammar->productions, i);
      LR1ItemList *nonTerminalTransition = transitionsForList != NULL ? hashMapGet(transitionsForList, production->name) : NULL;
      if(nonTerminalTransition != NULL)
      {
        GoTo *goTo = calloc(1, sizeof(GoTo));
        goTo->number = nonTerminalTransition->number;
        arrayListPush(state->gotos, goTo);
      }
    }

    arrayListPush(table->states, state);
  }
  hashSetForEnd

  for(int i = 0; i < arrayListCount(table->states); i++)
  {
    ParseState *state = arrayListGet(table->states, i);
    printf("%i: ", state->number);

    for(int j = 0; j < arrayListCount(state->actions); j++)
    {
      Action *action = arrayListGet(state->actions, j);
      printf("%s -> ", action->symbol);

      if(action->type == SHIFT) printf("s %i", action->toState);
      if(action->type == REDUCE) printf("r %s", action->toProduction->name);
      if(action->type == ACCEPT) printf("acc");
      printf(",");
    }
    printf("\n");
  }

  return NULL;
}
