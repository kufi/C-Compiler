#include "Calculator.h"

#include "Scanner/Scanner.h"
#include "Parser/Grammar.h"
#include "Parser/Parser.h"

ScannerConfig *createCalculatorScanner()
{
  ScannerConfig *config = createScannerConfig(3);
  addCategory(config, "num", "(-[0-9]+)|([1-9]|[0-9]*)");
  addCategory(config, "name", "[a-z]+");
  addCategory(config, "*", "\\*");
  addCategory(config, "/", "/");
  addCategory(config, "+", "+");
  addCategory(config, "-", "-");
  addCategory(config, "(", "\\(");
  addCategory(config, ")", "\\)");
  return config;
}

ASTNode *bracesReduce(ParseTreeItem *item)
{
  return parseTreeSubItem(item, 1)->astNode;
}

ASTNode *calculationReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, parseTreeSubItem(item, 1)->word);
  addSubNode(node, parseTreeSubItem(item, 0)->astNode);
  addSubNode(node, parseTreeSubItem(item, 2)->astNode);
  return node;
}

ParserTable *createCalculatorParser()
{
  ScannerConfig *config = createCalculatorScanner();
  Grammar *grammar = createGrammar(config);
  addProduction(grammar, "Goal", "Expr", NULL);
  addProduction(grammar, "Expr", "Expr + Term", calculationReduce);
  addProduction(grammar, "Expr", "Expr - Term", calculationReduce);
  addProduction(grammar, "Expr", "Term", NULL);
  addProduction(grammar, "Term", "Term * Factor", calculationReduce);
  addProduction(grammar, "Term", "Term / Factor", calculationReduce);
  addProduction(grammar, "Term", "Factor", NULL);
  addProduction(grammar, "Factor", "( Expr )", bracesReduce);
  addProduction(grammar, "Factor", "num", NULL);
  addProduction(grammar, "Factor", "name", NULL);

  return createParser(grammar, "Goal");
}
