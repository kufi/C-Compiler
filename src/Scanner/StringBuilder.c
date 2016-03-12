#include <stdlib.h>
#include <string.h>
#include "StringBuilder.h"

StringBuilder createStringBuilder()
{
  return stringBuilderCreateFull(15);
}

StringBuilder stringBuilderCreateFull(int initialSize)
{
  StringBuilder builder;
  builder.used = 0;
  builder.size = initialSize;
  builder.string = calloc(initialSize, sizeof(char));
  builder.string[0] = '\0';

  return builder;
}

void appendChars(StringBuilder *builder, char *c)
{
  int length = strlen(c);
  if(builder->used + length >= builder->size)
  {
    builder->size = (builder->size + length) * 2;
    builder->string = realloc(builder->string, sizeof(char) * builder->size);
  }

  builder->used += length;

  strcat(builder->string, c);
}

void appendChar(StringBuilder *builder, char c)
{
  if(builder->used + 1 == builder->size)
  {
    builder->size = builder->size * 2;
    builder->string = realloc(builder->string, sizeof(char) * builder->size);
  }

  builder->string[builder->used++] = c;
  builder->string[builder->used] = '\0';
}

void removeLastChar(StringBuilder *builder)
{
  builder->string[--builder->used] = '\0';
}
