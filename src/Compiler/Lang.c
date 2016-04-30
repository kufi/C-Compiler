#include "Lang.h"

#include "Scanner/Scanner.h"
#include "Parser/Grammar.h"
#include "Parser/Parser.h"

ScannerConfig *createLangScanner()
{
  ScannerConfig *config = createScannerConfig(3);
  addCategory(config, "val", "val");
  addCategory(config, "var", "var");
  addCategory(config, "def", "def");
  addCategory(config, "return", "return");
  addCategory(config, "string", "\"([a-zA-Z0-9]| |_|-)*\"");
  addCategory(config, "identifier", "[a-zA-Z][a-zA-Z0-9]*");
  addCategory(config, "integer", "(-[0-9]|[0-9])[0-9]*");
  addCategory(config, "=", "=");
  addCategory(config, ";", ";");
  addCategory(config, ",", ",");
  addCategory(config, "*", "\\*");
  addCategory(config, "/", "/");
  addCategory(config, "+", "\\+");
  addCategory(config, "-", "-");
  addCategory(config, "(", "\\(");
  addCategory(config, ")", "\\)");
  addCategory(config, "{", "{");
  addCategory(config, "}", "}");
  addCategory(config, "comp", "<|>|<=|>=|==");
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

ASTNode *declarationsReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  ASTNode *subDeclarations = parseTreeSubItem(item, 0)->astNode;

  for(int i = 0; i < arrayListCount(subDeclarations->subNodes); i++)
  {
    addSubNode(node, arrayListGet(subDeclarations->subNodes, i));
  }

  addSubNode(node, parseTreeSubItem(item, 1)->astNode);
  return node;
}

ASTNode *assignmentReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  addSubNode(node, parseTreeSubItem(item, 0)->astNode);
  addSubNode(node, parseTreeSubItem(item, 2)->astNode);
  return node;
}

ASTNode *variableAssignmentReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, parseTreeSubItem(item, 0)->word);
  addSubNode(node, parseTreeSubItem(item, 1)->astNode);
  return node;
}

ASTNode *functionReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, parseTreeSubItem(item, 1)->word);
  addSubNode(node, parseTreeSubItem(item, 3)->astNode);
  addSubNode(node, parseTreeSubItem(item, 6)->astNode);
  return node;
}

ASTNode *functionParametersReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  node->subNodes = parseTreeSubItem(item, 0)->astNode->subNodes;
  addSubNode(node, parseTreeSubItem(item, 2)->astNode);
  return node;
}

ASTNode *emptyReduce(ParseTreeItem *item)
{
  return createASTNode(item->rule, NULL);
}

ASTNode *singleFunctionParametersReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  addSubNode(node, parseTreeSubItem(item, 0)->astNode);
  return node;
}

ASTNode *expressionsReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  node->subNodes = parseTreeSubItem(item, 0)->astNode->subNodes;
  addSubNode(node, parseTreeSubItem(item, 1)->astNode);
  return node;
}

ASTNode *singleExpressionsReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  addSubNode(node, parseTreeSubItem(item, 0)->astNode);
  return node;
}

ASTNode *expression1Reduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  addSubNode(node, parseTreeSubItem(item, 0)->astNode);
  return node;
}

ASTNode *functionCallReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, parseTreeSubItem(item, 0)->word);
  addSubNode(node, parseTreeSubItem(item, 2)->astNode);
  return node;
}

ASTNode *returnReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, NULL);
  addSubNode(node, parseTreeSubItem(item, 1)->astNode);
  return node;
}

ASTNode *comparisonReduce(ParseTreeItem *item)
{
  ASTNode *node = createASTNode(item->rule, parseTreeSubItem(item, 1)->word);
  addSubNode(node, parseTreeSubItem(item, 0)->astNode);
  addSubNode(node, parseTreeSubItem(item, 2)->astNode);
  return node;
}

ParserTable *createLangParser()
{
  ScannerConfig *config = createLangScanner();
  Grammar *grammar = createGrammar(config);
  addProduction(grammar, "Goal", "Declarations", NULL);
  addProduction(grammar, "Declarations", "Declarations Declaration", declarationsReduce);
  addProduction(grammar, "Declarations", "Declaration", NULL);
  addProduction(grammar, "Declaration", "DeclareVariable", NULL);
  addProduction(grammar, "Declaration", EMPTY, NULL);
  addProduction(grammar, "Declaration", "Function", NULL);
  addProduction(grammar, "DeclareVariable", "val Assignment ;", variableAssignmentReduce);
  addProduction(grammar, "DeclareVariable", "var Assignment ;", variableAssignmentReduce);
  addProduction(grammar, "DeclareVariable", "var identifier ;", variableAssignmentReduce);
  addProduction(grammar, "Assignment", "identifier = Value", assignmentReduce);
  addProduction(grammar, "Value", "identifier", NULL);
  addProduction(grammar, "Value", "integer", NULL);
  addProduction(grammar, "Value", "Calculation", NULL);
  addProduction(grammar, "Value", "string", NULL);
  addProduction(grammar, "Value", "FunctionCall", NULL);
  addProduction(grammar, "Value", "Comparison", NULL);
  addProduction(grammar, "Function", "def identifier ( FunctionParameters ) { Expressions }", functionReduce);
  addProduction(grammar, "FunctionParameters", "FunctionParameters , Value", functionParametersReduce);
  addProduction(grammar, "FunctionParameters", "Value", singleFunctionParametersReduce);
  addProduction(grammar, "FunctionParameters", EMPTY, emptyReduce);
  addProduction(grammar, "FunctionCall", "identifier ( FunctionParameters )", functionCallReduce);
  addProduction(grammar, "Expressions", "Expressions Expression", expressionsReduce);
  addProduction(grammar, "Expressions", "Expression", singleExpressionsReduce);
  addProduction(grammar, "Expression", "DeclareVariable", expression1Reduce);
  addProduction(grammar, "Expression", "Assignment ;", expression1Reduce);
  addProduction(grammar, "Expression", "Value ;", expression1Reduce);
  addProduction(grammar, "Expression", "Return ;", expression1Reduce);
  addProduction(grammar, "Return", "return Value", returnReduce);
  addProduction(grammar, "Expression", EMPTY, emptyReduce);
  addProduction(grammar, "Calculation", "Calculation + Term", calculationReduce);
  addProduction(grammar, "Calculation", "Calculation - Term", calculationReduce);
  addProduction(grammar, "Calculation", "Term", NULL);
  addProduction(grammar, "Term", "Term * Factor", calculationReduce);
  addProduction(grammar, "Term", "Term / Factor", calculationReduce);
  addProduction(grammar, "Term", "Factor", NULL);
  addProduction(grammar, "Factor", "( Calculation )", bracesReduce);
  addProduction(grammar, "Factor", "identifier", NULL);
  addProduction(grammar, "Factor", "integer", NULL);
  addProduction(grammar, "Comparison", "Value comp Value", comparisonReduce);

  return createParser(grammar, "Goal");
}
