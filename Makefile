CC       = gcc
CFLAGS   = -std=c99 -Wall -Wextra -pedantic -O2
INCLUDES = -Iinclude
AR       = ar

LIB      = libskewheap.a

.PHONY: all teste executar clean

all: $(LIB) exemplo

$(LIB): src/skew_heap.c include/skew_heap.h
	$(CC) $(CFLAGS) $(INCLUDES) -c src/skew_heap.c -o skew_heap.o
	$(AR) rcs $@ skew_heap.o

exemplo: examples/exemplo.c $(LIB)
	$(CC) $(CFLAGS) $(INCLUDES) examples/exemplo.c -L. -lskewheap -o $@

teste_skew: tests/teste.c $(LIB)
	$(CC) $(CFLAGS) $(INCLUDES) tests/teste.c -L. -lskewheap -o $@

teste: teste_skew
	./teste_skew

executar: exemplo
	./exemplo

clean:
	$(RM) skew_heap.o $(LIB) exemplo exemplo.exe teste_skew teste_skew.exe
