/* a utility to count the total number of mers for a range of k values */
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LIMIT 46 /* 46 will take it up to a billion billion billion (1x10^27) */

int main(int argc, char *argv[])
{
    int i;
    unsigned res;
    double dres;
    printf("Simple demonstration of kmer space depending on size of k\n");
    printf("K\tQuan\n");
    for(i=1;i<LIMIT;i+=2) {
        if(i<10) {
            res=pow(4, i);
            printf("%i\t%u\n", i, res);
        } else {
            dres=(double)pow(4, i);
            printf("%i\t%.3e\n", i, dres);
        }
    }

    return 0;
}
