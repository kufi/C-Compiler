#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "InfixConverter.h"
#include "Nfa.h"
#include "Debug/Printer.h"
#include "Util/Collections/HashMap.h"
#include "Util/Collections/Stack.h"

NFAState *createState(int id, NFAState *out1, char outChar1, NFAState *out2, char outChar2)
{
  NFAState *state = malloc(sizeof(NFAState));
  state->id = id;
  state->outChar1 = outChar1;
  state->out1 = out1;

  state->outChar2 = outChar2;
  state->out2 = out2;

  state->accepting = false;

  return state;
}

NFA *createNFA(NFAState *start, NFAState *final)
{
  NFA *nfa = malloc(sizeof(NFA));
  nfa->start = start;
  nfa->final = final;
  nfa->final->accepting = true;

  return nfa;
}

NFAState *deepCopyNFAState(NFAState *state, int *id, int startStateId, int endStateId, NFAState **newStart, NFAState **newEnd, HashMap *existingStates)
{
  if(state == NULL) return NULL;

  NFAState *existingState = hashMapGet(existingStates, &(state->id));
  if(existingState != NULL) return existingState;

  NFAState *newState = createState(
    (*id)++,
    NULL,
    state->outChar1,
    NULL,
    state->outChar2
  );

  hashMapSet(existingStates, &(state->id), newState);

  newState->out1 = deepCopyNFAState(state->out1, id, startStateId, endStateId, newStart, newEnd, existingStates);
  newState->out2 = deepCopyNFAState(state->out2, id, startStateId, endStateId, newStart, newEnd, existingStates);

  if(state->id == startStateId) *newStart = newState;
  if(state->id == endStateId) *newEnd = newState;

  return newState;
}

static uint32_t intHash(void *a)
{
  int *key = a;
  return *key;
}

static bool intCompare(void *a, void *b)
{
  int *first = a;
  int *second = b;
  return *first == *second;
}

NFA *deepCopyNFA(NFA *nfa, int *id)
{
  NFAState *newStart;
  NFAState *newEnd;
  int startStateId = nfa->start->id;
  int endStateId = nfa->final->id;
  HashMap *copiedStates = hashMapCreateFull(16, 0.75f, intCompare, intHash);

  deepCopyNFAState(nfa->start, id, startStateId, endStateId, &newStart, &newEnd, copiedStates);
  newEnd->id = (*id)++;

  return createNFA(newStart, newEnd);
}

NFA *createKleene(NFA *nfa, int stateId1, int stateId2)
{
  nfa->final->accepting = false;

  NFAState *start = createState(stateId1, nfa->start, '\0', NULL, '\0');
  NFAState *end = createState(stateId2, NULL, '\0', NULL, '\0');
  start->out2 = end;

  nfa->final->outChar1 = '\0';
  nfa->final->out1 = end;
  nfa->final->outChar2 = '\0';
  nfa->final->out2 = nfa->start;

  return createNFA(start, end);
}

NFA *concat(NFA *nfa1, NFA *nfa2)
{
  nfa1->final->accepting = false;
  nfa2->final->accepting = false;

  nfa1->final->outChar1 = '\0';
  nfa1->final->out1 = nfa2->start;

  return createNFA(nfa1->start, nfa2->final);
}

NFA *buildNFA(int startId, char *regex)
{
  char *postfix = infixToPostfix(regex);

  Stack *nfaStack = stackCreate();
  int stateId = startId;

  for(int i = 0; postfix[i] != '\0'; i++)
  {
    char c = postfix[i];
    bool escaped = false;

    if(c == '\\')
    {
      escaped = true;
      c = postfix[++i];
    }

    if(c == '|' && !escaped)
    {
      NFA *nfa1 = stackPop(nfaStack);
      NFA *nfa2 = stackPop(nfaStack);

      nfa1->final->accepting = false;
      nfa2->final->accepting = false;

      NFAState *start = createState(stateId++, nfa1->start, '\0', nfa2->start, '\0');
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = end;

      nfa2->final->outChar1 = '\0';
      nfa2->final->out1 = end;

      NFA *alternationNFA = createNFA(start, end);

      stackPush(nfaStack, alternationNFA);
    }
    else if(c == '+' && !escaped)
    {
      NFA *nfa = stackPop(nfaStack);
      NFA *nfaCopy = deepCopyNFA(nfa, &stateId);
      int firstId = stateId++;
      int secondId = stateId++;
      NFA *kleene = createKleene(nfaCopy, firstId, secondId);
      stackPush(nfaStack, concat(nfa, kleene));
    }
    else if(c == '*' && !escaped)
    {
      NFA *nfa = stackPop(nfaStack);
      int firstId = stateId++;
      int secondId = stateId++;
      stackPush(nfaStack, createKleene(nfa, firstId, secondId));
    }
    else if(c == '.' && !escaped)
    {
      NFA *nfa2 = stackPop(nfaStack);
      NFA *nfa1 = stackPop(nfaStack);
      stackPush(nfaStack, concat(nfa1, nfa2));
    }
    else
    {
      NFAState *start = createState(stateId++, NULL, c, NULL, '\0');
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');
      start->out1 = end;

      stackPush(nfaStack, createNFA(start, end));
    }
  }

  return stackPop(nfaStack);
}
