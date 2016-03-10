#include "Hopcroft.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../Util/Collections/ArrayList.h"

typedef struct Partition {
  int categoryId;
  ArrayList *states;
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
  partition->states = arrayListCreate(10, sizeof(DFAState *));

  return partition;
}

Partition *tryGetExistingPartition(int categoryId, ArrayList *partitionTable)
{
  for(int i = 0; i < arrayListCount(partitionTable); i++)
  {
    Partition *partition = arrayListGet(partitionTable, i);
    if(partition->categoryId == categoryId) return partition;
  }

  return NULL;
}

Partition *findPartitionToState(DFAState *state, ArrayList *partitionTable)
{
  for(int i = 0; i < arrayListCount(partitionTable); i++)
  {
    Partition *partition = arrayListGet(partitionTable, i);

    for(int j = 0; j < arrayListCount(partition->states); j++)
    {
      if(arrayListGet(partition->states, j) == state) return partition;
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

PartitionTransition **partitionTransitionsForState(DFAState *state, ArrayList *partitionTable)
{
  PartitionTransition **partitionTransitions = calloc(arrayListCount(state->transitions), sizeof(PartitionTransition *));

  for(int i = 0; i < arrayListCount(state->transitions); i++)
  {
    DFATransition *trans = arrayListGet(state->transitions, i);

    partitionTransitions[i] = malloc(sizeof(PartitionTransition));
    partitionTransitions[i]->character = trans->characters[0];
    partitionTransitions[i]->partition = findPartitionToState(trans->toState, partitionTable);
  }

  return partitionTransitions;
}

bool transitionsEquivalent(DFAState *state, PartitionTransition **partitionTransitions, ArrayList *partitionTable)
{
  for(int j = 0; j < arrayListCount(state->transitions); j++)
  {
    DFATransition *trans = arrayListGet(state->transitions, j);

    PartitionTransition *transition = tryFindPartitionTransition(trans->characters[0], arrayListCount(state->transitions), partitionTransitions);

    if(transition == NULL || transition->partition != findPartitionToState(trans->toState, partitionTable)) return false;
  }

  return true;
}

PartitionSplit *split(Partition *partition, ArrayList *partitionTable)
{
  PartitionSplit *split = createPartitionSplit(partition->categoryId);

  DFAState *firstState = arrayListGet(partition->states, 0);
  arrayListPush(split->first->states, firstState);

  PartitionTransition **partitionTransitions = partitionTransitionsForState(firstState, partitionTable);

  for(int i = 1; i < arrayListCount(partition->states); i++)
  {
    DFAState *state = arrayListGet(partition->states, i);

    if(arrayListCount(state->transitions) != arrayListCount(firstState->transitions))
    {
      arrayListPush(split->second->states, state);
      split->split = true;
      continue;
    }

    if(transitionsEquivalent(state, partitionTransitions, partitionTable))
    {
      arrayListPush(split->first->states, state);
    }
    else
    {
      arrayListPush(split->second->states, state);
      split->split = true;
    }
  }

  return split;
}

int findPartitionIdToPartition(Partition *partition, ArrayList *partitionTable)
{
  for(int i = 0; i < arrayListCount(partitionTable); i++)
  {
    if(arrayListGet(partitionTable, i) == partition) return i;
  }

  return -1;
}

DFATransition *tryFindTransition(DFAState *state, DFAState *toState)
{
  for(int i = 0; i < arrayListCount(state->transitions); i++)
  {
    DFATransition *trans = arrayListGet(state->transitions, i);
    if(trans->toState == toState) return trans;
  }

  return NULL;
}

ArrayList *createPartitionTable()
{
  return arrayListCreate(10, sizeof(Partition *));
}

ArrayList *initializePartitionTable(DFA *dfa)
{
  ArrayList *partitionTable = createPartitionTable();

  for(int i = 0; i < arrayListCount(dfa->states); i++)
  {
    DFAState *state = arrayListGet(dfa->states, i);
    Partition *partition = tryGetExistingPartition(state->categoryId, partitionTable);

    if(partition == NULL)
    {
      partition = createPartition(state->categoryId);
      arrayListPush(partitionTable, partition);
    }

    arrayListPush(partition->states, state);
  }

  return partitionTable;
}

ArrayList *splitTable(ArrayList *partitionTable)
{
  bool hasNewPartitions;

  do
  {
    hasNewPartitions = false;

    ArrayList *newPartitionTable = createPartitionTable();

    for(int i = 0; i < arrayListCount(partitionTable); i++)
    {
      PartitionSplit *partitionSplit = split(arrayListGet(partitionTable, i), partitionTable);

      arrayListPush(newPartitionTable, partitionSplit->first);

      if(partitionSplit->split)
      {
        arrayListPush(newPartitionTable, partitionSplit->second);
        hasNewPartitions = true;
      }
    }

    partitionTable = newPartitionTable;
  } while(hasNewPartitions);

  return partitionTable;
}

DFA *initializeDFA(ArrayList *partitionTable)
{
  DFA *dfa = malloc(sizeof(DFA));
  dfa->states = arrayListCreate(arrayListCount(partitionTable), sizeof(DFAState *));

  for(int i = 0; i < arrayListCount(partitionTable); i++)
  {
    Partition *partition = arrayListGet(partitionTable, i);
    DFAState *state = arrayListGet(partition->states, 0);
    arrayListPush(dfa->states, createDFAState(i, state->categoryId));
  }

  return dfa;
}

int findPartitionIdToState(DFAState *state, ArrayList *partitionTable)
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

void createTransitions(DFA *minimizedDfa, ArrayList *partitionTable)
{
  for(int i = 0; i < arrayListCount(partitionTable); i++)
  {
    DFAState *partitionState = arrayListGet(minimizedDfa->states, i);

    //old state, which holds all transitions which have to be generated for the partition state
    Partition *partition = arrayListGet(partitionTable, i);
    DFAState *state = arrayListGet(partition->states, 0);

    for(int j = 0; j < arrayListCount(state->transitions); j++)
    {
      DFATransition *trans = arrayListGet(state->transitions, j);

      int partitionId = findPartitionIdToState(trans->toState, partitionTable);

      DFAState *toState = arrayListGet(minimizedDfa->states, partitionId);
      DFATransition *partitionTransition = tryFindTransition(partitionState, toState);

      if(partitionTransition == NULL)
      {
        partitionTransition = createPartitionTransition(toState);
        arrayListPush(partitionState->transitions, partitionTransition);
      }

      appendCharacter(partitionTransition, trans->characters);
    }
  }
}

void setStartState(DFA *dfa, DFAState *startState, ArrayList *partitionTable)
{
  dfa->start = arrayListGet(dfa->states, findPartitionIdToState(startState, partitionTable));
}

DFA *createDFAFromPartitionTable(ArrayList *partitionTable, DFAState *startState)
{
  DFA *dfa = initializeDFA(partitionTable);

  createTransitions(dfa, partitionTable);
  setStartState(dfa, startState, partitionTable);

  return dfa;
}

DFA *hopcroft(DFA *dfa)
{
  ArrayList *partitionTable = splitTable(initializePartitionTable(dfa));
  return createDFAFromPartitionTable(partitionTable, arrayListGet(dfa->states, 0));
}
