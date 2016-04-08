#ifndef SCANNER_HEADER
#define SCANNER_HEADER

#include "Dfa.h"
#include "Util/Collections/ArrayList.h"

typedef struct Category {
  int id;
  char *name;
} Category;

typedef struct ScannerConfig {
  ArrayList *categories;
  NFA *nfa;
  int nfaStateId;
} ScannerConfig;

typedef struct Scanner {
  DFA *dfa;
  char *text;
  int textPosition;
  ScannerConfig *config;
  struct InternalStates
  {
    DFAState *bad;
    DFAState *error;
  } internalStates;
} Scanner;

typedef struct Word {
  char *lexeme;
  Category *category;
} Word;

ScannerConfig *createScannerConfig(int initialSize);

void addCategory(ScannerConfig *config, char *name, char *regex);

Scanner *createScanner(ScannerConfig *config, char *text);

bool hasMoreWords(Scanner *scanner);

Word nextWord(Scanner *scanner);

#endif
