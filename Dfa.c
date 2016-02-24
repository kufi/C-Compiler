#include "Dfa.h"
#include <stdlib.h>

DFAState *createDFAState(int id, int categoryId)
{
  DFAState *state = malloc(sizeof(DFAState));
  state->id = id;
  state->categoryId = categoryId;
  state->usedTransitions = 0;
  state->transitionSize = 5;
  state->transitions = malloc(sizeof(DFATransition *) * state->transitionSize);

  return state;
}
