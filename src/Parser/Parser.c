#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"
#include "Parser.h"
#include "../Scanner/Scanner.h"
#include "../Util/Collections/ArrayList.h"
#include "../Util/Collections/Queue.h"
#include "../Util/Collections/HashSet.h"
#include "../Util/Collections/HashMap.h"
#include "../Util/Collections/Stack.h"
#include "../Util/Collections/LinkedList.h"
#include "../Scanner/StringBuilder.h"
#include "CanonicalCollection.h"
#include "FirstSet.h"

typedef struct Transition {
  ArrayList *from;
  ArrayList *to;
  char *symbol;
} Transition;

void printLR1Item(LR1Item *item)
{
  printf("[%s -> ", item->production->name);

  for(int j = 0; j < arrayListCount(item->rule->symbols); j++)
  {
    if(j == item->dotPosition) printf("\u2022");
    printf("%s ", (char *)arrayListGet(item->rule->symbols, j));
  }

  if(item->dotPosition == arrayListCount(item->rule->symbols)) printf("\u2022");

  printf(", %s]\n", item->lookahead);
}

void printLR1ItemList(LR1ItemList *list)
{
  printf("----CC%i----\n", list->number);
  for(int i = 0; i < arrayListCount(list->items); i++)
  {
    printLR1Item(arrayListGet(list->items, i));
  }
}

ParserTable *createParser(Grammar *grammar, ScannerConfig *config)
{
  Production *goalProduction = arrayListGet(grammar->productions, 0);
  HashMap *sets = createFirstSets(grammar, config);
  CC *cc = buildCanonicalCollection(sets, grammar, goalProduction);
  return createParserTable(cc, grammar, goalProduction);
}

ParseTreeItem *createNonterminalParseTreeItem(Production *production, Rule *rule)
{
  ParseTreeItem *item = malloc(sizeof(ParseTreeItem));
  item->type = NONTERMINAL;
  item->production = production;
  item->rule = rule;
  item->subItems = linkedListCreate();
  return item;
}

ParseTreeItem *createTerminalParseTreeItem(Word *word)
{
  ParseTreeItem *item = malloc(sizeof(ParseTreeItem));
  item->type = TERMINAL;
  item->word = word;
  return item;
}

enum ParseStackEntryType { STATE, ITEM };

typedef struct ParseStackEntry {
  enum ParseStackEntryType type;
  union {
    int state;
    ParseTreeItem *item;
  };
} ParseStackEntry;

ParseStackEntry *createStateEntry(int state)
{
  ParseStackEntry *entry = malloc(sizeof(ParseStackEntry));
  entry->type = STATE;
  entry->state = state;
  return entry;
}

ParseStackEntry *createItemEntry(ParseTreeItem *item)
{
  ParseStackEntry *entry = malloc(sizeof(ParseStackEntry));
  entry->type = ITEM;
  entry->item = item;
  return entry;
}

Word createEOF()
{
  Category *category = malloc(sizeof(Category));
  category->name = END;
  Word word;
  word.lexeme = END;
  word.category = category;
  return word;
}

ParseTree *runParser(ParserTable *table, ScannerConfig *scannerConfig, char *input)
{
  addCategory(scannerConfig, "insignificantWhitespace", "( |\n)( |\n)*");

  Scanner *scanner = createScanner(scannerConfig, input);

  Stack *parseStack = stackCreate();
  stackPush(parseStack, createStateEntry(-1));
  stackPush(parseStack, createStateEntry(0));

  ParseTree *tree = calloc(1, sizeof(ParseTree));
  tree->success = false;

  Word word = hasMoreWords(scanner) ? nextWord(scanner) : createEOF();

  while(true)
  {
    //ignore whitespace
    if(strcmp(word.category->name, "insignificantWhitespace") == 0)
    {
      word = hasMoreWords(scanner) ? nextWord(scanner) : createEOF();
      continue;
    }

    ParseStackEntry *stackTop = stackPeek(parseStack);
    ParseState *state = arrayListGet(table->states, stackTop->state);
    Action *action  = hashMapGet(state->actions, word.category->name);

    if(action == NULL) return tree;

    if(action->type == REDUCE)
    {
      Rule *rule = action->toRule;
      int popCount = arrayListCount(rule->symbols) * 2;

      //on empty do not pop anything
      if(arrayListCount(rule->symbols) == 1 && strcmp(arrayListGet(rule->symbols, 0), EMPTY) == 0)
      {
          popCount = 0;
      }

      ParseTreeItem *item = createNonterminalParseTreeItem(action->toProduction, action->toRule);
      ParseStackEntry *newEntry = createItemEntry(item);

      for(int i = 0; i < popCount; i++)
      {
        ParseStackEntry *entry = stackPop(parseStack);
        if(entry->type == ITEM) linkedListUnshift(item->subItems, entry->item);
      }

      stackTop = stackPeek(parseStack);
      state = arrayListGet(table->states, stackTop->state);
      stackPush(parseStack, newEntry);
      GoTo *goTo = hashMapGet(state->gotos, action->toProduction->name);
      stackPush(parseStack, createStateEntry(goTo->number));
    }
    else if(action->type == SHIFT)
    {
      Word *heapWord = calloc(1, sizeof(Word));
      memcpy(heapWord, &word, sizeof(Word));
      ParseTreeItem *item = createTerminalParseTreeItem(heapWord);
      stackPush(parseStack, createItemEntry(item));
      stackPush(parseStack, createStateEntry(action->toState));

      word = hasMoreWords(scanner) ? nextWord(scanner) : createEOF();
    }
    else if(action->type == ACCEPT)
    {
      stackPop(parseStack);
      ParseStackEntry *root = stackPop(parseStack);
      tree->success = true;
      tree->root = root->item;
      return tree;
    }
  }

  return tree;
}
