CC=gcc
DCFLAGS=-g -Wall
CFLAGS=-O3
SPECLIBS=-lz
EXES=tkscan tkscan_dbg coumer fqspl strarr zread z7 pafq zread pafqz sradeint

tkscan: tkscan.c
	${CC} ${DCFLAGS} -o $@ $^

fqspl: fqspl.c
	${CC} ${DCFLAGS} -o $@ $^

tkscan_dbg: tkscan.c
	${CC} ${DCFLAGS} -DDBG -o $@ $^

# coumer, a simple utility to print out all the possible k-permutation (with repetition) to give an idea of the space
coumer: coumer.c
	${CC} ${DCFLAGS} -o $@ $^ -lm

pafq: pafq.c
	${CC} ${DCFLAGS} -o $@ $^

strarr: strarr.c
	${CC} ${DCFLAGS} -o $@ $^

# Essential part fo fastq file is being able to read gzipped version of the them
# The zlib library is used for this.


# z7 taken from zpipe.c the canonical zlib usage example.
z7: z7.c
	${CC} ${DCFLAGS} -o $@ $^ ${SPECLIBS}

# only interested in inflating compressed files, not the other around. Given a nice name too:
zread: zread.c
	${CC} ${DCFLAGS} -o $@ $^ ${SPECLIBS}

# incorporate zread to be abel to parse a fastq.gz
pafqz: pafqz.c
	${CC} ${DCFLAGS} -o $@ $^ ${SPECLIBS}

# Special SRA format where the + line also includes the READNAME _AND_ forward and reverse are alternately interleaved into one file
# This program will separate the forward and reverse reads into separate file and will remove the repeated READNAME on the plus-sign line
sradeint: sradeint.c
	${CC} ${CFLAGS} -o $@ $^ ${SPECLIBS}

.PHONY: clean

clean:
	rm -f ${EXES}
