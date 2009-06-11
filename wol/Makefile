CC=gcc
CFLAGS=-g -pipe -I..
DEFS=-DORP_MAX_THREADS=16 -DORP_MAX_REQUEST=15 -DORP_STACK_SIZE=0x80000
LIBS=-pthread

all:
	make orpwol

orpwol: orpwol.c
	$(CC) $(CFLAGS) $(DEFS) orpwol.c -o orpwol $(LIBS)

clean:
	rm orpwol
