#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"

char *END = "END";
char *EMPTY = "EMPTY";

Grammar *createGrammar()
{
  Grammar *grammar = malloc(sizeof(Grammar));
  grammar->productions = arrayListCreate(10, sizeof(Production *));
  return grammar;
}

void appendSymbol(Rule *rule, char *symbol)
{
  arrayListPush(rule->symbols, symbol);
}

void addRule(Production *production, char *ruleString)
{
  Rule *rule = malloc(sizeof(Rule));

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

  arrayListPush(production->rules, rule);
}

void addProduction(Grammar *grammar, char *name, char *rule, ...)
{
  Production *production = malloc(sizeof(Production));
  production->rules = arrayListCreate(10, sizeof(Rule));

  production->name = name;

  char *ruleString = rule;
  if(ruleString != END) addRule(production, rule);

  va_list rules;
  va_start(rules, rule);

  while((ruleString = va_arg(rules, char *)) != END)
  {
    addRule(production, ruleString);
  }

  va_end(rules);

  arrayListPush(grammar->productions, production);
}

Production *getProductionForSymbol(Grammar *grammar, char *symbol)
{
  if(symbol == NULL) return NULL;

  for(int i = 0; i < arrayListCount(grammar->productions); i++)
  {
    Production *production = arrayListGet(grammar->productions, i);
    if(strcmp(production->name, symbol) == 0) return production;
  }

  return NULL;
}
