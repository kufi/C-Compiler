#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"
#include "Parser.h"
#include "../Scanner/Scanner.h"

typedef struct LR1Item {
  char *production;
  int dotPosition;
  int symbolSize;
  char **symbols;
  char *lookahead;
} LR1Item;

typedef struct LR1ItemList {
  int usedItems;
  int itemSize;
  LR1Item **items;
} LR1ItemList;

typedef struct FirstSet {
  char *name;
  int usedTerminals;
  int terminalSize;
  char **terminals;
} FirstSet;

typedef struct FirstSets {
  int usedSets;
  int setSize;
  FirstSet **sets;
} FirstSets;

typedef struct LR1ItemWorklist {
  LR1Item **items;
  int size;
  int maxSize;
} LR1ItemWorklist;

LR1Item *createLR1Item(char *name, Rule *rule, char *lookahead)
{
  LR1Item *item = malloc(sizeof(LR1Item));
  item->production = strdup(name);
  item->dotPosition = 0;
  item->symbolSize = rule->usedSymbols;
  item->symbols = malloc(sizeof(char *) * item->symbolSize);
  item->lookahead = lookahead;

  for(int i = 0; i < rule->usedSymbols; i++)
  {
    item->symbols[i] = strdup(rule->symbols[i]);
  }

  return item;
}

LR1ItemList *createLR1Items(Production *production)
{
  LR1ItemList *list = malloc(sizeof(LR1ItemList));
  list->usedItems = 0;
  list->itemSize = production->usedRules;
  list->items = malloc(sizeof(LR1Item *) * list->itemSize);

  for(int i =0; i < production->usedRules; i++)
  {
    Rule *rule = production->rules[i];
    list->items[list->usedItems++] = createLR1Item(production->name, rule, END);
  }

  return list;
}

void addSet(FirstSets *sets, FirstSet *set)
{
  if(sets->usedSets == sets->setSize)
  {
    sets->setSize = sets->setSize * 2;
    sets->sets = realloc(sets->sets, sizeof(FirstSet *) * sets->setSize);
  }

  sets->sets[sets->usedSets++] = set;
}

FirstSet *createFirstSet(char *name)
{
  FirstSet *set = calloc(1, sizeof(FirstSet));
  set->usedTerminals = 0;
  set->terminalSize = 5;
  set->terminals = calloc(set->terminalSize, sizeof(char *));
  set->name = name;
  return set;
}

void addTerminal(FirstSet *set, char *terminal)
{
  if(set->usedTerminals == set->terminalSize)
  {
    set->terminalSize = set->terminalSize * 2;
    set->terminals = realloc(set->terminals, sizeof(FirstSet *) * set->terminalSize);
  }

  set->terminals[set->usedTerminals++] = terminal;
}

FirstSet *findSet(FirstSets *sets, char *name)
{
  for(int i = 0; i < sets->usedSets; i++)
  {
    FirstSet *set = sets->sets[i];

    if(strcmp(set->name, name) == 0) return set;
  }

  return NULL;
}

FirstSet *copySet(FirstSet *set)
{
  FirstSet *copy = calloc(1, sizeof(FirstSet));
  *copy = *set;
  copy->terminals = calloc(set->terminalSize, sizeof(char *));
  memcpy(copy->terminals, set->terminals, sizeof(char *) * set->terminalSize);

  return copy;
}

bool setContainsEmpty(FirstSet *set)
{
  if(set->usedTerminals == 0) return false;
  return set->terminals[set->usedTerminals - 1] == EMPTY;
}

void removeEmptyFromSet(FirstSet *set)
{
  if(setContainsEmpty(set))
  {
    --set->usedTerminals;
  }
}

bool setContainsTerminal(FirstSet *set, char *search)
{
  for(int i = 0; i < set->usedTerminals; i++)
  {
    char *terminal = set->terminals[i];

    if(terminal == EMPTY && search == EMPTY) return true;
    if(terminal != EMPTY && strcmp(terminal, search) == 0) return true;
  }

  return false;
}

void combineSets(FirstSet *first, FirstSet *second)
{
  if(second == NULL) return;

  for(int i = 0; i < second->usedTerminals; i++)
  {
    char *terminal = second->terminals[i];
    if(!setContainsTerminal(first, terminal)) addTerminal(first, terminal);
  }
}

