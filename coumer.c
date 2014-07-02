/* a utility to count the totla number of mers for a range of k values */
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LIMIT 41

int main(int argc, char *argv[])
{
    int i;
    unsigned res;
    double dres;
    printf("Simple demonstration of kmer space depending on size of k\n");
    printf("K\tQuan\n");
    for(i=1;i<LIMIT;++i) {
        if(i<12) {
            res=pow(4, i);
            printf("%i\t%u\n", i, res);
        } else {
            dres=(double)pow(4, i);
            printf("%i\t%e\n", i, dres);
        }
    }

    return 0;
}
