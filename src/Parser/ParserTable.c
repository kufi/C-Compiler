#include "ParserTable.h"

void addActionToState(ParseState *state, char *symbol, Action *action)
{
  Action *existing = NULL;
  if((existing = hashMapGet(state->actions, symbol)) != NULL)
  {
    //Favor shift over reduce and accept over both
    if(existing->type < action->type) hashMapSet(state->actions, symbol, action);
  }
  else
  {
    hashMapSet(state->actions, symbol, action);
  }
}

GoTo *createGoto(int number, Production *production)
{
  GoTo *goTo = calloc(1, sizeof(GoTo));
  goTo->number = number;
  goTo->production = production;
  return goTo;
}

void buildGotoTable(ParseState *state, Grammar *grammar, HashMap *transitionsForList)
{
  for(int i = 0; i < arrayListCount(grammar->productions); i++)
  {
    Production *production = arrayListGet(grammar->productions, i);
    LR1ItemList *nonTerminalTransition = transitionsForList != NULL ? hashMapGet(transitionsForList, production->name) : NULL;

    if(nonTerminalTransition != NULL)
    {
      hashMapSet(state->gotos, production->name, createGoto(nonTerminalTransition->number, production));
    }
  }
}

Action *createReduceAcceptAction(LR1Item *item, Production *goalProduction)
{
  Action *action = calloc(1, sizeof(Action));
  action->symbol = item->lookahead;

  if(item->production == goalProduction && strcmp(item->lookahead, END) == 0)
  {
    action->type = ACCEPT;
  }
  else
  {
    action->type = REDUCE;
    action->toProduction = item->production;
    action->toRule = item->rule;
  }

  return action;
}

Action *createShiftAction(LR1ItemList *transition, char *symbol)
{
  Action *shiftAction = calloc(1, sizeof(Action));
  shiftAction->type = SHIFT;
  shiftAction->toState = transition->number;
  shiftAction->symbol = symbol;
  return shiftAction;
}

void buildActionTable(ParseState *state, Grammar *grammar, LR1ItemList *list, HashMap *transitionsForList, Production *goalProduction)
{
  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    LR1Item *item = arrayListGet(list->items, i);
    bool isAtEnd = item->dotPosition == arrayListCount(item->rule->symbols);

    //ignore empty entries, as they would never be used anyway
    if(strcmp(item->lookahead, EMPTY) == 0) continue;

    if(isAtEnd)
    {
      addActionToState(state, item->lookahead, createReduceAcceptAction(item, goalProduction));
      continue;
    }

    char *nextSymbol = getDotSymbol(item);
    Production *nextProduction = nextSymbol != NULL ? getProductionForSymbol(grammar, nextSymbol) : NULL;

    //nextSymbol is a terminal symbol
    if(nextProduction == NULL)
    {
      LR1ItemList *toTransition = transitionsForList != NULL ? hashMapGet(transitionsForList, nextSymbol) : NULL;
      if(toTransition != NULL) addActionToState(state, nextSymbol, createShiftAction(toTransition, nextSymbol));
    }
  }
}

ParseState *createParseState(int number)
{
  ParseState *state = calloc(1, sizeof(ParseState));
  state->number = number;
  state->actions = hashMapCreate();
  state->gotos = hashMapCreate();
  return state;
}

ParserTable *createParserTable(CC *cc, Grammar *grammar, Production *goalProduction)
{
  ParserTable *table = calloc(1, sizeof(ParserTable));
  table->states = arrayListCreate(cc->itemLists->count, sizeof(ParseState *));

  hashSetFor(cc->itemLists, cur)
  {
    LR1ItemList *list = hashSetForItem(cur);
    HashMap *transitionsForList = hashMapGet(cc->transitions, list);
    ParseState *state = createParseState(list->number);

    buildActionTable(state, grammar, list, transitionsForList, goalProduction);
    buildGotoTable(state, grammar, transitionsForList);

    arrayListSet(table->states, state->number, state);
  }
  hashSetForEnd

  return table;
}
