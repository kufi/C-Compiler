#include "CanonicalCollection.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../Util/Collections/Queue.h"
#include "FirstSet.h"
#include "../Scanner/StringBuilder.h"

LR1Item *createLR1Item(Production *production, Rule *rule, char *lookahead)
{
  LR1Item *item = calloc(1, sizeof(LR1Item));
  item->production = production;
  item->rule = rule;
  item->dotPosition = arrayListCount(rule->symbols) == 1 && strcmp(arrayListGet(rule->symbols, 0), EMPTY) == 0 ? 1 : 0;
  item->lookahead = lookahead;

  return item;
}

LR1ItemList *createLR1Items(Production *production)
{
  LR1ItemList *list = calloc(1, sizeof(LR1ItemList));
  list->items = arrayListCreate(10, sizeof(LR1Item *));

  for(int i =0; i < arrayListCount(production->rules); i++)
  {
    arrayListPush(list->items, createLR1Item(production, arrayListGet(production->rules, i), END));
  }

  return list;
}

bool containsSymbol(ArrayList *symbols, char *symbol)
{
  for(int i = 0; i < symbols->used; i++)
  {
    if(strcmp(symbols->items[i], symbol) == 0) return true;
  }

  return false;
}

char *getDotSymbol(LR1Item *item)
{
  if(item->dotPosition == arrayListCount(item->rule->symbols)) return NULL;

  return arrayListGet(item->rule->symbols, item->dotPosition);
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

char *getNextSymbol(LR1Item *item)
{
  if(item->dotPosition < arrayListCount(item->rule->symbols))
  {
    return arrayListGet(item->rule->symbols, item->dotPosition);
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

LR1Item *copyLR1Item(LR1Item *item)
{
  LR1Item *newItem = malloc(sizeof(LR1Item));
  newItem->production = item->production;
  newItem->dotPosition = item->dotPosition;
  newItem->rule = item->rule;
  newItem->lookahead = item->lookahead;
  return newItem;
}

bool addLR1ItemToList(LR1ItemList *list, LR1Item *item)
{
  if(itemListContainsItem(list->items, item)) return false;
  arrayListPush(list->items, item);
  return true;
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
  {;
    Production *production = getProductionForSymbol(grammar, getNextSymbol(item));

    if(production == NULL) continue;

    FirstSet *set = getFirstSetForLookaheads(getLookaheadSymbols(item, item->lookahead), getLookaheadSymbolsSize(item), sets);

    for(int j = 0; j < arrayListCount(production->rules); j++)
    {
      Rule *rule = arrayListGet(production->rules, j);

      for(int k = 0; k < arrayListCount(set->terminals); k++)
      {
        LR1Item *ruleItem = createLR1Item(production, rule, arrayListGet(set->terminals, k));
        if(addLR1ItemToList(newList, ruleItem)) queueEnqueue(worklist, ruleItem);
      }
    }
  }

  return newList;
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

char *createStringFromLR1Item(LR1Item *item)
{
  StringBuilder s = stringBuilderCreateFull(50);

  appendChars(&s, item->production->name);
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

int itemListCompare(const void *a, const void *b)
{
  LR1Item *first = *(void **)a;
  LR1Item *second = *(void **)b;
  return strcmp(createStringFromLR1Item(first), createStringFromLR1Item(second));
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

bool ccCompare(void *a, void *b)
{
  return areLR1ItemListSame(a, b);
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

static uint32_t transitionsHash(void *a)
{
  return ((LR1ItemList *)a)->number;
}

bool transitionsCompare(void *a, void *b)
{
  return ((HashMap *)a) == ((HashMap *)b);
}

CC *buildCanonicalCollection(HashMap *sets, Grammar *grammar, Production *goalProduction)
{
  HashSet *itemLists = hashSetCreateFull(0, 0.0f, ccCompare, ccHash);
  HashMap *transitions = hashMapCreateFull(0, 0.0f, transitionsCompare, transitionsHash);
  LR1ItemList *cc0 = closure(createLR1Items(goalProduction), grammar, sets);
  cc0->number = nextItemListNumber();
  hashSetPut(itemLists, cc0);

  Queue *worklist = queueCreate();
  queueEnqueue(worklist, cc0);

  LR1ItemList *itemList = NULL;
  while((itemList = queueDequeue(worklist)) != NULL)
  {
    ArrayList *following = getSymbolsFollowingDot(itemList);

    for(int i = 0; i < arrayListCount(following); i++)
    {
      char *symbol = arrayListGet(following, i);
      LR1ItemList *temp = goTo(itemList, symbol, grammar, sets);
      arrayListQSort(temp->items, itemListCompare);
      LR1ItemList *existing = hashSetGetExisting(itemLists, temp);

      if(existing == NULL)
      {
        hashSetPut(itemLists, temp);
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

  CC *cc = malloc(sizeof(CC));
  cc->itemLists = itemLists;
  cc->transitions = transitions;

  return cc;
}
