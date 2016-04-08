
#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <stdbool.h>
#include "Scanner/Scanner.h"
#include "Util/Collections/HashMap.h"
#include "Util/Collections/ArrayList.h"
#include "ParserTable.h"
#include "Grammar.h"

enum ParseTreeItemType { TERMINAL, NONTERMINAL };

typedef struct ASTNode {
  Rule *rule;
  ArrayList *subNodes;
  Word *word;
} ASTNode;

typedef struct ParseTreeItem {
  enum ParseTreeItemType type;
  union {
      struct {
        Rule *rule;
        ArrayList *subItems;
      };
      struct {
        Word *word;
      };
  };
  ASTNode *astNode;
} ParseTreeItem;

typedef struct ParseTree {
  bool success;
  ParseTreeItem *root;
} ParseTree;

static inline ParseTreeItem *parseTreeSubItem(ParseTreeItem *item, int index)
{
  return (ParseTreeItem *)arrayListGet(item->subItems, index);
}

ParserTable *createParser(Grammar *grammar, char *goalProduction);

ASTNode *createASTNode(Rule *rule, Word *word);

void addSubNode(ASTNode *node, ASTNode *subNode);

ParseTree *runParser(ParserTable *table, char *input);

#endif