FirstSets *createFirstSets(Grammar *grammar, ScannerConfig *config)
{
  FirstSets *sets = malloc(sizeof(FirstSets));
  sets->usedSets = 0;
  sets->setSize = 10;
  sets->sets = malloc(sizeof(FirstSet *) * sets->setSize);

  FirstSet *empty = createFirstSet(EMPTY);
  addTerminal(empty, EMPTY);
  addSet(sets, empty);

  FirstSet *end = createFirstSet(END);
  addTerminal(end, END);
  addSet(sets, end);

  for(int i = 0; i < config->usedCategories; i++)
  {
    Category *cat = config->categories[i];
    FirstSet *set = createFirstSet(cat->name);
    addTerminal(set, cat->name);
    addSet(sets, set);
  }

  for(int i = 0; i < grammar->usedProductions; i++)
  {
    Production *prod = grammar->productions[i];
    FirstSet *set = createFirstSet(prod->name);
    addSet(sets, set);
  }

  bool changing;

  do
  {
    changing = false;

    for(int i = 0; i < grammar->usedProductions; i++)
    {
      Production *prod = grammar->productions[i];

      FirstSet *prodSet = findSet(sets, prod->name);

      for(int j = 0; j < prod->usedRules; j++)
      {
        Rule *rule = prod->rules[j];

        FirstSet *rhs = copySet(findSet(sets, rule->symbols[0]));
        bool containsEmpty = setContainsEmpty(rhs);
        removeEmptyFromSet(rhs);

        int k = 1;
        for(; k < rule->usedSymbols && containsEmpty; k++)
        {
          char *symbol = rule->symbols[k];

          FirstSet *symbolSet = copySet(findSet(sets, symbol));
          containsEmpty = setContainsEmpty(symbolSet);
          removeEmptyFromSet(symbolSet);
          combineSets(rhs, symbolSet);
        }

        if(k == rule->usedSymbols && containsEmpty)
        {
          addTerminal(rhs, EMPTY);
        }

        int oldSize = prodSet->usedTerminals;
        combineSets(prodSet, rhs);

        changing |= oldSize != prodSet->usedTerminals;
      }
    }
  } while(changing);

  return sets;
}

char *getNextSymbol(LR1Item *item)
{
  if(item->dotPosition < item->symbolSize)
  {
    return item->symbols[item->dotPosition];
  }

  return NULL;
}

Production *getProductionForSymbol(Grammar *grammar, char *symbol)
{
  if(symbol == NULL) return NULL;

  for(int i = 0; i < grammar->usedProductions; i++)
  {
    Production *production = grammar->productions[i];
    if(strcmp(production->name, symbol) == 0) return production;
  }

  return NULL;
}

int getLookaheadSymbolsSize(LR1Item *item)
{
  return item->symbolSize - item->dotPosition;
}

char **getLookaheadSymbols(LR1Item *item, char *lookahead)
{
  int remainingSymbols = getLookaheadSymbolsSize(item);
  char **lookaheadsSymbols = malloc(sizeof(char *) * remainingSymbols);

  int lookaheadPosition = 0;
  for(int i = item->dotPosition + 1; i < item->symbolSize; i++)
  {
    lookaheadsSymbols[lookaheadPosition++] = item->symbols[i];
  }

  lookaheadsSymbols[lookaheadPosition] = lookahead;

  return lookaheadsSymbols;
}

FirstSet *getFirstSetForLookaheads(char **lookaheads, int lookaheadSize, FirstSets *sets)
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
  if(first->symbolSize != second->symbolSize) return false;
  if(strcmp(first->lookahead, second->lookahead) != 0) return false;

  for(int i = 0; i < first->symbolSize; i++)
  {
    if(strcmp(first->symbols[i], second->symbols[i]) != 0) return false;
  }

  return true;
}

bool itemListContainsItem(LR1ItemList *list, LR1Item *searchItem)
{
  for(int i = 0; i < list->usedItems; i++)
  {
    LR1Item *item = list->items[i];

    if(areLR1ItemsSame(item, searchItem)) return true;
  }

  return false;
}

bool addLR1ItemToList(LR1ItemList *list, LR1Item *item)
{
  if(itemListContainsItem(list, item)) return false;

  if(list->usedItems == list->itemSize)
  {
    list->itemSize = list->itemSize * 2;
    list->items = realloc(list->items, sizeof(LR1Item *) * list->itemSize);
  }

  list->items[list->usedItems++] = item;

  return true;
}

LR1Item *copyLR1Item(LR1Item *item)
{
  LR1Item *newItem = malloc(sizeof(LR1Item));
  newItem->production = strdup(item->production);
  newItem->dotPosition = item->dotPosition;
  newItem->symbolSize = item->symbolSize;
  newItem->symbols = malloc(sizeof(char *) * item->symbolSize);
  newItem->lookahead = strdup(item->lookahead);

  for(int i = 0; i < item->symbolSize; i++)
  {
    newItem->symbols[i] = strdup(item->symbols[i]);
  }

  return newItem;
}

