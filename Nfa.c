#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "InfixConverter.h"
#include "Nfa.h"

NFAState *createState(int id, NFAState *out1, char outChar1, NFAState *out2, char outChar2)
{
  NFAState *state = malloc(sizeof(NFAState));
  state->id = id;
  state->outChar1 = outChar1;
  state->out1 = out1;

  state->outChar2 = outChar2;
  state->out2 = out2;

  return state;
}

NFA *createNFA(NFAState *start, NFAState *final)
{
  NFA *nfa = malloc(sizeof(NFA));
  nfa->start = start;
  nfa->final = final;

  return nfa;
}

NFA *buildNFA(char *regex)
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

      NFAState *start = createState(stateId++, nfa1->start, '\0', nfa2->start, '\0');
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = end;

      nfa2->final->outChar1 = '\0';
      nfa2->final->out1 = end;

      NFA *alternationNFA = createNFA(start, end);

      nfaStack[nfaPosition++] = alternationNFA;
    }
    else if(c == '*')
    {
      NFA *nfa = nfaStack[--nfaPosition];

      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');
      NFAState *start = createState(stateId++, nfa->start, '\0', end, '\0');

      nfa->final->outChar1 = '\0';
      nfa->final->out1 = end;
      nfa->final->outChar2 = '\0';
      nfa->final->out2 = nfa->start;

      nfaStack[nfaPosition++] = createNFA(start, end);
    }
    else if(c == '.')
    {
      NFA *nfa2 = nfaStack[--nfaPosition];
      NFA *nfa1 = nfaStack[--nfaPosition];

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = nfa2->start;

      nfaStack[nfaPosition++] = createNFA(nfa1->start, nfa2->final);
    }
    else
    {
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');
      NFAState *start = createState(stateId++, end, c, NULL, '\0');

      nfaStack[nfaPosition++] = createNFA(start, end);
    }
  }

  return nfaStack[0];
}
