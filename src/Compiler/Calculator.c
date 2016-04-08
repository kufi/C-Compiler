#include "Calculator.h"

#include "Scanner/Scanner.h"
#include "Parser/Grammar.h"
#include "Parser/Parser.h"

ScannerConfig *createCalculatorScanner()
{
  ScannerConfig *config = createScannerConfig(3);
  addCategory(config, "num", "(-(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*)|((1|2|3|4|5|6|7|8|9)|(1|2|3|4|5|6|7|8|9)*)");
  addCategory(config, "name", "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)*");
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
