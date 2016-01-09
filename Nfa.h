#ifndef NFA_HEADER
#define NFA_HEADER

typedef struct NFAState {
  int id;
  char outChar1;
  char outChar2;
  struct NFAState *out1;
  struct NFAState *out2;
} NFAState;

typedef struct NFA {
  NFAState *start;
  NFAState *final;
} NFA;

NFA *buildNFA(char *regex);

#endif
