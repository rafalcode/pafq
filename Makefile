CC=gcc
CFLAGS=-g -Wall
SPECLIBS=-lz
EXES=tkscan tkscan_dbg coumer fqspl strarr z5 z7 pafq

tkscan: tkscan.c
	${CC} ${CFLAGS} -o $@ $^

fqspl: fqspl.c
	${CC} ${CFLAGS} -o $@ $^

tkscan_dbg: tkscan.c
	${CC} ${CFLAGS} -DDBG -o $@ $^

# coumer, a simple utility to print out all the possible k-permutation (with repetition) to give an idea of the space
coumer: coumer.c
	${CC} ${CFLAGS} -o $@ $^ -lm

pafq: pafq.c
	${CC} ${CFLAGS} -o $@ $^

strarr: strarr.c
	${CC} ${CFLAGS} -o $@ $^

z5: z5.c
	${CC} ${CFLAGS} -o $@ $^ ${SPECLIBS}

z7: z7.c
	${CC} ${CFLAGS} -o $@ $^ ${SPECLIBS}

.PHONY: clean

clean:
	rm -f ${EXES}
