#include "Scanner.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Nfa.h"
#include "SubsetConstruction.h"
#include "Hopcroft.h"

typedef struct StringBuilder {
  int used;
  int size;
  char *string;
} StringBuilder;

typedef struct DFAStateStack {
  int size;
  int pointer;
  DFAState **states;
} DFAStateStack;

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

StringBuilder createStringBuilder()
{
  StringBuilder builder;
  builder.used = 0;
  builder.size = 10;
  builder.string = malloc(sizeof(char) * builder.size);
  builder.string[0] = '\0';

  return builder;
}

DFAStateStack createStack()
{
  DFAStateStack stack;
  stack.pointer = 0;
  stack.size = 10;
  stack.states = malloc(sizeof(DFAState *) * stack.size);

  return stack;
}

void push(DFAStateStack *stack, DFAState *state)
{
  if(stack->pointer == stack->size)
  {
    stack->size = stack->size * 2;
    stack->states = realloc(stack->states, sizeof(DFAState *) * stack->size);
  }

  stack->states[stack->pointer++] = state;
}

DFAState *pop(DFAStateStack *stack)
{
  if(stack->pointer < 0) return NULL;
  return stack->states[--stack->pointer];
}

void appendChar(StringBuilder *builder, char c)
{
  if(builder->used  + 1 == builder->size)
  {
    builder->size = builder->size * 2;
    builder->string = realloc(builder->string, sizeof(char) * builder->size);
  }

  builder->string[builder->used++] = c;
  builder->string[builder->used] = '\0';
}

void removeLastChar(StringBuilder *builder)
{
  builder->string[--builder->used] = '\0';
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
  for(int i = 0; i < state->usedTransitions; i++)
  {
    DFATransition *trans = state->transitions[i];

    if(strchr(trans->characters, c) != NULL)
    {
      return trans->toState;
    }
  }

  return scanner->internalStates.error;
}

Category *categoryToId(Scanner *scanner, int categoryId)
{
  for(int i = 0; i < scanner->config->usedCategories; i++)
  {
    Category *category = scanner->config->categories[i];
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
  DFAStateStack stack = createStack();
  push(&stack, scanner->internalStates.bad);

  DFAState *state = scanner->dfa->start;

  while(state != scanner->internalStates.error)
  {
    char c = nextChar(scanner);
    appendChar(&lexeme, c);
    push(&stack, state);
    state = getNextState(scanner, state, c);
  }

  while(state != scanner->internalStates.bad && state->categoryId == -1)
  {
    state = pop(&stack);
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
