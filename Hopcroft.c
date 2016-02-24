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

typedef struct PartitionTable {
  int usedPartitions;
  int partitionSize;
  Partition **partitions;
} PartitionTable;

Partition *createPartition(const int categoryId)
{
  Partition *partition = malloc(sizeof(Partition));
  partition->categoryId = categoryId;
  partition->usedStates = 0;
  partition->stateSize = 10;
  partition->states = malloc(sizeof(DFAState *) * partition->stateSize);

  return partition;
}

Partition *tryGetExistingPartition(int categoryId, PartitionTable *partitionTable)
{
  for(int i = 0; i < partitionTable->usedPartitions; i++)
  {
    Partition *partition = partitionTable->partitions[i];
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

Partition *findPartitionToState(DFAState *state, PartitionTable *partitionTable)
{
  for(int i = 0; i < partitionTable->usedPartitions; i++)
  {
    Partition *partition = partitionTable->partitions[i];

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

PartitionTransition **partitionTransitionsForState(DFAState *state, PartitionTable *partitionTable)
{
  PartitionTransition **partitionTransitions = malloc(sizeof(PartitionTransition *) * state->usedTransitions);

  for(int i = 0; i < state->usedTransitions; i++)
  {
    DFATransition *trans = state->transitions[i];

    partitionTransitions[i] = malloc(sizeof(PartitionTransition));
    partitionTransitions[i]->character = trans->characters[0];
    partitionTransitions[i]->partition = findPartitionToState(trans->toState, partitionTable);
  }

  return partitionTransitions;
}

bool transitionsEquivalent(DFAState *state, PartitionTransition **partitionTransitions, PartitionTable *partitionTable)
{
  for(int j = 0; j < state->usedTransitions; j++)
  {
    DFATransition *trans = state->transitions[j];

    PartitionTransition *transition = tryFindPartitionTransition(trans->characters[0], state->usedTransitions, partitionTransitions);

    if(transition == NULL || transition->partition != findPartitionToState(trans->toState, partitionTable)) return false;
  }

  return true;
}

PartitionSplit *split(Partition *partition, PartitionTable *partitionTable)
{
  PartitionSplit *split = createPartitionSplit(partition->categoryId);

  DFAState *firstState = partition->states[0];
  addStateToPartition(split->first, firstState);

  PartitionTransition **partitionTransitions = partitionTransitionsForState(firstState, partitionTable);

  for(int i = 1; i < partition->usedStates; i++)
  {
    DFAState *state = partition->states[i];

    if(state->usedTransitions != firstState->usedTransitions)
    {
      addStateToPartition(split->second, state);
      split->split = true;
      continue;
    }

    if(transitionsEquivalent(state, partitionTransitions, partitionTable))
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

int findPartitionIdToPartition(Partition *partition, PartitionTable *partitionTable)
{
  for(int i = 0; i < partitionTable->usedPartitions; i++)
  {
    if(partitionTable->partitions[i] == partition) return i;
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

PartitionTable *createPartitionTable()
{
  PartitionTable *partitionTable = malloc(sizeof(PartitionTable));
  partitionTable->usedPartitions = 0;
  partitionTable->partitionSize = 10;
  partitionTable->partitions = malloc(sizeof(Partition *) * partitionTable->partitionSize);

  return partitionTable;
}

void addPartitionToTable(PartitionTable *table, Partition *partition)
{
  if(table->usedPartitions == table->partitionSize)
  {
    table->partitionSize = table->partitionSize * 2;
    table->partitions = realloc(table->partitions, sizeof(Partition *) * table->partitionSize);
  }

  table->partitions[table->usedPartitions++] = partition;
}

PartitionTable *initializePartitionTable(DFA *dfa)
{
  PartitionTable *partitionTable = createPartitionTable();

  for(int i = 0; i < dfa->stateSize; i++)
  {
    DFAState *state = dfa->states[i];

    Partition *partition = tryGetExistingPartition(state->categoryId, partitionTable);

    if(partition == NULL)
    {
      partition = createPartition(state->categoryId);
      addPartitionToTable(partitionTable, partition);
    }

    addStateToPartition(partition, state);
  }

  return partitionTable;
}

PartitionTable *splitTable(PartitionTable *partitionTable)
{
  bool hasNewPartitions;

  do
  {
    hasNewPartitions = false;

    PartitionTable *newPartitionTable = createPartitionTable();

    for(int i = 0; i < partitionTable->usedPartitions; i++)
    {
      PartitionSplit *partitionSplit = split(partitionTable->partitions[i], partitionTable);

      addPartitionToTable(newPartitionTable, partitionSplit->first);

      if(partitionSplit->split)
      {
        addPartitionToTable(newPartitionTable, partitionSplit->second);
        hasNewPartitions = true;
      }
    }

    partitionTable = newPartitionTable;
  } while(hasNewPartitions);

  return partitionTable;
}

DFA *initializeDFA(PartitionTable *partitionTable)
{
  DFA *dfa = malloc(sizeof(DFA));

  dfa->stateSize = partitionTable->usedPartitions;
  dfa->states = malloc(sizeof(DFAState *) * partitionTable->usedPartitions);

  for(int i = 0; i < partitionTable->usedPartitions; i++)
  {
    DFAState *state = partitionTable->partitions[i]->states[0];
    dfa->states[i] = createDFAState(i, state->categoryId);
    dfa->states[i]->usedTransitions = 0;
    dfa->states[i]->transitionSize = state->usedTransitions;
    dfa->states[i]->transitions = malloc(sizeof(DFATransition *) * state->usedTransitions);
  }

  return dfa;
}

int findPartitionIdToState(DFAState *state, PartitionTable *partitionTable)
{
  return findPartitionIdToPartition(findPartitionToState(state, partitionTable), partitionTable);
}

DFATransition *createPartitionTransition(DFAState *toState)
{
  DFATransition *partitionTransition = malloc(sizeof(DFATransition));
  partitionTransition->characters = "";
  partitionTransition->toState = toState;

  return partitionTransition;
}

void appendCharacter(DFATransition *partitionTransition, char *character)
{
  char *newCharacters = malloc(sizeof(char) * strlen(partitionTransition->characters) + 2);
  strcpy(newCharacters, partitionTransition->characters);
  partitionTransition->characters = strncat(newCharacters, character, 1);
}

void createTransitions(DFA *minimizedDfa, PartitionTable *partitionTable)
{
  for(int i = 0; i < partitionTable->usedPartitions; i++)
  {
    DFAState *partitionState = minimizedDfa->states[i];

    //old state, which holds all transitions which have to be generated for the partition state
    DFAState *state = partitionTable->partitions[i]->states[0];

    for(int j = 0; j < state->usedTransitions; j++)
    {
      DFATransition *trans = state->transitions[j];

      int partitionId = findPartitionIdToState(trans->toState, partitionTable);

      DFAState *toState = minimizedDfa->states[partitionId];
      DFATransition *partitionTransition = tryFindTransition(partitionState, toState);

      if(partitionTransition == NULL)
      {
        partitionTransition = createPartitionTransition(toState);
        partitionState->transitions[partitionState->usedTransitions++] = partitionTransition;
      }

      appendCharacter(partitionTransition, trans->characters);
    }
  }
}

void setStartState(DFA *dfa, DFAState *startState, PartitionTable *partitionTable)
{
  dfa->start = dfa->states[findPartitionIdToState(startState, partitionTable)];
}

DFA *createDFAFromPartitionTable(PartitionTable *partitionTable, DFAState *startState)
{
  DFA *dfa = initializeDFA(partitionTable);

  createTransitions(dfa, partitionTable);
  setStartState(dfa, startState, partitionTable);

  return dfa;
}

DFA *hopcroft(DFA *dfa)
{
  PartitionTable *partitionTable = splitTable(initializePartitionTable(dfa));
  return createDFAFromPartitionTable(partitionTable, dfa->states[0]);
}
