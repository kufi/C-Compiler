#include "SubsetConstruction.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../Util/Collections/ArrayList.h"

typedef struct Configuration {
  int id;
  ArrayList *states;
} Configuration;

typedef struct ConfigurationWorklistItem {
  Configuration *configuration;
  struct ConfigurationWorklistItem *next;
} ConfigurationWorklistItem;

typedef struct Transition {
  Configuration *from;
  Configuration *to;
  char character;
} Transition;

typedef struct TransitionList {
  int usedTransitions;
  int transitionSize;
  Transition **transitions;
} TransitionList;

Configuration *createConfiguration(int id, int size)
{
  Configuration *configuration = calloc(1, sizeof(Configuration));
  configuration->states = arrayListCreate(size, sizeof(NFAState *));

  return configuration;
}

bool configurationEquals(Configuration *first, Configuration *second)
{
  if(arrayListCount(first->states) != arrayListCount(second->states)) return false;

  for(int i = 0; i < arrayListCount(first->states); i++)
  {
    bool idPresent = false;
    NFAState *firstState = arrayListGet(first->states, i);

    for(int j = 0; j < arrayListCount(second->states); j++)
    {
      NFAState *secondState = arrayListGet(second->states, j);

      if(firstState->id == secondState->id)
      {
        idPresent = true;
        break;
      }
    }

    if(!idPresent) return false;
  }

  return true;
}

Configuration *tryGetExistingConfiguration(ArrayList *list, Configuration *check)
{
  for(int i = 0; i < arrayListCount(list); i++)
  {
    Configuration *config = arrayListGet(list, i);
    if(configurationEquals(config, check)) return config;
  }

  return NULL;
}

ConfigurationWorklistItem *createWorklistItem(Configuration *configuration, ConfigurationWorklistItem *next)
{
  ConfigurationWorklistItem *worklistItem = malloc(sizeof(ConfigurationWorklistItem));
  worklistItem->configuration = configuration;
  worklistItem->next = next;

  return worklistItem;
}

Configuration *eClosure(Configuration *configuration)
{
  int used = 0;
  int outputSize = 10;
  NFAState **outputStates = malloc(sizeof(NFAState *) * outputSize);

  for(int i = 0; i < arrayListCount(configuration->states); i++)
  {
    NFAState *state = arrayListGet(configuration->states, i);
    outputStates[used++] = state;

    Configuration *subConfiguration = calloc(1, sizeof(Configuration));
    subConfiguration->states = arrayListCreate(2, sizeof(NFAState *));

    if(state->outChar1 == '\0' && state->out1 != NULL)
    {
      arrayListPush(subConfiguration->states, state->out1);
    }

    if(state->outChar2 == '\0' && state->out2 != NULL)
    {
      arrayListPush(subConfiguration->states, state->out2);
    }

    Configuration *subConfigurationClosure = eClosure(subConfiguration);

    if(used + arrayListCount(subConfigurationClosure->states) >= outputSize)
    {
        outputSize = (used + arrayListCount(subConfigurationClosure->states)) * 2;
        outputStates = realloc(outputStates, sizeof(NFAState *) * outputSize);
    }

    for(int j = 0; j < arrayListCount(subConfigurationClosure->states); j++)
    {
      outputStates[used++] = arrayListGet(subConfigurationClosure->states, j);
    }
  }

  Configuration *result = calloc(1, sizeof(Configuration));
  result->states = arrayListCreate(used, sizeof(NFAState *));
  result->states->used = used;
  result->states->items = (void **)outputStates;

  return result;
}

Configuration *delta(Configuration *configuration, char c)
{
  Configuration *result = calloc(1, sizeof(Configuration));
  result->states = arrayListCreate(arrayListCount(configuration->states) * 2, sizeof(NFAState *));

  for(int i = 0; i < arrayListCount(configuration->states); i++)
  {
    NFAState *checkState = arrayListGet(configuration->states, i);

    if(checkState->outChar1 == c)
    {
      arrayListPush(result->states, checkState->out1);
    }

    if(checkState->outChar2 == c)
    {
      arrayListPush(result->states, checkState->out2);
    }
  }

  return result;
}

