#include "Scanner.h"
#include <stdlib.h>
#include <stdio.h>

ScannerConfig *createScannerConfig(int initialSize)
{
    ScannerConfig *config = malloc(sizeof(ScannerConfig));
    config->usedCategories = 0;
    config->categoriesSize = initialSize;
    config->categories = malloc(sizeof(Category *) * config->categoriesSize);
    config->nfaStateId = 0;

    return config;
}

void addCategory(ScannerConfig *config, char *name, char *regex)
{
  NFA *nfa = buildNFA(config->nfaStateId, regex);
  config->nfaStateId = nfa->final->id + 1;

  Category *category = malloc(sizeof(Category));
  category->id = config->usedCategories;
  category->id = nfa->final->id;
  category->name = name;

  if(config->nfa == NULL)
  {
    config->nfa = nfa;
  }
  else
  {
    NFAState *start = createState(config->nfaStateId++, nfa->start, '\0', config->nfa->start, '\0');

    NFA *concatNFA = malloc(sizeof(NFA));
    concatNFA->start = start;
    concatNFA->final = NULL;

    config->nfa = concatNFA;
  }

  if(config->usedCategories == config->categoriesSize)
  {
    config->categoriesSize = config->categoriesSize * 2;
    config->categories = realloc(config->categories, sizeof(Category *) * config->categoriesSize);
  }

  config->categories[config->usedCategories++] = category;
}

void createScanner(ScannerConfig *config);
