#ifndef DFA_HEADER
#define DFA_HEADER

#include "Nfa.h"

typedef struct DFATransition {
  int characterSize;
  char *characters;
} DFATransition;

typedef struct DFA {
  int numberOfStates;
  DFATransition ***transitions;
} DFA;

DFA *subsetConstruction(NFA *nfa, char *characterSet, int characterSetSize);

#endif
