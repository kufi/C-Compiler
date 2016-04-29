#include "File.h"
#include <stdio.h>
#include <stdlib.h>

char *readFile(char *filePath)
{
  char *fileContents;
  long inputFileSize;

  FILE *inputFile = fopen(filePath, "rb");

  fseek(inputFile, 0, SEEK_END);

  inputFileSize = ftell(inputFile);
  rewind(inputFile);

  fileContents = malloc(inputFileSize * (sizeof(char)));

  fread(fileContents, sizeof(char), inputFileSize, inputFile);
  fclose(inputFile);

  return fileContents;
}
