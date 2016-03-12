#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"
#include "Parser.h"
#include "../Scanner/Scanner.h"
#include "../Util/Collections/ArrayList.h"
#include "../Util/Collections/Queue.h"
#include "../Util/Collections/HashSet.h"
#include "../Scanner/StringBuilder.h"

typedef struct LR1Item {
  char *production;
  int dotPosition;
  ArrayList *symbols;
  char *lookahead;
} LR1Item;

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

typedef struct Transition {
  ArrayList *from;
  ArrayList *to;
  char *symbol;
} Transition;

LR1Item *createLR1Item(char *name, Rule *rule, char *lookahead)
{
  LR1Item *item = calloc(1, sizeof(LR1Item));
  item->production = strdup(name);
  item->dotPosition = 0;
  item->symbols = arrayListCreate(rule->usedSymbols, sizeof(char *));
  item->lookahead = lookahead;

  for(int i = 0; i < rule->usedSymbols; i++)
  {
    arrayListPush(item->symbols, strdup(rule->symbols[i]));
  }

  return item;
}

ArrayList *createLR1Items(Production *production)
{
  ArrayList *list = arrayListCreate(10, sizeof(LR1Item *));

  for(int i =0; i < production->usedRules; i++)
  {
    Rule *rule = production->rules[i];
    arrayListPush(list, createLR1Item(production->name, rule, END));
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

  for(int i = 0; i < config->categories->used; i++)
  {
    Category *cat = arrayListGet(config->categories, i);
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
  if(item->dotPosition < arrayListCount(item->symbols))
  {
    return arrayListGet(item->symbols, item->dotPosition);
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
  return arrayListCount(item->symbols) - item->dotPosition;
}

char **getLookaheadSymbols(LR1Item *item, char *lookahead)
{
  int remainingSymbols = getLookaheadSymbolsSize(item);
  char **lookaheadsSymbols = malloc(sizeof(char *) * remainingSymbols);

  int lookaheadPosition = 0;
  for(int i = item->dotPosition + 1; i < arrayListCount(item->symbols); i++)
  {
    lookaheadsSymbols[lookaheadPosition++] = arrayListGet(item->symbols, i);
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
  if(arrayListCount(first->symbols) != arrayListCount(second->symbols)) return false;
  if(strcmp(first->lookahead, second->lookahead) != 0) return false;

  for(int i = 0; i < arrayListCount(first->symbols); i++)
  {
    if(strcmp(arrayListGet(first->symbols, i), arrayListGet(second->symbols, i)) != 0) return false;
  }

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

bool addLR1ItemToList(ArrayList *list, LR1Item *item)
{
  if(itemListContainsItem(list, item)) return false;
  arrayListPush(list, item);
  return true;
}

LR1Item *copyLR1Item(LR1Item *item)
{
  LR1Item *newItem = malloc(sizeof(LR1Item));
  newItem->production = strdup(item->production);
  newItem->dotPosition = item->dotPosition;
  newItem->symbols = arrayListCreate(arrayListCount(item->symbols), sizeof(char *));
  newItem->lookahead = strdup(item->lookahead);

  for(int i = 0; i < arrayListCount(item->symbols); i++)
  {
    arrayListPush(newItem->symbols, strdup(arrayListGet(item->symbols, i)));
  }

  return newItem;
}

ArrayList *copyLR1ItemList(ArrayList *list)
{
  ArrayList *newList = arrayListCreate(arrayListCount(list), sizeof(LR1Item *));

  for(int i = 0; i < arrayListCount(list); i++)
  {
    arrayListPush(newList, copyLR1Item(arrayListGet(list, i)));
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

ArrayList *closure(ArrayList *list, Grammar *grammar, FirstSets *sets)
{
  ArrayList *newList = copyLR1ItemList(list);
  LR1ItemWorklist *worklist = createWorklist();

  for(int i = 0; i < arrayListCount(newList); i++) addToWorklist(worklist, arrayListGet(newList, i));

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
  if(item->dotPosition == arrayListCount(item->symbols)) return NULL;

  return arrayListGet(item->symbols, item->dotPosition);
}

ArrayList *goTo(ArrayList *list, char *symbol, Grammar *grammar, FirstSets *firstSets)
{
  ArrayList *moved = arrayListCreate(10, sizeof(LR1Item *));

  for(int i = 0; i < arrayListCount(list); i++)
  {
    LR1Item *item = arrayListGet(list, i);
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

void printLR1ItemList(ArrayList *list)
{
  for(int i = 0; i < arrayListCount(list); i++)
  {
    LR1Item *item = arrayListGet(list, i);

    printf("[%s -> ", item->production);

    for(int j = 0; j < arrayListCount(item->symbols); j++)
    {
      if(j == item->dotPosition) printf("\u2022");
      printf("%s ", (char *)arrayListGet(item->symbols, j));
    }

    if(item->dotPosition == arrayListCount(item->symbols)) printf("\u2022");

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

ArrayList *getSymbolsFollowingDot(ArrayList *list)
{
  ArrayList *following = arrayListCreate(10, sizeof(char *));

  for(int i = 0; i < arrayListCount(list); i++)
  {
    LR1Item *item = arrayListGet(list, i);
    char *symbol = getDotSymbol(item);

    if(symbol != NULL && !containsSymbol(following, symbol))
    {
      arrayListPush(following, symbol);
    }
  }

  return following;
}

bool areLR1ItemListSame(ArrayList *first, ArrayList *second)
{
  if(arrayListCount(first) != arrayListCount(second)) return false;

  for(int i = 0; i < arrayListCount(first); i++)
  {
    if(!itemListContainsItem(second, arrayListGet(first, i))) return false;
  }

  return true;
}

ArrayList *tryGetExistingItemList(HashSet *collection, ArrayList *search)
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

  for(int i = 0; i < arrayListCount(item->symbols); i++)
  {
    appendChars(&s, arrayListGet(item->symbols, i));
  }

  return s.string;
}

uint32_t ccHash(void *hashable)
{
  ArrayList *list = (ArrayList *)hashable;
  StringBuilder builder = createStringBuilder();

  for(int i = 0; i < arrayListCount(list); i++)
  {
    LR1Item *item = arrayListGet(list, i);
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

Parser *createParser(Grammar *grammar, ScannerConfig *config)
{
  Production *goalProduction = grammar->productions[0];

  FirstSets *sets = createFirstSets(grammar, config);
  ArrayList *cc0 = closure(createLR1Items(goalProduction), grammar, sets);

  HashSet *cc = hashSetCreateFull(0, 0.0f, ccCompare, ccHash);
  hashSetPut(cc, cc0);

  Queue *worklist = queueCreate();
  queueEnqueue(worklist, cc0);

  ArrayList *transitions = arrayListCreate(10, sizeof(Transition *));

  ArrayList *itemList = NULL;

  int c = 0;
  while((itemList = queueDequeue(worklist)) != NULL)
  {
    c++;
    if(c==100) break;
    ArrayList *following = getSymbolsFollowingDot(itemList);

    for(int i = 0; i < following->used; i++)
    {
      char *symbol = arrayListGet(following, i);
      ArrayList *temp = goTo(itemList, symbol, grammar, sets);
      arrayListQSort(temp, itemListCompare);
      ArrayList *existing = tryGetExistingItemList(cc, temp);

      if(existing == NULL)
      {
        hashSetPut(cc, temp);
        queueEnqueue(worklist, temp);
        existing = temp;
      }

      Transition *transition = calloc(1, sizeof(Transition));
      transition->from = itemList;
      transition->to = existing;
      transition->symbol = symbol;

      arrayListPush(transitions, transition);
    }
  }

  int i = 0;
  hashSetFor(cc, cur)
  {
    ArrayList *list = hashSetForItem(cur);
    printf("----CC%i----\n", i++);
    printLR1ItemList(list);
  }
  hashSetForEnd

  for(int i = 0; i < transitions->used; i++)
  {
    Transition *transition = arrayListGet(transitions, i);

    printf("CC%i -> CC%i on %s\n", transition->from, transition->to, transition->symbol);
  }

  return NULL;
}
