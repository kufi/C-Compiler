#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Grammar.h"
#include "Parser.h"
#include "Scanner/Scanner.h"
#include "Util/Collections/ArrayList.h"
#include "Util/Collections/Queue.h"
#include "Util/Collections/HashSet.h"
#include "Util/Collections/HashMap.h"
#include "Util/Collections/Stack.h"
#include "Util/Collections/LinkedList.h"
#include "Scanner/StringBuilder.h"
#include "CanonicalCollection.h"
#include "FirstSet.h"

typedef struct Transition {
  ArrayList *from;
  ArrayList *to;
  char *symbol;
} Transition;

enum ParseStackEntryType { STATE, ITEM };

typedef struct ParseStackEntry {
  enum ParseStackEntryType type;
  union {
    int state;
    ParseTreeItem *item;
  };
} ParseStackEntry;

ParserTable *createParser(Grammar *grammar, char *goalProduction)
{
  HashMap *sets = createFirstSets(grammar);
  CC *cc = buildCanonicalCollection(sets, grammar, goalProduction);
  return createParserTable(cc, grammar, goalProduction);
}

ParseTreeItem *createNonterminalParseTreeItem(Rule *rule, int subItemCount)
{
  ParseTreeItem *item = malloc(sizeof(ParseTreeItem));
  item->type = NONTERMINAL;
  item->rule = rule;
  item->subItems = arrayListCreate(subItemCount, sizeof(ParseTreeItem));
  return item;
}

ParseTreeItem *createTerminalParseTreeItem(Word *word)
{
  ParseTreeItem *item = malloc(sizeof(ParseTreeItem));
  item->type = TERMINAL;
  item->word = word;
  item->astNode = createASTNode(NULL, word);
  return item;
}

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

ASTNode *createASTNode(Rule *rule, Word *word)
{
  ASTNode *node = calloc(1, sizeof(ASTNode));
  node->rule = rule;
  node->word = word;
  node->subNodes = arrayListCreate(10, sizeof(ASTNode *));
  return node;
}

void addSubNode(ASTNode *node, ASTNode *subNode)
{
  arrayListPush(node->subNodes, subNode);
}

ParseTree *runParser(ParserTable *table, char *input)
{
  ScannerConfig *scannerConfig = table->grammar->scannerConfig;
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
      int symbolCount = arrayListCount(rule->symbols);
      int popCount = symbolCount * 2;

      //on empty do not pop anything
      if(symbolCount == 1 && strcmp(arrayListGet(rule->symbols, 0), EMPTY) == 0)
      {
          popCount = 0;
      }

      ParseTreeItem *item = createNonterminalParseTreeItem(action->toRule, symbolCount);
      ParseStackEntry *newEntry = createItemEntry(item);

      int pos = symbolCount;
      for(int i = 0; i < popCount; i++)
      {
        ParseStackEntry *entry = stackPop(parseStack);
        if(entry->type == ITEM) arrayListSet(item->subItems, --pos, entry->item);
      }

      stackTop = stackPeek(parseStack);
      state = arrayListGet(table->states, stackTop->state);
      stackPush(parseStack, newEntry);

      ParseTreeItem *firstItem = (ParseTreeItem *)arrayListGet(item->subItems, 0);

      item->astNode = rule->reduceAction == NULL ? (firstItem != NULL ? firstItem->astNode : NULL) : rule->reduceAction(item);

      GoTo *goTo = hashMapGet(state->gotos, action->toRule->productionName);
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
