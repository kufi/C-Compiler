#include "Braces.h"

#include "Scanner/Scanner.h"
#include "Parser/Grammar.h"
#include "Parser/Parser.h"


ASTNode *emptyReduce(ParseTreeItem *parseTreeItem)
{
  return NULL;
}

ASTNode *pairReduce(ParseTreeItem *parseTreeItem)
{
  ASTNode *node = createASTNode(parseTreeItem->rule, NULL);
  addSubNode(node, parseTreeSubItem(parseTreeItem, 0)->astNode);

  ParseTreeItem *listTreeItem = parseTreeSubItem(parseTreeItem, 1);
  if(listTreeItem->astNode != NULL)
  {
    addSubNode(node, listTreeItem->astNode);
  }
  addSubNode(node, parseTreeSubItem(parseTreeItem, 2)->astNode);

  return node;
}

ASTNode *listPairReduce(ParseTreeItem *parseTreeItem)
{
  ASTNode *node = createASTNode(parseTreeItem->rule, NULL);
  addSubNode(node, parseTreeSubItem(parseTreeItem, 0)->astNode);
  addSubNode(node, parseTreeSubItem(parseTreeItem, 1)->astNode);
  return node;
}

ParserTable *createBracesParser()
{
  ScannerConfig *config = createScannerConfig(2);
  addCategory(config, "(", "\\(");
  addCategory(config, ")", "\\)");

  Grammar *grammar = createGrammar(config);
  addProduction(grammar, "Goal", "List", NULL);
  addProduction(grammar, "List", "List Pair", listPairReduce);
  addProduction(grammar, "List", "Pair", NULL);
  addProduction(grammar, "Pair", "( List )", pairReduce);
  addProduction(grammar, "Pair", EMPTY, emptyReduce);

  return createParser(grammar, "Goal");
}
