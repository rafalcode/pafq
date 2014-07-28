CC=gcc
CFLAGS=-g -Wall
EXES=tkscan tkscan_dbg coumer

tkscan: tkscan.c
	${CC} ${CFLAGS} -o $@ $^

tkscan_dbg: tkscan.c
	${CC} ${CFLAGS} -DDBG -o $@ $^

# coumer, a simple utility to print out all the possible k-permutation (with repetition) to give an idea of the space
coumer: coumer.c
	${CC} ${CFLAGS} -o $@ $^ -lm

.PHONY: clean

clean:
	rm -f ${EXES}
