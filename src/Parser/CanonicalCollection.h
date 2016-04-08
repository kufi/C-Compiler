#ifndef CANONICALCOLLECTION_HEADER
#define CANONICALCOLLECTION_HEADER

#include "Grammar.h"
#include "Util/Collections/HashMap.h"
#include "Util/Collections/HashSet.h"

typedef struct LR1Item {
  Rule *rule;
  int dotPosition;
  char *lookahead;
} LR1Item;

typedef struct LR1ItemList {
  int number;
  ArrayList *items;
} LR1ItemList;

typedef struct CC {
  HashSet *itemLists;
  HashMap *transitions;
} CC;

char *getDotSymbol(LR1Item *item);

CC *buildCanonicalCollection(HashMap *sets, Grammar *grammar, char *goalProduction);

#endif
