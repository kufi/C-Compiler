#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"

char *END = "END";
char *EMPTY = "EMPTY";

Grammar *createGrammar(ScannerConfig *config)
{
  Grammar *grammar = malloc(sizeof(Grammar));
  grammar->productions = hashMapCreate();
  grammar->scannerConfig = config;
  return grammar;
}

void appendSymbol(Rule *rule, char *symbol)
{
  arrayListPush(rule->symbols, symbol);
}

void addRule(ArrayList *productionRules, char *productionName, char *ruleString, ReduceAction reduceAction)
{
  Rule *rule = malloc(sizeof(Rule));
  rule->productionName = productionName;
  rule->reduceAction = reduceAction;

  if(ruleString == EMPTY)
  {
    rule->symbols = arrayListCreate(1, sizeof(char *));
    appendSymbol(rule, EMPTY);
  }
  else
  {
    rule->symbols = arrayListCreate(10, sizeof(char *));

    char *modifieableRule = strdup(ruleString);
    char *symbol = strtok(modifieableRule, " ");

    while (symbol != NULL)
    {
      appendSymbol(rule, symbol);
      symbol = strtok (NULL, " ");
    }
  }

  arrayListPush(productionRules, rule);
}

void addProduction(Grammar *grammar, char *name, char *rule, ReduceAction reduceAction)
{
  ArrayList *productionRules = hashMapGet(grammar->productions, name);

  if(productionRules == NULL)
  {
    productionRules = arrayListCreate(10, sizeof(Rule *));
    hashMapSet(grammar->productions, name, productionRules);
  }

  addRule(productionRules, name, rule, reduceAction);
}

ArrayList *getProductionForSymbol(Grammar *grammar, char *symbol)
{
  if(symbol == NULL) return NULL;
  return hashMapGet(grammar->productions, symbol);
}
