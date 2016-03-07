#ifndef GRAMMAR_HEADER
#define GRAMMAR_HEADER

extern char *END;
extern char *EMPTY;

typedef struct Rule {
  int usedSymbols;
  int symbolSize;
  char **symbols;
} Rule;

typedef struct Production {
  char *name;
  int usedRules;
  int ruleSize;
  Rule **rules;
} Production;

typedef struct Grammar {
  int usedProductions;
  int productionSize;
  Production **productions;
} Grammar;

Grammar *createGrammar();

void addProduction(Grammar *grammar, char *name, char *ruleSize, ...);

#endif
