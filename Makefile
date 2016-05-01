CC=gcc
CFLAGS=-g -Wall
SPECLIBS=-lz
EXES=tkscan tkscan_dbg coumer fqspl strarr zread z7 pafq zread

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

# Essential part fo fastq file is being able to read gzipped version of the them
# The zlib library is used for this.


# z7 taken from zpipe.c the canonical zlib usage example.
z7: z7.c
	${CC} ${CFLAGS} -o $@ $^ ${SPECLIBS}

# only interested in inflating compressed files, not the other around. Given a nice name too:
zread: zread.c
	${CC} ${CFLAGS} -o $@ $^ ${SPECLIBS}

.PHONY: clean

clean:
	rm -f ${EXES}
