
#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <stdbool.h>
#include "../Scanner/Scanner.h"
#include "../Util/Collections/HashMap.h"

enum ActionType { SHIFT, REDUCE, ACCEPT };

typedef struct Action {
  enum ActionType type;
  char *symbol;
  union {
    int toState;
    struct {
      Production *toProduction;
      Rule *toRule;
    };
  };
} Action;

typedef struct GoTo {
  int number;
  Production *production;
} GoTo;

typedef struct ParseState {
  int number;
  HashMap *actions;
  HashMap *gotos;
} ParseState;

typedef struct ParserTable {
  ArrayList *states;
} ParserTable;

enum ParseTreeItemType { TERMINAL, NONTERMINAL };

typedef struct ParseTreeItem {
  enum ParseTreeItemType type;
  union {
      struct {
        Production *production;
        Rule *rule;
        LinkedList *subItems;
      };
      struct {
        Word *word;
      };
  };
} ParseTreeItem;

typedef struct ParseTree {
  bool success;
  ParseTreeItem *root;
} ParseTree;

ParserTable *createParser(Grammar *grammar, ScannerConfig *config);

ParseTree *runParser(ParserTable *table, ScannerConfig *scannerConfig, char *input);

#endif
