#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "StringBuilder.h"
#include "../Util/Collections/Stack.h"

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

void addToOutput(StringBuilder *output, char c)
{
  appendChar(output, c);
}

char *infixToPostfix(char *regex)
{
  Stack *state = stackCreate();
  StringBuilder output = createStringBuilder();

  for(int i = 0; regex[i] != '\0'; i++)
  {
    char c = regex[i];
    bool escapedChar = false;

    if(c == '\\')
    {
      c = regex[++i];
      escapedChar = true;
    }

    if(escapedChar)
    {
      addToOutput(&output, '\\');
      addToOutput(&output, c);
    }
    else if(c == '|' || c == '*')
    {
      while(state->count > 0 && getOperatorPrecedence(*(char *)stackPeek(state)) > getOperatorPrecedence(c))
      {
        addToOutput(&output, *(char *)stackPop(state));
      }

      stackPush(state, strdup(&c));
    }
    else if(c == '(')
    {
      stackPush(state, strdup(&c));
    }
    else if(c == ')')
    {
      while(*(char *)stackPeek(state) != '(')
      {
        addToOutput(&output, *(char *)stackPop(state));
      }

      stackPop(state);
    }
    else
    {
      addToOutput(&output, c);
    }

    char nextC = regex[i + 1];
    if(c != '|' && c != ')' && c != '(' && nextC != '|' && nextC != ')' && nextC != '*' && nextC != '\0')
    {
      if(c == '*') addToOutput(&output, *(char *)stackPop(state));

      stackPush(state, ".");
    }
    else if(c == ')' && nextC != ')' && nextC != '*' && nextC != '|' && nextC != '\0')
    {
      stackPush(state, ".");
    }
  }

  while(state->count > 0) addToOutput(&output, *(char *)stackPop(state));

  addToOutput(&output, '\0');
  return output.string;
}
