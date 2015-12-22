#ifndef NFA_HEADER
#define NFA_HEADER

typedef struct State {
  int id;
  char outChar1;
  char outChar2;
  struct State *out1;
  struct State *out2;
} State;

typedef struct NFA {
  State *start;
  State *final;
} NFA;

NFA *buildNFA(char *regex);

#endif