LR1ItemList *copyLR1ItemList(LR1ItemList *list)
{
  LR1ItemList *newList = malloc(sizeof(LR1ItemList));
  newList->usedItems = list->usedItems;
  newList->itemSize = list->itemSize;
  newList->items = malloc(sizeof(LR1Item *) * list->itemSize);

  for(int i = 0; i < list->usedItems; i++)
  {
    newList->items[i] = copyLR1Item(list->items[i]);
  }

  return newList;
}

LR1ItemWorklist *createWorklist()
{
  LR1ItemWorklist *worklist = malloc(sizeof(LR1ItemWorklist));
  worklist->size = 0;
  worklist->maxSize = 10;
  worklist->items = malloc(sizeof(LR1Item *) * worklist->maxSize);
  return worklist;
}

void addToWorklist(LR1ItemWorklist *worklist, LR1Item *item)
{
  if(worklist->maxSize == worklist->size)
  {
    worklist->maxSize = worklist->maxSize * 2;
    worklist->items = realloc(worklist->items, sizeof(LR1Item *) * worklist->maxSize);
  }

  worklist->items[worklist->size++] = item;
}

LR1Item *removeFromWorklist(LR1ItemWorklist *worklist)
{
  return worklist->items[--worklist->size];;
}

LR1ItemList *closure(LR1ItemList *list, Grammar *grammar, FirstSets *sets)
{
  LR1ItemList *newList = copyLR1ItemList(list);
  LR1ItemWorklist *worklist = createWorklist();

  for(int i = 0; i < newList->usedItems; i++) addToWorklist(worklist, newList->items[i]);

  while(worklist->size > 0)
  {
    LR1Item *item = removeFromWorklist(worklist);
    Production *production = getProductionForSymbol(grammar, getNextSymbol(item));

    if(production == NULL) continue;

    FirstSet *set = getFirstSetForLookaheads(getLookaheadSymbols(item, item->lookahead), getLookaheadSymbolsSize(item), sets);

    for(int j = 0; j < production->usedRules; j++)
    {
      Rule *rule = production->rules[j];

      for(int k = 0; k < set->usedTerminals; k++)
      {
        LR1Item *ruleItem = createLR1Item(production->name, rule, set->terminals[k]);
        if(addLR1ItemToList(newList, ruleItem)) addToWorklist(worklist, ruleItem);
      }
    }
  }

  return newList;
}

char *getDotSymbol(LR1Item *item)
{
  if(item->dotPosition == item->symbolSize) return NULL;

  return item->symbols[item->dotPosition];
}

