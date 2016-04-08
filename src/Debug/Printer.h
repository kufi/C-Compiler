#ifndef PRINTER_HEADER
#define PRINTER_HEADER

#include "../Scanner/Nfa.h"
#include "../Scanner/Dfa.h"
#include "../Scanner/Scanner.h"
#include "../Parser/Grammar.h"
#include "../Parser/CanonicalCollection.h"
#include "../Parser/ParserTable.h"
#include "../Parser/Parser.h"

void printNFA(NFA *nfa, int stateSize);

void printDFA(DFA *dfa);

void printGrammar(Grammar *grammar);

void printLR1ItemList(LR1ItemList *list);

void printParserTable(ScannerConfig *config, Grammar *grammar, ParserTable *table);

void printParseTree(ParseTree *tree);

void printAST(ParseTree *tree);

#endif
