#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "InfixConverter.h"
#include "Nfa.h"
#include "Debug/Printer.h"

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

NFA *deepCopyNFA(NFA *nfa)
{
  NFAState *newStart;
  NFAState *newEnd;
  deepCopyNFAStates(nfa->start, nfa->end->id + 1, &newStart, &newEnd);
  return createNFA(newStart, newEnd);
}

NFA *buildNFA(int startId, char *regex)
{
  char *postfix = infixToPostfix(regex);

  struct NFA **nfaStack = malloc(sizeof(postfix)/sizeof(postfix[0]) * sizeof(NFA));
  int nfaPosition = 0;
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
      NFA *nfa1 = nfaStack[--nfaPosition];
      NFA *nfa2 = nfaStack[--nfaPosition];

      nfa1->final->accepting = false;
      nfa2->final->accepting = false;

      NFAState *start = createState(stateId++, nfa1->start, '\0', nfa2->start, '\0');
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = end;

      nfa2->final->outChar1 = '\0';
      nfa2->final->out1 = end;

      NFA *alternationNFA = createNFA(start, end);

      nfaStack[nfaPosition++] = alternationNFA;
    }
    else if(c == '*' && !escaped)
    {
      NFA *nfa = nfaStack[--nfaPosition];

      nfa->final->accepting = false;

      NFAState *start = createState(stateId++, nfa->start, '\0', NULL, '\0');
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');
      start->out2 = end;

      nfa->final->outChar1 = '\0';
      nfa->final->out1 = end;
      nfa->final->outChar2 = '\0';
      nfa->final->out2 = nfa->start;

      nfaStack[nfaPosition++] = createNFA(start, end);
    }
    else if(c == '.' && !escaped)
    {
      NFA *nfa2 = nfaStack[--nfaPosition];
      NFA *nfa1 = nfaStack[--nfaPosition];

      nfa1->final->accepting = false;
      nfa2->final->accepting = false;

      nfa1->final->outChar1 = '\0';
      nfa1->final->out1 = nfa2->start;

      nfaStack[nfaPosition++] = createNFA(nfa1->start, nfa2->final);
    }
    else
    {
      NFAState *start = createState(stateId++, NULL, c, NULL, '\0');
      NFAState *end = createState(stateId++, NULL, '\0', NULL, '\0');
      start->out1 = end;

      nfaStack[nfaPosition++] = createNFA(start, end);
    }
  }

  return nfaStack[0];
}
