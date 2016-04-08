#ifndef GRAMMAR_HEADER
#define GRAMMAR_HEADER

#include "Util/Collections/HashMap.h"
#include "Util/Collections/ArrayList.h"
#include "Scanner/Scanner.h"

extern char *END;
extern char *EMPTY;

struct ParseTreeItem;
struct ASTNode;

typedef struct ASTNode *(*ReduceAction)(struct ParseTreeItem *parseTreeItem);

typedef struct Rule {
  char *productionName;
  ArrayList *symbols;
  ReduceAction reduceAction;
} Rule;

typedef struct Grammar {
  HashMap *productions;
  ScannerConfig *scannerConfig;
} Grammar;

Grammar *createGrammar(ScannerConfig *config);

void addProduction(Grammar *grammar, char *name, char *rule, ReduceAction reduceAction);

ArrayList *getProductionForSymbol(Grammar *grammar, char *symbol);

#endif
