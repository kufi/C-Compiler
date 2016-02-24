#include "Hopcroft.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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

PartitionSplit *createPartitionSplit(int categoryId)
{
  PartitionSplit *split = malloc(sizeof(PartitionSplit));

  split->split = false;
  split->first = createPartition(categoryId);
  split->second = createPartition(categoryId);
  return split;
}

PartitionTransition **partitionTransitionsForState(DFAState *state, int partitionSize, Partition **partitions)
{
  PartitionTransition **partitionTransitions = malloc(sizeof(PartitionTransition *) * state->usedTransitions);

  for(int i = 0; i < state->usedTransitions; i++)
  {
    DFATransition *trans = state->transitions[i];

    partitionTransitions[i] = malloc(sizeof(PartitionTransition));
    partitionTransitions[i]->character = trans->characters[0];
    partitionTransitions[i]->partition = findPartitionToState(trans->toState, partitionSize, partitions);
  }

  return partitionTransitions;
}

bool transitionsEquivalent(DFAState *state, PartitionTransition **partitionTransitions, int partitionSize, Partition **partitions)
{
  for(int j = 0; j < state->usedTransitions; j++)
  {
    DFATransition *trans = state->transitions[j];

    PartitionTransition *transition = tryFindPartitionTransition(trans->characters[0], state->usedTransitions, partitionTransitions);

    if(transition == NULL || transition->partition != findPartitionToState(trans->toState, partitionSize, partitions)) return false;
  }

  return true;
}

PartitionSplit *split(Partition *partition, int partitionSize, Partition **partitions)
{
  PartitionSplit *split = createPartitionSplit(partition->categoryId);

  DFAState *firstState = partition->states[0];
  addStateToPartition(split->first, firstState);

  PartitionTransition **partitionTransitions = partitionTransitionsForState(firstState, partitionSize, partitions);

  for(int i = 1; i < partition->usedStates; i++)
  {
    DFAState *state = partition->states[i];

    if(state->usedTransitions != firstState->usedTransitions)
    {
      addStateToPartition(split->second, state);
      split->split = true;
      continue;
    }

    if(transitionsEquivalent(state, partitionTransitions, partitionSize, partitions))
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
