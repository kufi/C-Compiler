
#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <stdbool.h>
#include "../Scanner/Scanner.h"
#include "../Util/Collections/HashMap.h"
#include "ParserTable.h"

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
