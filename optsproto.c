#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct  /* optstruct, a struct for the options */
{
    int aflg, bflg;
    char *cval;
} optstruct;

int catchopts(optstruct *opstru, int argc, char **argv)
{
    int index, c;
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

    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

    return 0;
}

int main (int argc, char **argv)
{
    optstruct opstru={0};
    catchopts(&opstru, argc, argv);
    printf ("aflag = %d, bflag = %d, cvalue = %s\n", opstru.aflg, opstru.bflg, opstru.cval);

    return 0;
}
