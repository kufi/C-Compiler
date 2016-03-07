
#ifndef PARSER_HEADER
#define PARSER_HEADER

#include "../Scanner/Scanner.h"

typedef struct Parser {

} Parser;

Parser *createParser(Grammar *grammar, ScannerConfig *config);

#endif
