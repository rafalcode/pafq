#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GENBUF 2

typedef struct  /* optstruct, a struct for the options */
{
    int aflg, bflg;
    char *cval;
    char **inputs;
    int numinps;
} optstruct;

int catchopts(optstruct *opstru, int argc, char **argv)
{
    int index, c, bfsz=GENBUF;
    opterr = 0;

    while ((c = getopt (argc, argv, "abc:")) != -1)
        switch (c) {
            case 'a':
                opstru->aflg = 1;
                break;
            case 'b':
                opstru->bflg = 1;
                break;
            case 'c':
                opstru->cval = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                abort();
        }

    opstru->numinps=0;
    if(optind<argc) {
        opstru->inputs=malloc(bfsz*sizeof(char*));
        for (index = optind; index < argc; index++) {
            if(opstru->numinps==bfsz) {
                bfsz += GENBUF;
                opstru->inputs = realloc(opstru->inputs, bfsz*sizeof(char*));
            }
            opstru->inputs[opstru->numinps++]= argv[index]; /* all these will ahve to be check to ensure they are proper files. */
        }
        opstru->inputs = realloc(opstru->inputs, opstru->numinps*sizeof(char*)); /* normalize */
    }

    return 0;
}

int main (int argc, char **argv)
{
    int i;
    optstruct opstru={0};
    catchopts(&opstru, argc, argv);
    printf ("Flag values: aflag = %d, bflag = %d, cvalue = %s. %i input filenames:\n", opstru.aflg, opstru.bflg, opstru.cval, opstru.numinps);
    for(i=0;i<opstru.numinps;i++)
        printf("%s ", opstru.inputs[i]);
    printf("\n"); 
    for(i=0;i<opstru.numinps;i++)
        printf("%s ", opstru.inputs[i]);

    /* Let's not forget to free that array of character pointers! */
    free(opstru.inputs);

    return 0;
}