int getMinimalCategoryId(Configuration *config)
{
  int minimalCategoryId = -1;

  for(int i = 0; i < arrayListCount(config->states); i++)
  {
    NFAState *nfaState = arrayListGet(config->states, i);

    if(nfaState->accepting && (nfaState->id < minimalCategoryId || minimalCategoryId == -1))
    {
      minimalCategoryId = nfaState->id;
    }
  }

  return minimalCategoryId;
}

Transition *createTransition(Configuration *from, Configuration *to, char c)
{
  Transition *transition = calloc(1, sizeof(Transition));
  transition->from = from;
  transition->to = to;
  transition->character = c;

  return transition;
}

TransitionList *createTransitionList()
{
  TransitionList *list = malloc(sizeof(TransitionList));
  list->usedTransitions = 0;
  list->transitionSize = 10;
  list->transitions = malloc(sizeof(Transition *) * list->transitionSize);
  return list;
}

void addTransition(TransitionList *list, Transition *transition)
{
  if(list->usedTransitions == list->transitionSize)
  {
    list->transitionSize = list->transitionSize * 2;
    list->transitions = realloc(list->transitions, sizeof(Transition *) * list->transitionSize);
  }

  list->transitions[list->usedTransitions++] = transition;
}

void createStatesFromConfigurations(DFA *dfa, ArrayList *configurations)
{
  dfa->states = arrayListCreate(arrayListCount(configurations), sizeof(DFAState *));

  for(int i = 0; i < arrayListCount(configurations); i++)
  {
    Configuration *config = arrayListGet(configurations, i);
    arrayListPush(dfa->states, createDFAState(config->id, getMinimalCategoryId(config)));
  }
}

void createDFATransitions(DFA *dfa, TransitionList *transitions)
{
  for(int i = 0; i < transitions->usedTransitions; i++)
  {
    Transition *trans = transitions->transitions[i];
    DFAState *from = arrayListGet(dfa->states, trans->from->id);
    DFAState *to = arrayListGet(dfa->states, trans->to->id);

    DFATransition *dfaTrans = malloc(sizeof(DFATransition));
    dfaTrans->characters = strdup(&trans->character);
    dfaTrans->toState = to;

    arrayListPush(from->transitions, dfaTrans);
  }
}

DFA *subsetConstruction(NFA *nfa, char *characterSet)
{
  Configuration *startConfiguration = createConfiguration(0, 1);
  arrayListPush(startConfiguration->states, nfa->start);

  Configuration *q0 = eClosure(startConfiguration);

  ArrayList *configurations = arrayListCreate(10, sizeof(Configuration *));
  arrayListPush(configurations, q0);

  TransitionList *transitions = createTransitionList();

  ConfigurationWorklistItem *nextWorklistItem = createWorklistItem(q0, NULL);
  ConfigurationWorklistItem *lastWorklistItem = nextWorklistItem;

  while(nextWorklistItem != NULL)
  {
    ConfigurationWorklistItem *check = nextWorklistItem;

    for(int i = 0; i < strlen(characterSet); i++)
    {
      char character = characterSet[i];
      Configuration *charConfiguration = eClosure(delta(check->configuration, character));

      if(arrayListCount(charConfiguration->states) == 0) continue;

      Configuration *existingCharConfiguration = tryGetExistingConfiguration(configurations, charConfiguration);

      if(existingCharConfiguration == NULL)
      {
        charConfiguration->id = arrayListCount(configurations);
        arrayListPush(configurations, charConfiguration);

        ConfigurationWorklistItem *newWorklistItem = createWorklistItem(charConfiguration, NULL);
        lastWorklistItem->next = newWorklistItem;
        lastWorklistItem = newWorklistItem;
      }
      else
      {
        charConfiguration = existingCharConfiguration;
      }

      addTransition(transitions, createTransition(check->configuration, charConfiguration, character));
    }

    nextWorklistItem = nextWorklistItem->next;
  }

  DFA *dfa = malloc(sizeof(DFA));
  createStatesFromConfigurations(dfa, configurations);
  createDFATransitions(dfa, transitions);
  dfa->start = arrayListGet(dfa->states, 0);

  return dfa;
}
