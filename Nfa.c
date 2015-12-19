#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "InfixConverter.h"
#include "Nfa.h"

NFA *createNFA(char *regex)
{
  char *postfix = infixToPostfix(regex);

  struct NFA **nfaStack = malloc(sizeof(postfix)/sizeof(postfix[0]) * sizeof(NFA));
  int nfaPosition = 0;
  int stateId = 0;

  for(int i = 0; postfix[i] != '\0'; i++)
  {
    char c = postfix[i];

    if(c == '|')
    {
      NFA *nfa1 = nfaStack[--nfaPosition];
      NFA *nfa2 = nfaStack[--nfaPosition];

      State *start = malloc(sizeof(State));
      start->id = stateId++;

      start->outChar1 = '\0';
      start->out1 = nfa1->start;
      start->outChar2 = '\0';
      start->out2 = nfa2->start;

      State *end = malloc(sizeof(State));
      end->id = stateId++;

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = end;
      nfa2->final->outChar1 = '\0';
      nfa2->final->out1 = end;

      NFA *alternationNFA = malloc(sizeof(NFA));
      alternationNFA->start = start;
      alternationNFA->final = end;

      nfaStack[nfaPosition++] = alternationNFA;
    }
    else if(c == '*')
    {
      NFA *nfa = nfaStack[--nfaPosition];

      State *start = malloc(sizeof(State));
      start->id = stateId++;

      State *end = malloc(sizeof(State));
      end->id = stateId++;

      start->outChar1 = '\0';
      start->out1 = nfa->start;
      start->outChar2 = '\0';
      start->out2 = end;

      nfa->final->outChar1 = '\0';
      nfa->final->out1 = end;
      nfa->final->outChar2 = '\0';
      nfa->final->out2 = nfa->start;

      NFA *closureNFA = malloc(sizeof(NFA));
      closureNFA->start = start;
      closureNFA->final = end;

      nfaStack[nfaPosition++] = closureNFA;
    }
    else if(c == '.')
    {
      NFA *nfa2 = nfaStack[--nfaPosition];
      NFA *nfa1 = nfaStack[--nfaPosition];

      NFA *concatNFA = malloc(sizeof(struct NFA));
      concatNFA->start = nfa1->start;
      concatNFA->final = nfa2->final;

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = nfa2->start;

      nfaStack[nfaPosition++] = concatNFA;
    }
    else
    {
      State *state = malloc(sizeof(State));
      state->id = stateId++;
      state->outChar1 = c;

      State *finalState = malloc(sizeof(State));
      finalState->id = stateId++;

      state->out1 = finalState;

      NFA *nfa = malloc(sizeof(struct NFA));
      nfa->start = state;
      nfa->final = finalState;

      nfaStack[nfaPosition++] = nfa;
    }
  }

  return nfaStack[0];
}
