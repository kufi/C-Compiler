#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Debug/Printer.h"
#include "Compiler/Calculator.h"

int main(int argc, char **argv)
{
  ParserTable *table = createCalculatorParser();
  ParseTree *tree = runParser(table, "(1+2)*(3+(3-b)+3/4)");

  //ParseTree *tree = runParser(table, config, "(()())()");

  if(tree->success)
  {
    //printParseTree(tree);
    printAST(tree);
  }
  else
  {
    printf("Could not parse input\n");
  }

  return 0;
}
