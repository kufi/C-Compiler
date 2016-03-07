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
  FirstSet *set = malloc(sizeof(FirstSet));
  set->usedTerminals = 0;
  set->terminalSize = 5;
  set->terminals = malloc(sizeof(char *) * set->terminalSize);
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
  FirstSet *copy = malloc(sizeof(FirstSet));
  *copy = *set;
  copy->terminals = malloc(sizeof(char *) * set->terminalSize);
  memcpy(copy->terminals, set->terminals, sizeof(char *) * set->terminalSize);

  return copy;
}

bool setContainsEmpty(FirstSet *set)
{
  return set->terminals[set->usedTerminals - 1] == EMPTY;
}

void removeEmptyFromSet(FirstSet *set)
{
  if(set->terminals[set->usedTerminals - 1] == EMPTY)
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
  return (item->symbolSize - item->dotPosition) + 1;
}

char **getLookaheadSymbols(LR1Item *item, char *lookahead)
{
  int remainingSymbols = getLookaheadSymbolsSize(item);
  char **lookaheadsSymbols = malloc(sizeof(char *) * remainingSymbols);

  int lookaheadPosition = 0;
  for(int i = item->dotPosition; i < item->symbolSize; i++)
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

typedef struct LR1ItemWorklist {
  LR1Item **items;
  int size;
  int maxSize;
} LR1ItemWorklist;

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

  for(int i = 0; i < newList->itemSize; i++)
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

  for(int i = 0; i < newList->usedItems; i++)
  {
    addToWorklist(worklist, newList->items[i]);
  }

  while(worklist->size > 0)
  {
    LR1Item *item = removeFromWorklist(worklist);

    Production *production = getProductionForSymbol(grammar, getNextSymbol(item));

    if(production == NULL) continue;

    char **lookaheads = getLookaheadSymbols(item, item->lookahead);

    FirstSet *set = getFirstSetForLookaheads(lookaheads, getLookaheadSymbolsSize(item), sets);

    for(int j = 0; j < production->usedRules; j++)
    {
      Rule *rule = production->rules[j];

      for(int k = 0; k < set->usedTerminals; k++)
      {
        LR1Item *ruleItem = createLR1Item(production->name, rule, set->terminals[k]);

        if(addLR1ItemToList(newList, ruleItem))
        {
          addToWorklist(worklist, ruleItem);
        }
      }
    }
  }

  return newList;
}

Parser *createParser(Grammar *grammar, ScannerConfig *config)
{
  Production *goalProduction = grammar->productions[0];

  FirstSets *sets = createFirstSets(grammar, config);
  LR1ItemList *cc0 = closure(createLR1Items(goalProduction), grammar, sets);

  for(int i = 0; i < cc0->usedItems; i++)
  {
    LR1Item *item = cc0->items[i];

    printf("[%s -> ", item->production);

    for(int j = 0; j < item->symbolSize; j++)
    {
      if(j == item->dotPosition) printf(".");
      printf("%s ", item->symbols[j]);
    }

    printf(", %s]\n", item->lookahead);
  }

  return NULL;
}
