#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Dfa.h"

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

Configuration *createConfiguration(int id, int size)
{
  Configuration *configuration = malloc(sizeof(Configuration));
  configuration->size = size;
  configuration->states = malloc(sizeof(NFAState *) * size);

  return configuration;
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

    Configuration *subConfiguration = malloc(sizeof(Configuration));
    subConfiguration->states = malloc(sizeof(NFAState *) * 2);
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

  Configuration *result = malloc(sizeof(Configuration));
  result->size = used;
  result->states = malloc(sizeof(NFAState *) * used);
  result->states = outputStates;

  return result;
}

Configuration *delta(Configuration *configuration, char c)
{
  Configuration *result = malloc(sizeof(Configuration));
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

Configuration *tryGetExistingConfiguration(Configuration **usedConfigurations, int usedConfigurationSize, Configuration *check)
{
  for(int i = 0; i < usedConfigurationSize; i++)
  {
    if(configurationEquals(usedConfigurations[i], check))
    {
      return usedConfigurations[i];
    }
  }

  return NULL;
}

ConfigurationWorklistItem *createWorklistItem(Configuration *configuration, ConfigurationWorklistItem *next)
{
  ConfigurationWorklistItem *worklistItem = malloc(sizeof(ConfigurationWorklistItem *));
  worklistItem->configuration = configuration;
  worklistItem->next = next;

  return worklistItem;
}

Transition *createTransition(Configuration *from, Configuration *to, char c)
{
  Transition *transition = malloc(sizeof(Transition));
  transition->from = from;
  transition->to = to;
  transition->character = c;

  return transition;
}

DFA *subsetConstruction(NFA *nfa, char *characterSet, int characterSetSize)
{
  Configuration *startConfiguration = createConfiguration(0, 1);
  startConfiguration->states[0] = nfa->start;

  Configuration *q0 = eClosure(startConfiguration);

  int usedConfigurations = 0;
  int configurationSize = 10;
  Configuration **configurations = malloc(sizeof(Configuration *) * configurationSize);
  configurations[usedConfigurations++] = q0;

  int usedTransitions = 0;
  int transitionSize = 10;
  Transition **transitions = malloc(sizeof(Transition *) * transitionSize);

  ConfigurationWorklistItem *nextWorklistItem = createWorklistItem(startConfiguration, NULL);
  ConfigurationWorklistItem *lastWorklistItem = nextWorklistItem;

  while(nextWorklistItem != NULL)
  {
    ConfigurationWorklistItem *check = nextWorklistItem;

    for(int i = 0; i < characterSetSize; i++)
    {
      char character = characterSet[i];
      Configuration *charConfiguration = eClosure(delta(check->configuration, character));

      if(charConfiguration->size == 0) continue;

      Configuration *existingCharConfiguration = tryGetExistingConfiguration(configurations, usedConfigurations, charConfiguration);

      if(existingCharConfiguration == NULL)
      {
        charConfiguration->id = usedConfigurations;
        configurations[usedConfigurations++] = charConfiguration;

        if(usedConfigurations >= configurationSize)
        {
          configurations = realloc(configurations, sizeof(Configuration *) * configurationSize * 2);
          configurationSize = configurationSize * 2;
        }

        ConfigurationWorklistItem *newWorklistItem = createWorklistItem(charConfiguration, NULL);
        lastWorklistItem->next = newWorklistItem;
        lastWorklistItem = newWorklistItem;
      }
      else
      {
        charConfiguration = existingCharConfiguration;
      }

      transitions[usedTransitions++] = createTransition(check->configuration, charConfiguration, character);

      if(usedTransitions >= transitionSize)
      {
        transitions = realloc(transitions, sizeof(Transition *) * transitionSize * 2);
        transitionSize = transitionSize * 2;
      }
    }

    nextWorklistItem = nextWorklistItem->next;
  }

  DFA *dfa = malloc(sizeof(DFA *));
  dfa->numberOfStates = usedConfigurations;

  dfa->transitions = malloc(sizeof(DFATransition **) * usedConfigurations);
  for(int i = 0; i < usedConfigurations; i++)
  {
    dfa->transitions[i] = malloc(sizeof(DFATransition *) * usedConfigurations);
  }

  for(int i = 0; i < usedTransitions; i++)
  {
    Transition *trans = transitions[i];

    DFATransition *dfaTrans = dfa->transitions[trans->from->id][trans->to->id];

    if(dfaTrans == NULL)
    {
      dfaTrans = malloc(sizeof(DFATransition));
      dfaTrans->characters = malloc(sizeof(char));
      dfaTrans->characterSize = 1;
      dfa->transitions[trans->from->id][trans->to->id] = dfaTrans;
    }
    else
    {
      dfaTrans->characterSize = dfaTrans->characterSize + 1;
      dfaTrans->characters = realloc(dfaTrans->characters, dfaTrans->characterSize);
    }

    dfaTrans->characters[dfaTrans->characterSize - 1] = trans->character;
  }

  return dfa;
}
