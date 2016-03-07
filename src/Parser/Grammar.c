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
  grammar->usedProductions = 0;
  grammar->productionSize = 10;
  grammar->productions = malloc(sizeof(Production *) * grammar->productionSize);

  return grammar;
}

void appendSymbol(Rule *rule, char *symbol)
{
  if(rule->usedSymbols == rule->symbolSize)
  {
    rule->symbolSize = rule->symbolSize * 2;
    rule->symbols = realloc(rule->symbols, sizeof(char *) * rule->symbolSize);
  }

  rule->symbols[rule->usedSymbols++] = symbol;
}

void addRule(Production *production, char *ruleString)
{
  Rule *rule = malloc(sizeof(Rule));

  if(ruleString == EMPTY)
  {
    rule->symbolSize = 1;
    rule->usedSymbols = 0;
    rule->symbols = malloc(sizeof(char *));
    appendSymbol(rule, EMPTY);
  }
  else
  {
    rule->usedSymbols = 0;
    rule->symbolSize = 5;
    rule->symbols = malloc(sizeof(char *) * rule->symbolSize);

    char *modifieableRule = strdup(ruleString);
    char *symbol = strtok(modifieableRule, " ");

    while (symbol != NULL)
    {
      appendSymbol(rule, symbol);
      symbol = strtok (NULL, " ");
    }
  }

  if(production->usedRules == production->ruleSize)
  {
    production->ruleSize = production->ruleSize * 2;
    production->rules = malloc(sizeof(Rule *) * production->ruleSize);
  }

  production->rules[production->usedRules++] = rule;
}

void addProduction(Grammar *grammar, char *name, char *rule, ...)
{
  Production *production = malloc(sizeof(Production));
  production->usedRules = 0;
  production->ruleSize = 10;
  production->rules = malloc(sizeof(Rule) * production->ruleSize);

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

  if(grammar->usedProductions == grammar->productionSize)
  {
    grammar->productionSize = grammar->productionSize * 2;
    grammar->productions = realloc(grammar->productions, sizeof(Production *) * grammar->productionSize);
  }

  grammar->productions[grammar->usedProductions++] = production;
}
