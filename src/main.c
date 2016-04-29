#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Debug/Printer.h"
#include "Compiler/Lang.h"
#include "Util/IO/File.h"

int main(int argc, char **argv)
{
  if(argc != 2)
  {
    printf("Please set file to read\n");
    return 0;
  }

  ParserTable *table = createLangParser();
  ParseTree *tree = runParser(table, readFile(argv[1]));

  if(tree != NULL && tree->success)
  {
    printParseTree(tree);
    printAST(tree);
  }
  else
  {
    printf("Could not parse input\n");
  }

  return 0;
}
