#ifndef GRAMMAR_HEADER
#define GRAMMAR_HEADER

#include "../Util/Collections/ArrayList.h"

extern char *END;
extern char *EMPTY;

typedef struct Rule {
  ArrayList *symbols;
} Rule;

typedef struct Production {
  char *name;
  ArrayList *rules;
} Production;

typedef struct Grammar {
  ArrayList *productions;
} Grammar;

Grammar *createGrammar();

void addProduction(Grammar *grammar, char *name, char *ruleSize, ...);

Production *getProductionForSymbol(Grammar *grammar, char *symbol);

#endif
