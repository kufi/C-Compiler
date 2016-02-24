#ifndef SCANNER_HEADER
#define SCANNER_HEADER

#include "Nfa.h"

typedef struct Category {
  int id;
  char *name;
} Category;

typedef struct ScannerConfig {
  int usedCategories;
  int categoriesSize;
  Category **categories;
  NFA *nfa;
  int nfaStateId;
} ScannerConfig;

ScannerConfig *createScannerConfig();

void addCategory(ScannerConfig *config, char *name, char *regex);

void createScanner(ScannerConfig *config);

#endif
