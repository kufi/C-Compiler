#include <string.h>
#include <stdlib.h>
#include "Grammar.h"
#include "Parser.h"
#include "../Scanner/Scanner.h"

typedef struct LR1Item {
  int dotPosition;
  int symbolSize;
  char **symbols;
  char *lookahead;
} LR1Item;

typedef struct LR1ItemList {
  int usedItems;
  int itemSize;
  LR1Item **items;
} LR1ItemList;

typedef struct FirstSet {
  char *name;
  int usedTerminals;
  int terminalSize;
  char **terminals;
} FirstSet;

typedef struct FirstSets {
  int usedSets;
  int setSize;
  FirstSet **sets;
} FirstSets;

LR1Item *createLR1Item(Rule *rule)
{
  LR1Item *item = malloc(sizeof(LR1Item));
  item->dotPosition = 0;
  item->symbolSize = rule->usedSymbols;
  item->lookahead = "";

  for(int i = 0; i < rule->usedSymbols; i++)
  {
    item->symbols[i] = strdup(rule->symbols[i]);
  }

  return item;
}

LR1ItemList *createLR1Items(Production *production)
{
  LR1ItemList *list = malloc(sizeof(LR1ItemList));
  list->usedItems = 0;
  list->itemSize = production->usedRules;

  for(int i =0; i < production->usedRules; i++)
  {
    Rule *rule = production->rules[i];
    list->items[list->usedItems++] = createLR1Item(rule);
  }

  return list;
}

void addSet(FirstSets *sets, FirstSet *set)
{
  if(sets->usedSets == sets->setSize)
  {
    sets->setSize = sets->setSize * 2;
    sets->sets = realloc(sets->sets, sizeof(FirstSet *) * sets->setSize);
  }

  sets->sets[sets->usedSets++] = set;
}

FirstSet *createFirstSet(char *name)
{
  FirstSet *set = malloc(sizeof(FirstSet));
  set->usedTerminals = 0;
  set->terminalSize = 5;
  set->terminals = malloc(sizeof(char *) * set->terminalSize);
  set->name = name;
  return set;
}

void addTerminal(FirstSet *set, char *terminal)
{
  if(set->usedTerminals == set->terminalSize)
  {
    set->terminalSize = set->terminalSize * 2;
    set->terminals = realloc(set->terminals, sizeof(FirstSet *) * set->terminalSize);
  }

  set->terminals[set->usedTerminals++] = terminal;
}

FirstSet *findSet(FirstSets *sets, char *name)
{
  for(int i = 0; i < sets->usedSets; i++)
  {
    FirstSet *set = sets->sets[i];

    if(set->name == EMPTY && name == EMPTY) return set;
    if(set->name != EMPTY && strcmp(set->name, name) == 0) return set;
  }

  return NULL;
}

FirstSet *copySet(FirstSet *set)
{
  FirstSet *copy = malloc(sizeof(FirstSet));
  *copy = *set;
  copy->terminals = malloc(sizeof(char *) * set->terminalSize);
  memcpy(copy->terminals, set->terminals, sizeof(char *) * set->terminalSize);

  return copy;
}

bool setContainsEmpty(FirstSet *set)
{
  return set->terminals[set->usedTerminals - 1] == EMPTY;
}

void removeEmptyFromSet(FirstSet *set)
{
  if(set->terminals[set->usedTerminals - 1] == EMPTY)
  {
    --set->usedTerminals;
  }
}

bool setContainsTerminal(FirstSet *set, char *search)
{
  for(int i = 0; i < set->usedTerminals; i++)
  {
    char *terminal = set->terminals[i];

    if(terminal == EMPTY && search == EMPTY) return true;
    if(terminal != EMPTY && strcmp(terminal, search) == 0) return true;
  }

  return false;
}

void combineSets(FirstSet *first, FirstSet *second)
{
  if(second == NULL) return;

  for(int i = 0; i < second->usedTerminals; i++)
  {
    char *terminal = second->terminals[i];
    if(!setContainsTerminal(first, terminal)) addTerminal(first, terminal);
  }
}

FirstSets *createFirstSets(Grammar *grammar, ScannerConfig *config)
{
  FirstSets *sets = malloc(sizeof(FirstSets));
  sets->usedSets = 0;
  sets->setSize = 10;
  sets->sets = malloc(sizeof(FirstSet *) * sets->setSize);

  FirstSet *empty = createFirstSet(EMPTY);
  addTerminal(empty, EMPTY);
  addSet(sets, empty);

  for(int i = 0; i < config->usedCategories; i++)
  {
    Category *cat = config->categories[i];
    FirstSet *set = createFirstSet(cat->name);
    addTerminal(set, cat->name);
    addSet(sets, set);
  }

  for(int i = 0; i < grammar->usedProductions; i++)
  {
    Production *prod = grammar->productions[i];
    FirstSet *set = createFirstSet(prod->name);
    addSet(sets, set);
  }

  bool changing;

  do
  {
    changing = false;

    for(int i = 0; i < grammar->usedProductions; i++)
    {
      Production *prod = grammar->productions[i];

      FirstSet *prodSet = findSet(sets, prod->name);

      for(int j = 0; j < prod->usedRules; j++)
      {
        Rule *rule = prod->rules[j];

        FirstSet *rhs = copySet(findSet(sets, rule->symbols[0]));
        bool containsEmpty = setContainsEmpty(rhs);
        removeEmptyFromSet(rhs);

        int k = 1;
        for(; k < rule->usedSymbols && containsEmpty; k++)
        {
          char *symbol = rule->symbols[k];

          FirstSet *symbolSet = copySet(findSet(sets, symbol));
          containsEmpty = setContainsEmpty(symbolSet);
          removeEmptyFromSet(symbolSet);
          combineSets(rhs, symbolSet);
        }

        if(k == rule->usedSymbols && containsEmpty)
        {
          addTerminal(rhs, EMPTY);
        }

        int oldSize = prodSet->usedTerminals;
        combineSets(prodSet, rhs);

        changing |= oldSize != prodSet->usedTerminals;
      }
    }
  } while(changing);

  return sets;
}

LR1ItemList *closure(LR1ItemList *list)
{
  LR1ItemList *newList = malloc(sizeof(LR1ItemList));
  bool changing = false;

  do
  {

  } while(changing);

  return newList;
}

Parser *createParser(Grammar *grammar, ScannerConfig *config)
{
  Production *goalProduction = grammar->productions[0];

  FirstSets *sets = createFirstSets(grammar, config);

  for(int i = 0; i < sets->usedSets; i++)
  {
    FirstSet *set = sets->sets[i];
    printf("%s: ", set->name);

    for(int j = 0; j < set->usedTerminals; j++)
    {
      printf("%s, ", set->terminals[j]);
    }

    printf("\n");
  }

  LR1ItemList *cc0 = closure(createLR1Items(goalProduction));

  return NULL;
}