LR1ItemList *goTo(LR1ItemList *list, char *symbol, Grammar *grammar, FirstSets *firstSets)
{
  LR1ItemList *moved = malloc(sizeof(LR1ItemList));
  moved->usedItems = 0;
  moved->itemSize = 10;
  moved->items = malloc(sizeof(LR1Item *) * moved->itemSize);

  for(int i = 0; i < list->usedItems; i++)
  {
    LR1Item *item = list->items[i];

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
  for(int i = 0; i < list->usedItems; i++)
  {
    LR1Item *item = list->items[i];

    printf("[%s -> ", item->production);

    for(int j = 0; j < item->symbolSize; j++)
    {
      if(j == item->dotPosition) printf("\u2022");
      printf("%s ", item->symbols[j]);
    }

    if(item->dotPosition == item->symbolSize) printf("\u2022");

    printf(", %s]\n", item->lookahead);
  }
}

typedef struct CanonicalCollection {
  int usedLists;
  int listSize;
  LR1ItemList **itemLists;
} CanonicalCollection;

typedef struct LR1ItemListWorklist {
  LR1ItemList **itemLists;
  int size;
  int maxSize;
} LR1ItemListWorklist;

typedef struct Transition {
  LR1ItemList *from;
  LR1ItemList *to;
  char *symbol;
} Transition;

typedef struct Transitions {
  int usedTransitions;
  int transitionSize;
  Transition **transitions;
} Transitions;

void addToItemListWorklist(LR1ItemListWorklist *worklist, LR1ItemList *itemList)
{
  if(worklist->maxSize == worklist->size)
  {
    worklist->maxSize = worklist->maxSize * 2;
    worklist->itemLists = realloc(worklist->itemLists, sizeof(LR1ItemList *) * worklist->maxSize);
  }

  worklist->itemLists[worklist->size++] = itemList;
}

typedef struct FollowingSymbols {
  int usedSymbols;
  int symbolSize;
  char **symbols;
} FollowingSymbols;

LR1ItemList *removeFromItemListWorklist(LR1ItemListWorklist *worklist)
{
  return worklist->itemLists[--worklist->size];
}

bool containsSymbol(FollowingSymbols *symbols, char *symbol)
{
  for(int i = 0; i < symbols->usedSymbols; i++)
  {
    if(strcmp(symbols->symbols[i], symbol) == 0) return true;
  }

  return false;
}

FollowingSymbols *getSymbolsFollowingDot(LR1ItemList *list)
{
  FollowingSymbols *following = malloc(sizeof(FollowingSymbols));
  following->usedSymbols = 0;
  following->symbolSize = 10;
  following->symbols = malloc(sizeof(char *) * following->symbolSize);

  for(int i = 0; i < list->usedItems; i++)
  {
    LR1Item *item = list->items[i];

    char *symbol = getDotSymbol(item);

    if(symbol != NULL && !containsSymbol(following, symbol))
    {
      if(following->usedSymbols == following->symbolSize)
      {
        following->symbolSize = following->symbolSize * 2;
        following->symbols = realloc(following->symbols, sizeof(char *) * following->symbolSize);
      }

      following->symbols[following->usedSymbols++] = symbol;
    }
  }

  return following;
}

bool areLR1ItemListSame(LR1ItemList *first, LR1ItemList *second)
{
  if(first->usedItems != second->usedItems) return false;
  for(int i = 0; i < first->usedItems; i++)
  {
    if(!itemListContainsItem(second, first->items[i])) return false;
  }

  return true;
}

LR1ItemList *tryGetExistingItemList(CanonicalCollection *collection, LR1ItemList *search)
{
  for(int i = 0; i < collection->usedLists; i++)
  {
    LR1ItemList *list = collection->itemLists[i];
    if(areLR1ItemListSame(list, search)) return list;
  }

  return NULL;
}

Parser *createParser(Grammar *grammar, ScannerConfig *config)
{
  Production *goalProduction = grammar->productions[0];

  FirstSets *sets = createFirstSets(grammar, config);
  LR1ItemList *cc0 = closure(createLR1Items(goalProduction), grammar, sets);

  CanonicalCollection *cc = malloc(sizeof(CanonicalCollection));
  cc->usedLists = 0;
  cc->listSize = 10;
  cc->itemLists = malloc(sizeof(LR1ItemList *) * cc->listSize);
  cc->itemLists[cc->usedLists++] = cc0;

  LR1ItemListWorklist *worklist = malloc(sizeof(LR1ItemListWorklist));
  worklist->size = 0;
  worklist->maxSize = 10;
  worklist->itemLists = malloc(sizeof(LR1ItemList *) * worklist->maxSize);
  addToItemListWorklist(worklist, cc0);

  Transitions *transitions = malloc(sizeof(Transitions));
  transitions->usedTransitions = 0;
  transitions->transitionSize = 10;
  transitions->transitions = malloc(sizeof(Transition *) * transitions->transitionSize);

  while(worklist->size > 0)
  {
    LR1ItemList *itemList = removeFromItemListWorklist(worklist);

    FollowingSymbols *following = getSymbolsFollowingDot(itemList);
    for(int i = 0; i < following->usedSymbols; i++)
    {
      char *symbol = following->symbols[i];
      LR1ItemList *temp = goTo(itemList, symbol, grammar, sets);

      LR1ItemList *existing = tryGetExistingItemList(cc, temp);

      if(existing == NULL)
      {
        if(cc->usedLists == cc->listSize)
        {
          cc->listSize = cc->listSize * 2;
          cc->itemLists = realloc(cc->itemLists, sizeof(LR1ItemList *) * cc->listSize);
        }

        cc->itemLists[cc->usedLists++] = temp;

        addToItemListWorklist(worklist, temp);

        existing = temp;
      }

      Transition *transition = calloc(1, sizeof(Transition));
      transition->from = itemList;
      transition->to = existing;
      transition->symbol = symbol;

      if(transitions->usedTransitions == transitions->transitionSize)
      {
        transitions->transitionSize = transitions->transitionSize * 2;
        transitions->transitions = realloc(transitions->transitions, sizeof(Transition *) * transitions->transitionSize);
      }

      transitions->transitions[transitions->usedTransitions++] = transition;
    }
  }

  for(int i = 0; i < cc->usedLists; i++)
  {
    LR1ItemList *list = cc->itemLists[i];
    printf("----CC%i----\n", i);
    printLR1ItemList(list);
  }

  for(int i = 0; i < transitions->usedTransitions; i++)
  {
    Transition *transition = transitions->transitions[i];

    int fromCC = -1;
    int toCC = -1;

    for(int j = 0; j < cc->usedLists; j++)
    {
      if(fromCC != -1 && toCC != -1) break;
      LR1ItemList *list = cc->itemLists[j];

      if(list == transition->from) fromCC = j;
      if(list == transition->to) toCC = j;
    }

    printf("CC%i -> CC%i on %s\n", fromCC, toCC, transition->symbol);
  }

  return NULL;
}
