#include "FirstSet.h"
#include "../Util/Collections/HashSet.h"

FirstSet *createFirstSet(char *name)
{
  FirstSet *set = calloc(1, sizeof(FirstSet));
  set->terminals = arrayListCreate(5, sizeof(char *));
  set->name = name;
  return set;
}

void addTerminal(FirstSet *set, char *terminal)
{
  arrayListPush(set->terminals, terminal);
}

void addSet(HashMap *sets, FirstSet *set)
{
  hashMapSet(sets, set->name, set);
}

FirstSet *findSet(HashSet *sets, char *name)
{
  return hashMapGet(sets, name);
}

FirstSet *copySet(FirstSet *set)
{
  FirstSet *copy = calloc(1, sizeof(FirstSet));
  *copy = *set;
  copy->terminals = arrayListCreate(arrayListCount(set->terminals), sizeof(char *));
  for(int i = 0; i < arrayListCount(set->terminals); i++)
  {
    arrayListPush(copy->terminals, strdup(arrayListGet(set->terminals, i)));
  }

  return copy;
}

bool setContainsEmpty(FirstSet *set)
{
  if(arrayListCount(set->terminals) == 0) return false;
  return arrayListGet(set->terminals, arrayListCount(set->terminals)) == EMPTY;
}

void removeEmptyFromSet(FirstSet *set)
{
  if(setContainsEmpty(set))
  {
    arrayListPop(set->terminals);
  }
}

bool setContainsTerminal(FirstSet *set, char *search)
{
  for(int i = 0; i < arrayListCount(set->terminals); i++)
  {
    char *terminal = arrayListGet(set->terminals, i);

    if(terminal == EMPTY && search == EMPTY) return true;
    if(terminal != EMPTY && strcmp(terminal, search) == 0) return true;
  }

  return false;
}

void combineSets(FirstSet *first, FirstSet *second)
{
  if(second == NULL) return;

  for(int i = 0; i < arrayListCount(second->terminals); i++)
  {
    char *terminal = arrayListGet(second->terminals, i);
    if(!setContainsTerminal(first, terminal)) addTerminal(first, terminal);
  }
}

HashMap *createFirstSets(Grammar *grammar, ScannerConfig *config)
{
  HashMap *sets = hashMapCreate(10);

  FirstSet *empty = createFirstSet(EMPTY);
  addTerminal(empty, EMPTY);
  addSet(sets, empty);

  FirstSet *end = createFirstSet(END);
  addTerminal(end, END);
  addSet(sets, end);

  for(int i = 0; i < config->categories->used; i++)
  {
    Category *cat = arrayListGet(config->categories, i);
    FirstSet *set = createFirstSet(cat->name);
    addTerminal(set, cat->name);
    addSet(sets, set);
  }

  for(int i = 0; i < arrayListCount(grammar->productions); i++)
  {
    Production *prod = arrayListGet(grammar->productions, i);
    addSet(sets, createFirstSet(prod->name));
  }

  bool changing;

  do
  {
    changing = false;

    for(int i = 0; i < arrayListCount(grammar->productions); i++)
    {
      Production *prod = arrayListGet(grammar->productions, i);

      FirstSet *prodSet = findSet(sets, prod->name);

      for(int j = 0; j < arrayListCount(prod->rules); j++)
      {
        Rule *rule = arrayListGet(prod->rules, j);

        FirstSet *rhs = copySet(findSet(sets, arrayListGet(rule->symbols, 0)));
        bool containsEmpty = setContainsEmpty(rhs);
        removeEmptyFromSet(rhs);

        int k = 1;
        for(; k < arrayListCount(rule->symbols) && containsEmpty; k++)
        {
          char *symbol = arrayListGet(rule->symbols, k);

          FirstSet *symbolSet = copySet(findSet(sets, symbol));
          containsEmpty = setContainsEmpty(symbolSet);
          removeEmptyFromSet(symbolSet);
          combineSets(rhs, symbolSet);
        }

        if(k == arrayListCount(rule->symbols) && containsEmpty)
        {
          addTerminal(rhs, EMPTY);
        }

        int oldSize = arrayListCount(prodSet->terminals);
        combineSets(prodSet, rhs);

        changing |= oldSize != arrayListCount(prodSet->terminals);
      }
    }
  } while(changing);

  return sets;
}

FirstSet *getFirstSetForLookaheads(char **lookaheads, int lookaheadSize, HashMap *sets)
{
  FirstSet *resultingSet = createFirstSet("");

  for(int i = 0; i < lookaheadSize; i++)
  {
    char *lookahead = lookaheads[i];

    FirstSet *set = findSet(sets, lookahead);
    combineSets(resultingSet, set);

    if(!setContainsEmpty(set)) break;
  }

  return resultingSet;
}
