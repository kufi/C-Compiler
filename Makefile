CC = gcc
CFLAGS = -Wall -std=c99
HEADERS = InfixConverter.h Nfa.h
OBJECTS = main.o Nfa.o InfixConverter.o

default: scanner

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS) -g

scanner: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -g

clean:
	-rm -f $(OBJECTS)
	-rm -f scanner
