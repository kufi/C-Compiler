#include "File.h"
#include <stdio.h>
#include <stdlib.h>

char *readFile(char *filePath)
{
  FILE *inputFile = fopen(filePath, "rb");

  fseek(inputFile, 0, SEEK_END);

  long inputFileSize = ftell(inputFile) + 1;
  rewind(inputFile);

  char *fileContents = calloc(1, inputFileSize * (sizeof(char)));

  fread(fileContents, sizeof(char), inputFileSize, inputFile);
  fclose(inputFile);

  return fileContents;
}
