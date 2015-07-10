CC=gcc
CFLAGS=-g -Wall -std=c11 -O1
LDFLAGS=

.PHONY: all clean
all: ciapos
clean:
	rm -f *.o ciapos

ciapos: main.o symbol.o sexp.o chargen.o utf8.o unicode.o lexutil.o token.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

gendeps = $(shell $(CC) -MM $(1) | tr -d '\n\\')
$(call gendeps,main.c)
$(call gendeps,symbol.c)
$(call gendeps,sexp.c)
$(call gendeps,chargen.c)
$(call gendeps,utf8.c)
$(call gendeps,unicode.c)
$(call gendeps,lexutil.c)
$(call gendeps,token.c)
