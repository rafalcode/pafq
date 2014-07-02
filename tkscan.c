#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HBUF 4 /* harshly sized buffer */
#define GBUF 32

/* Quick macro for conditionally enlarging an array */
#define CONDREALLOC(x, b, c, a, t); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
    }

struct cl /* "character link" structure ... a linked list of characters ... essentially this is the token scanner. Links are looped into a ring equal to the szie of the token */
{
    char c;
    struct cl *n;
};
typedef struct cl cl_t;

typedef struct /* bva_t */
{
    char *bca; /* base call array */
    char *va; /* value array */;
    unsigned sz;
    unsigned bf;
} bva_t; /* base value array type */


bva_t *crea_bva(void)
{
    bva_t *sr=malloc(sizeof(bva_t)); /* short read */
    sr->bca=malloc(HBUF*sizeof(char));
    sr->va=malloc(HBUF*sizeof(char));
    sr->sz=0;
    sr->bf=HBUF;
    return sr;
}


void free_bva(bva_t *sr)
{
    free(sr->bca);
    free(sr->va);
    free(sr);
}

char *scas(char *s, int *sz) /* a function to scan the token strings and convert newlines double symbols to their proper 0x0A equivalents. Also the size is returned in the arguments */
{
    int i, j=0;
    int tsz=(int)strlen(s);
    char *p=s, *ts= calloc(tsz+1, sizeof(char));
    char oldc='\0';
    for(i=0; i<tsz; i++) {
        if(oldc != '\\') {
            if( *(p+i) != '\\') {
                *(ts+j) = *(p+i);
                j++;
            } else {
                oldc=*(p+i);
                continue;
            }
        } else {
            switch(*(p+i)) {
                case 'n':
                    *(ts+j) ='\n';
                    j++;
                    oldc='\n';
                    continue;
                case 't':
                    *(ts+j) ='\t';
                    j++;
                    oldc='\t';
                    continue;
                case 's':
                    *(ts+j) =' ';
                    j++;
                    oldc=' ';
                    continue;
                default: /* placeholder for other symbols */
                    printf("Error. Only newline, tab and space  double symbols allowed.\n");
                    free(ts);
                    exit(EXIT_FAILURE);
            }
        }
    }
    *(ts+j)='\0';
    ts=realloc(ts, (j+1)*sizeof(char));
    // *sz=(unsigned)strlen(ts);
    *sz=j;
    return ts;
}

void freering(cl_t *mou)
{
    cl_t *st=mou->n, *nxt;
    while (st !=mou) {
        nxt=st->n;
        free(st);
        st=nxt;
    }
    free(mou);
}

char cmpcl(cl_t *st1, cl_t *st2)
{
    char yes=1;
    cl_t *st1_=st1, *st2_=st2;
    do {
        if(st1_->c != st2_->c) {
            yes=0;
            break;
        }
        st1_=st1_->n;
        st2_=st2_->n;
    } while (st1_ !=st1);
    return yes;
}

cl_t *creacl(int ssz) /* create empty ring of size ssz */
{
    int i;
    cl_t *mou /* mouth with a tendency to eat the tail*/, *ttmp /* type temporary */;

    mou=malloc(sizeof(cl_t));
    mou->c='\0';
    ttmp=mou;
    for(i=0;i<ssz-1;++i) {
        ttmp->n=malloc(sizeof(cl_t));
        ttmp->n->c='\0';
        ttmp=ttmp->n; /* with ->nmove on */
    }
    ttmp->n=mou; /* with ->nmove on */
    return mou;
}

cl_t *creaclstr(char *stg, int ssz) /* create empty ring of size ssz */
{
    int i;
    cl_t *mou /* mouth with a tendency to eat the tail*/, *ttmp /* type temporary */;

    mou=malloc(sizeof(cl_t));
    mou->c=stg[ssz-1];
    ttmp=mou;
    // for(i=ssz-2;i>=0;--i) { /*wrong sense */
    for(i=0;i<ssz-1;++i) {
        ttmp->n=malloc(sizeof(cl_t));
        ttmp->n->c=stg[i];
        ttmp=ttmp->n; /* with ->nmove on */
    }
    ttmp->n=mou; /* with ->nmove on */
    return mou;
}

inline void stopatma(FILE *fin, char *str2ma, size_t *sqidx, bva_t *pa)
{
    int c, ssz;
    char *scanstr=scas(str2ma, &ssz);
    if(ssz<1) {
        printf("Error. The sliding window must be 1 or more.\n");
        exit(EXIT_FAILURE);
    }
    cl_t *d2ma=creaclstr(scanstr, ssz); /* the cl_t to match, so to speak */
    cl_t *mou=creacl(ssz);
    unsigned nmoves=0;
    while( ( (c = fgetc(fin)) != EOF) ) {
        CONDREALLOC(nmoves, pa->bf, HBUF, pa->bca, char);
        pa->bca[nmoves]=c;
        mou->c = c;
        if((mou->n->c) && (cmpcl(mou, d2ma))) {
            // printf("yes: at %zu\n", sqidx);
            goto outro;
        }
        mou=mou->n;
        nmoves++;
    }

    printf("Abnormal c or no match or unplanned EOF was reached. ");
    printf("Last seen c was %x\n", c); 
    exit(EXIT_FAILURE);

outro:    
    *sqidx+=nmoves-ssz+1;
    pa->sz=nmoves-ssz+1;
    pa->bca=realloc(pa->bca, pa->sz*sizeof(char));
    free(scanstr);
    freering(mou);
    freering(d2ma);

    return;
}

int main(int argc, char *argv[])
{
    if(argc!=3) {
        printf("Error. Pls supply 2 arguments: 1) name of text file 2) token string to match\n");
        exit(EXIT_FAILURE);
    }
    size_t sqidx=0;

    FILE *fin=fopen(argv[1], "r");

    bva_t *pa=crea_bva();
    stopatma(fin, argv[2], &sqidx, pa);
    printf("Match found starting at charidx=%zu\n", sqidx);
    fclose(fin);

    int i;
    for(i=0;i<pa->sz;++i) 
       putchar(pa->bca[i]);
    putchar('\n');
    free_bva(pa);

    return 0;
}
