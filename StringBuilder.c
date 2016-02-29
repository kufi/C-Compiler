#include <stdlib.h>
#include "StringBuilder.h"

StringBuilder createStringBuilder()
{
  StringBuilder builder;
  builder.used = 0;
  builder.size = 10;
  builder.string = malloc(sizeof(char) * builder.size);
  builder.string[0] = '\0';

  return builder;
}

void appendChar(StringBuilder *builder, char c)
{
  if(builder->used  + 1 == builder->size)
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
