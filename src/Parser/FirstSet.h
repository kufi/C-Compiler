#ifndef FIRSTSET_HEADER
#define FIRSTSET_HEADER

#include "Grammar.h"
#include  "Scanner/Scanner.h"
#include "Util/Collections/HashMap.h"

typedef struct FirstSet {
  char *name;
  ArrayList *terminals;
} FirstSet;

HashMap *createFirstSets(Grammar *grammar);

FirstSet *getFirstSetForLookaheads(char **lookaheads, int lookaheadSize, HashMap *sets);

#endif
