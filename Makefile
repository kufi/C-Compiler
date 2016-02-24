CC = gcc
CFLAGS = -Wall -std=gnu99
HEADERS = InfixConverter.h Nfa.h Dfa.h Scanner.h
OBJECTS = main.o Nfa.o Dfa.o InfixConverter.o Scanner.o

default: scanner

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS) -g

scanner: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -g

clean:
	-rm -f $(OBJECTS)
	-rm -f scanner
