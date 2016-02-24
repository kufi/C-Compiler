#ifndef NFA_HEADER
#define NFA_HEADER

#include <stdbool.h>

typedef struct NFAState {
  int id;
  char outChar1;
  char outChar2;
  struct NFAState *out1;
  struct NFAState *out2;
  bool accepting;
} NFAState;

typedef struct NFA {
  NFAState *start;
  NFAState *final;
} NFA;

NFAState *createState(int id, NFAState *out1, char outChar1, NFAState *out2, char outChar2);

NFA *buildNFA(int startId, char *regex);

#endif
