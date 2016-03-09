#include "SubsetConstruction.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct Configuration {
  int id;
  int size;
  NFAState **states;
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

typedef struct ConfigurationList {
  int usedConfigurations;
  int configurationSize;
  Configuration **configurations;
} ConfigurationList;

typedef struct TransitionList {
  int usedTransitions;
  int transitionSize;
  Transition **transitions;
} TransitionList;

Configuration *createConfiguration(int id, int size)
{
  Configuration *configuration = calloc(1, sizeof(Configuration));
  configuration->size = size;
  configuration->states = calloc(size, sizeof(NFAState *));

  return configuration;
}

bool configurationEquals(Configuration *first, Configuration *second)
{
  if(first->size != second->size) return false;

  for(int i = 0; i < first->size; i++)
  {
    bool idPresent = false;
    int stateIdFromFirst = first->states[i]->id;

    for(int j = 0; j < second->size; j++)
    {
      int stateIdFromSecond = second->states[j]->id;

      if(stateIdFromFirst == stateIdFromSecond)
      {
        idPresent = true;
        break;
      }
    }

    if(!idPresent) return false;
  }

  return true;
}

Configuration *tryGetExistingConfiguration(ConfigurationList *list, Configuration *check)
{
  for(int i = 0; i < list->usedConfigurations; i++)
  {
    if(configurationEquals(list->configurations[i], check)) return list->configurations[i];
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

  for(int i = 0; i < configuration->size; i++)
  {
    NFAState *state = configuration->states[i];
    outputStates[used++] = state;

    Configuration *subConfiguration = calloc(1, sizeof(Configuration));
    subConfiguration->states = calloc(2, sizeof(NFAState *));
    subConfiguration->size = 0;

    if(state->outChar1 == '\0' && state->out1 != NULL)
    {
      subConfiguration->states[subConfiguration->size++] = state->out1;
    }

    if(state->outChar2 == '\0' && state->out2 != NULL)
    {
      subConfiguration->states[subConfiguration->size++] = state->out2;
    }

    Configuration *subConfigurationClosure = eClosure(subConfiguration);

    if(used + subConfigurationClosure->size >= outputSize)
    {
        outputSize = (used + subConfigurationClosure->size) * 2;
        outputStates = realloc(outputStates, sizeof(NFAState *) * outputSize);
    }

    for(int j = 0; j < subConfigurationClosure->size; j++)
    {
      outputStates[used++] = subConfigurationClosure->states[j];
    }
  }

  Configuration *result = calloc(1, sizeof(Configuration));
  result->size = used;
  result->states = malloc(sizeof(NFAState *) * used);
  result->states = outputStates;

  return result;
}

Configuration *delta(Configuration *configuration, char c)
{
  Configuration *result = calloc(1, sizeof(Configuration));
  result->size = 0;
  result->states = malloc(sizeof(NFAState *) * configuration->size * 2);

  for(int i = 0; i < configuration->size; i++)
  {
    NFAState *checkState = configuration->states[i];

    if(checkState->outChar1 == c)
    {
      result->states[result->size++] = checkState->out1;
    }

    if(checkState->outChar2 == c)
    {
      result->states[result->size++] = checkState->out2;
    }
  }

  return result;
}

int getMinimalCategoryId(Configuration *config)
{
  int minimalCategoryId = -1;

  for(int i = 0; i < config->size; i++)
  {
    NFAState *nfaState = config->states[i];

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

ConfigurationList *createConfigurationList()
{
  ConfigurationList *list = malloc(sizeof(ConfigurationList));
  list->usedConfigurations = 0;
  list->configurationSize = 10;
  list->configurations = malloc(sizeof(Configuration *) * list->configurationSize);
  return list;
}

void addConfiguration(ConfigurationList *list, Configuration *configuration)
{
  if(list->usedConfigurations == list->configurationSize)
  {
    list->configurationSize = list->configurationSize * 2;
    list->configurations = realloc(list->configurations, sizeof(Configuration *) * list->configurationSize);
  }

  list->configurations[list->usedConfigurations++] = configuration;
}

void createStatesFromConfigurations(DFA *dfa, ConfigurationList *configurations)
{
  dfa->stateSize = configurations->usedConfigurations;
  dfa->states = malloc(sizeof(DFAState *) * configurations->usedConfigurations);

  for(int i = 0; i < configurations->usedConfigurations; i++)
  {
    Configuration *config = configurations->configurations[i];
    dfa->states[config->id] = createDFAState(config->id, getMinimalCategoryId(config));
  }
}

void createDFATransitions(DFA *dfa, TransitionList *transitions)
{
  for(int i = 0; i < transitions->usedTransitions; i++)
  {
    Transition *trans = transitions->transitions[i];
    DFAState *from = dfa->states[trans->from->id];
    DFAState *to = dfa->states[trans->to->id];

    DFATransition *dfaTrans = malloc(sizeof(DFATransition));
    dfaTrans->characters = strdup(&trans->character);
    dfaTrans->toState = to;

    if(from->usedTransitions == from->transitionSize)
    {
      from->transitionSize = from->transitionSize * 2;
      from->transitions = realloc(from->transitions, sizeof(DFATransition *) * from->transitionSize);
    }

    from->transitions[from->usedTransitions++] = dfaTrans;
  }
}

DFA *subsetConstruction(NFA *nfa, char *characterSet)
{
  Configuration *startConfiguration = createConfiguration(0, 1);
  startConfiguration->states[0] = nfa->start;

  Configuration *q0 = eClosure(startConfiguration);

  ConfigurationList *configurations = createConfigurationList();
  addConfiguration(configurations, q0);

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

      if(charConfiguration->size == 0) continue;

      Configuration *existingCharConfiguration = tryGetExistingConfiguration(configurations, charConfiguration);

      if(existingCharConfiguration == NULL)
      {
        charConfiguration->id = configurations->usedConfigurations;

        addConfiguration(configurations, charConfiguration);

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
  dfa->start = dfa->states[0];

  return dfa;
}
