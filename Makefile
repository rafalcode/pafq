CC=gcc
CFLAGS=-g -Wall# -pg # note the gprof option
EXES=tkscan

tkscan: tkscan.c
	${CC} ${CFLAGS} -o $@ $^

tkscan_dbg: tkscan.c
	${CC} ${CFLAGS} -DDBG -o $@ $^

.PHONY: clean

clean:
	rm -f ${EXES}
