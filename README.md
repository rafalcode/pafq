pafq
====

Parse a short read FASTQ file

Create a data structure to hold base calls and the quality values.

tkscan.c was a generic token finder. You give it a token and it stop when it reaches a match.
However, it grew into the parser itself. So I renamed it to pafq

fqspl.c  is for splitting a fq file for a parallelising context, so it will - in the future-
accept an argument for "Number of Processes" and split the file into that number of 
fq files.

b106bit.fq is a test FASTQ file
