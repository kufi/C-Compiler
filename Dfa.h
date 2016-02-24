#ifndef DFA_HEADER
#define DFA_HEADER

#include "Nfa.h"

typedef struct DFATransition {
  char *characters;
  struct DFAState *toState;
} DFATransition;

typedef struct DFAState {
  int id;
  int categoryId;
  int usedTransitions;
  int transitionSize;
  DFATransition **transitions;
} DFAState;

typedef struct DFA {
  int stateSize;
  DFAState **states;
  DFAState *start;
} DFA;

DFA *subsetConstruction(NFA *nfa, char *characterSet);

DFA *hopcroft(DFA *dfa);

#endif
