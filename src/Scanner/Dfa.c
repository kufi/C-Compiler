#include "Dfa.h"
#include <stdlib.h>

DFAState *createDFAState(int id, int categoryId)
{
  DFAState *state = malloc(sizeof(DFAState));
  state->id = id;
  state->categoryId = categoryId;
  state->transitions = arrayListCreate(10, sizeof(DFATransition *));
  return state;
}
