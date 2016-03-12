#ifndef STRINGBUILDER_HEADER
#define STRINGBUILDER_HEADER

typedef struct StringBuilder {
  int used;
  int size;
  char *string;
} StringBuilder;

StringBuilder createStringBuilder();

StringBuilder stringBuilderCreateFull(int initialSize);

void appendChars(StringBuilder *builder, char *c);

void appendChar(StringBuilder *builder, char c);

void removeLastChar(StringBuilder *builder);

#endif
