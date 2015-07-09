CC=gcc
CFLAGS=-g -Wall -std=c11 -O1
LDFLAGS=-g

.PHONY: all clean
all: ciapos
clean:
	rm -f *.o ciapos

ciapos: main.o symbol.o sexp.o chargen.o utf8.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(shell $(CC) -MM main.c)
$(shell $(CC) -MM symbol.c)
$(shell $(CC) -MM sexp.c)
$(shell $(CC) -MM chargen.c)
$(shell $(CC) -MM utf8.c)
