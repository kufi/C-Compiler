#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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

typedef struct Partition {
  int categoryId;
  int usedStates;
  int stateSize;
  DFAState **states;
} Partition;

typedef struct PartitionSplit {
  bool split;
  Partition *first;
  Partition *second;
} PartitionSplit;

typedef struct PartitionTransition {
  char character;
  Partition *partition;
} PartitionTransition;

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

DFAState *createDFAState(int id, int categoryId)
{
  DFAState *state = malloc(sizeof(DFAState));
  state->id = id;
  state->categoryId = categoryId;
  state->usedTransitions = 0;
  state->transitionSize = 5;
  state->transitions = malloc(sizeof(DFATransition *) * state->transitionSize);

  return state;
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

DFA *subsetConstruction(NFA *nfa, char *characterSet)
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

  DFA *dfa = malloc(sizeof(DFA));

  dfa->stateSize = usedConfigurations;
  dfa->states = malloc(sizeof(DFAState *) * usedConfigurations);

  for(int i = 0; i < usedConfigurations; i++)
  {
    Configuration *config = configurations[i];
    dfa->states[config->id] = createDFAState(config->id, getMinimalCategoryId(config));
  }

  for(int i = 0; i < usedTransitions; i++)
  {
    Transition *trans = transitions[i];
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

  dfa->start = dfa->states[0];

  return dfa;
}

Partition *createPartition(const int categoryId)
{
  Partition *partition = malloc(sizeof(Partition));
  partition->categoryId = categoryId;
  partition->usedStates = 0;
  partition->stateSize = 10;
  partition->states = malloc(sizeof(DFAState *) * partition->stateSize);

  return partition;
}

Partition *tryGetExistingPartition(int categoryId, int partitionSize, Partition **partitions)
{
  for(int i = 0; i < partitionSize; i++)
  {
    Partition *partition = partitions[i];

    if(partition->categoryId == categoryId) return partition;
  }

  return NULL;
}

void addStateToPartition(Partition *partition, DFAState *state)
{
  if(partition->usedStates == partition->stateSize)
  {
    partition->stateSize = partition->stateSize * 2;
    partition->states = realloc(partition->states, sizeof(DFAState *) * partition->stateSize);
  }

  partition->states[partition->usedStates++] = state;
}

Partition *findPartitionToState(DFAState *state, int partitionSize, Partition **partitions)
{
  for(int i = 0; i < partitionSize; i++)
  {
    Partition *partition = partitions[i];

    for(int j = 0; j < partition->usedStates; j++)
    {
      if(partition->states[j] == state) return partition;
    }
  }

  return NULL;
}

PartitionTransition *tryFindPartitionTransition(char character, int transitionSize, PartitionTransition **transitions)
{
  for(int i = 0; i < transitionSize; i++)
  {
    if(transitions[i]->character == character) return transitions[i];
  }

  return NULL;
}

PartitionSplit *split(Partition *partition, int partitionSize, Partition **partitions)
{
  PartitionSplit *split = malloc(sizeof(PartitionSplit));

  split->split = false;
  split->first = createPartition(partition->categoryId);
  split->second = createPartition(partition->categoryId);

  DFAState *firstState = partition->states[0];
  addStateToPartition(split->first, firstState);

  PartitionTransition **partitionTransitions = malloc(sizeof(PartitionTransition *) * firstState->usedTransitions);

  for(int i = 0; i < firstState->usedTransitions; i++)
  {
    DFATransition *trans = firstState->transitions[i];

    partitionTransitions[i] = malloc(sizeof(PartitionTransition));
    partitionTransitions[i]->character = trans->characters[0];
    partitionTransitions[i]->partition = findPartitionToState(trans->toState, partitionSize, partitions);
  }

  for(int i = 1; i < partition->usedStates; i++)
  {
    DFAState *state = partition->states[i];

    if(state->usedTransitions != firstState->usedTransitions)
    {
      addStateToPartition(split->second, state);
      split->split = true;
      continue;
    }

    bool equivalentTransitions = true;

    for(int j = 0; j < state->usedTransitions; j++)
    {
      DFATransition *trans = state->transitions[j];

      PartitionTransition *transition = tryFindPartitionTransition(trans->characters[0], state->usedTransitions, partitionTransitions);

      if(transition == NULL)
      {
        equivalentTransitions = false;
        break;
      }

      if(transition->partition != findPartitionToState(trans->toState, partitionSize, partitions))
      {
        equivalentTransitions = false;
        break;
      }
    }

    if(equivalentTransitions)
    {
      addStateToPartition(split->first, state);
    }
    else
    {
      addStateToPartition(split->second, state);
      split->split = true;
    }
  }

  return split;
}

int findPartitionIdToPartition(Partition *partition, int partitionSize, Partition **partitions)
{
  for(int i = 0; i < partitionSize; i++)
  {
    if(partitions[i] == partition) return i;
  }

  return -1;
}

DFATransition *tryFindTransition(DFAState *state, DFAState *toState)
{
  for(int i = 0; i < state->usedTransitions; i++)
  {
    DFATransition *trans = state->transitions[i];

    if(trans->toState == toState) return trans;
  }

  return NULL;
}

DFA *hopcroft(DFA *dfa)
{
  int usedPartitions = 0;
  int partitionsSize = 10;
  Partition **partitions = malloc(sizeof(Partition *) * partitionsSize);

  for(int i = 0; i < dfa->stateSize; i++)
  {
    DFAState *state = dfa->states[i];

    Partition *partition = tryGetExistingPartition(state->categoryId, usedPartitions, partitions);

    if(partition == NULL)
    {
      partition = createPartition(state->categoryId);
      partitions[usedPartitions++] = partition;
    }

    addStateToPartition(partition, state);

    if(usedPartitions == partitionsSize)
    {
      partitionsSize = partitionsSize * 2;
      partitions = realloc(partitions, sizeof(Partition *) * partitionsSize);
    }
  }

  bool hasNewPartitions;

  do
  {
    hasNewPartitions = false;

    int newUsedPartitions = 0;

    int newPartitionsSize = usedPartitions;
    Partition **newPartitions = malloc(sizeof(Partition *) * newPartitionsSize);

    for(int i = 0; i < usedPartitions; i++)
    {
      PartitionSplit *partitionSplit = split(partitions[i], usedPartitions, partitions);

      if(newUsedPartitions == newPartitionsSize)
      {
        newPartitionsSize = newPartitionsSize * 2;
        newPartitions = realloc(newPartitions, sizeof(Partition *) * newPartitionsSize);
      }

      newPartitions[newUsedPartitions++] = partitionSplit->first;

      if(partitionSplit->split)
      {
        hasNewPartitions = true;
        newPartitions[newUsedPartitions++] = partitionSplit->second;
      }
    }

    partitions = newPartitions;
    usedPartitions = newUsedPartitions;
    partitionsSize = newPartitionsSize;
  } while(hasNewPartitions);

  DFA *newDfa = malloc(sizeof(DFA));

  newDfa->stateSize = usedPartitions;
  newDfa->states = malloc(sizeof(DFAState *) * usedPartitions);

  for(int i = 0; i < usedPartitions; i++)
  {
    DFAState *state = partitions[i]->states[0];
    newDfa->states[i] = createDFAState(i, state->categoryId);
    newDfa->states[i]->usedTransitions = 0;
    newDfa->states[i]->transitionSize = state->usedTransitions;
    newDfa->states[i]->transitions = malloc(sizeof(DFATransition *) * state->usedTransitions);
  }

  for(int i = 0; i < usedPartitions; i++)
  {
    DFAState *newState = newDfa->states[i];
    DFAState *state = partitions[i]->states[0];

    for(int j = 0; j < state->usedTransitions; j++)
    {
      DFATransition *trans = state->transitions[j];

      Partition *partition = findPartitionToState(trans->toState, usedPartitions, partitions);
      int partitionId = findPartitionIdToPartition(partition, usedPartitions, partitions);

      DFAState *toState = newDfa->states[partitionId];

      DFATransition *partitionTransition = tryFindTransition(newState, toState);

      if(partitionTransition == NULL)
      {
        partitionTransition = malloc(sizeof(DFATransition));
        partitionTransition->characters = "";
        newState->transitions[newState->usedTransitions++] = partitionTransition;
      }

      partitionTransition->toState = newDfa->states[partitionId];

      char *newCharacters = malloc(sizeof(char) * strlen(partitionTransition->characters) + 2);
      strcpy(newCharacters, partitionTransition->characters);
      partitionTransition->characters = strncat(newCharacters, trans->characters, 1);
    }
  }

  Partition *startPartition = findPartitionToState(dfa->states[0], usedPartitions, partitions);
  int partitionId  = findPartitionIdToPartition(startPartition, usedPartitions, partitions);

  newDfa->start = newDfa->states[partitionId];

  return newDfa;
}
