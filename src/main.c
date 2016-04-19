#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Debug/Printer.h"
#include "Compiler/Calculator.h"

int main(int argc, char **argv)
{
  ParserTable *table = createCalculatorParser();
  ParseTree *tree = runParser(table, "(43+5402)*(313+(32-b)+3/-433)");

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
