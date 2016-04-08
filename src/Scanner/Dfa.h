#ifndef DFA_HEADER
#define DFA_HEADER

#include "Nfa.h"
#include "Util/Collections/ArrayList.h"

typedef struct DFATransition {
  char *characters;
  struct DFAState *toState;
} DFATransition;

typedef struct DFAState {
  int id;
  int categoryId;
  ArrayList *transitions;
} DFAState;

typedef struct DFA {
  ArrayList *states;
  DFAState *start;
} DFA;

DFAState *createDFAState(int id, int categoryId);

#endif
