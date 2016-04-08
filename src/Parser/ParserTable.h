#ifndef PARSERTABLE_HEADER
#define PARSERTABLE_HEADER

#include "CanonicalCollection.h"

enum ActionType { REDUCE, SHIFT, ACCEPT };

typedef struct Action {
  enum ActionType type;
  char *symbol;
  union {
    int toState;
    Rule *toRule;
  };
} Action;

typedef struct GoTo {
  int number;
  char *productionName;
} GoTo;

typedef struct ParseState {
  int number;
  HashMap *actions;
  HashMap *gotos;
} ParseState;

typedef struct ParserTable {
  ArrayList *states;
  Grammar *grammar;
} ParserTable;

ParserTable *createParserTable(CC *cc, Grammar *grammar, char *goalProduction);

#endif
