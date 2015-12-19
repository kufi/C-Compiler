#include <stdbool.h>
#include <stdlib.h>

typedef struct PostfixState {
  char operatorStack[100];
  int operatorPosition;
  int outputLast;
  char *output;
} PostfixState;

int getOperatorPrecedence(char op)
{
  switch(op)
  {
    case '*': return 3; break;
    case '|': return 1; break;
    case '.': return 2; break;
    case '(': return 0; break;
  }

  return -1;
}

void addToOutput(struct PostfixState *state, char c)
{
  state->output[state->outputLast++] = c;
}

char popOperator(struct PostfixState *state)
{
  return state->operatorStack[--state->operatorPosition];
}

void pushOperator(struct PostfixState *state, char c)
{
  state->operatorStack[state->operatorPosition++] = c;
}

char peekOperator(struct PostfixState *state)
{
  return state->operatorStack[state->operatorPosition - 1];
}

char *infixToPostfix(char *regex)
{
  struct PostfixState state;
  state.output = malloc(sizeof(regex)/sizeof(regex[0]) * 2);
  state.outputLast = 0;
  state.operatorPosition = 0;

  for(int i = 0; regex[i] != '\0'; i++)
  {
    char c = regex[i];

    if(c == '|' || c == '*')
    {
      while(state.operatorPosition > 0 && getOperatorPrecedence(peekOperator(&state)) > getOperatorPrecedence(c))
      {
        addToOutput(&state, popOperator(&state));
      }

      pushOperator(&state, c);
    }
    else if(c == '(')
    {
      pushOperator(&state, c);
    }
    else if(c == ')')
    {
      while(state.operatorStack[state.operatorPosition - 1] != '(')
      {
        addToOutput(&state, popOperator(&state));
      }

      popOperator(&state);
    }
    else
    {
      addToOutput(&state, c);
    }

    char nextC = regex[i + 1];
    if(c != '|' && c != ')' && c != '(' && nextC != '|' && nextC != ')' && nextC != '*' && nextC != '\0')
    {
      if(c == '*') addToOutput(&state, popOperator(&state));

      pushOperator(&state, '.');
    }
    else if(c == ')' && nextC != ')' && nextC != '*' && nextC != '|' && nextC != '\0')
    {
      pushOperator(&state, '.');
    }
  }

  while(state.operatorPosition > 0) addToOutput(&state, popOperator(&state));

  addToOutput(&state, '\0');
  return state.output;
}
