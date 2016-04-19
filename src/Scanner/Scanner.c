#include "Scanner.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Nfa.h"
#include "SubsetConstruction.h"
#include "Hopcroft.h"
#include "StringBuilder.h"
#include "Util/Collections/Stack.h"
#include "Debug/Printer.h"

ScannerConfig *createScannerConfig(int initialSize)
{
    ScannerConfig *config = malloc(sizeof(ScannerConfig));
    config->categories = arrayListCreate(initialSize, sizeof(Category *));
    config->nfaStateId = 0;
    config->nfa = NULL;

    return config;
}

void addCategory(ScannerConfig *config, char *name, char *regex)
{
  NFA *nfa = buildNFA(config->nfaStateId, regex);
  config->nfaStateId = nfa->final->id + 1;

  Category *category = malloc(sizeof(Category));
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

  arrayListPush(config->categories, category);
}

char *asciiCharset()
{
  char *ascii = malloc(sizeof(char) * 127);
  for(int i = 1; i < 127; i++) ascii[i-1] = i;
  ascii[126] = '\0';
  return ascii;
}

Scanner *createScanner(ScannerConfig *config, char *text)
{
  Scanner *scanner = malloc(sizeof(Scanner));
  DFA *dfa = hopcroft(subsetConstruction(config->nfa, asciiCharset()));
  printDFA(dfa);
  scanner->dfa = dfa;
  scanner->text = text;
  scanner->config = config;
  scanner->textPosition = 0;
  scanner->internalStates.bad = malloc(sizeof(DFAState));
  scanner->internalStates.bad->categoryId = -1;
  scanner->internalStates.error = malloc(sizeof(DFAState));
  scanner->internalStates.error->categoryId = -1;

  return scanner;
}

char nextChar(Scanner *scanner)
{
  return scanner->text[scanner->textPosition++];
}

void rollbackChar(Scanner *scanner)
{
  --scanner->textPosition;
}

DFAState *getNextState(Scanner *scanner, DFAState *state, char c)
{
  if(c == '\0') return scanner->internalStates.error;

  for(int i = 0; i < arrayListCount(state->transitions); i++)
  {
    DFATransition *trans = arrayListGet(state->transitions, i);

    if(strchr(trans->characters, c) != NULL) return trans->toState;
  }

  return scanner->internalStates.error;
}

Category *categoryToId(Scanner *scanner, int categoryId)
{
  for(int i = 0; i < scanner->config->categories->used; i++)
  {
    Category *category = arrayListGet(scanner->config->categories, i);
    if(category->id == categoryId) return category;
  }

  return NULL;
}

bool hasMoreWords(Scanner *scanner)
{
  return scanner->text[scanner->textPosition] != '\0';
}

Word nextWord(Scanner *scanner)
{
  StringBuilder lexeme = createStringBuilder();
  Stack *stack = stackCreate();
  stackPush(stack, scanner->internalStates.bad);

  DFAState *state = scanner->dfa->start;

  while(state != scanner->internalStates.error)
  {
    char c = nextChar(scanner);
    appendChar(&lexeme, c);
    stackPush(stack, state);
    state = getNextState(scanner, state, c);
  }

  while(state != scanner->internalStates.bad && state->categoryId == -1)
  {
    state = stackPop(stack);
    removeLastChar(&lexeme);
    rollbackChar(scanner);
  }

  Word word;

  if(state != scanner->internalStates.bad)
  {
    word.category = categoryToId(scanner, state->categoryId);
    word.lexeme = malloc(sizeof(char) * lexeme.used);
    strcpy(word.lexeme, lexeme.string);
  }
  else
  {
    word.category = NULL;
    word.lexeme = "";
  }

  return word;
}
