#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<assert.h>
#include<zlib.h>
#include <locale.h>

#define EBUF 2
#define HBUF 64 /* somewhat harshly sized buffer */
#define GBUF 32
#define CHUNK 64
#define MAXL 4
unsigned ifsz;
#define ENDB "\n+\n" // this is the way fastq files are built

/* Quick macro for conditionally enlarging an array */
#define CONDREALLOC(x, b, c, a, t); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
    }

typedef struct /* overall summmry, summarya per fastq-file struct */
{
    size_t totb /* total number of bases called */;
    size_t nsqs;
} osmmry_t;

typedef struct /* smmry_, a per fastq-file struct */
{
    char mn, mx; /* this is for max and min qual values on the bases: note these are phred encoded: so just chars */
    size_t totb /* total number of bases called */, totbacval /* total number of bases above a maximum value */;
    unsigned mnnbps, mxnbps; /* min & max number of bases per sequence */
} smmry_t; /* Min and max value type */

struct cl /* "character link" structure ... a linked list of characters ... essentially this is the token scanner. Links are looped into a ring equal to the szie of the token */
{
    char c;
    struct cl *n;
};
typedef struct cl cl_t;

typedef struct /* bva_t */
{
    char *id; /* id line */
    char *bca; /* base call array */
    char *va; /* value array */;
    unsigned sz;
    unsigned idsz;
    unsigned bf;
    unsigned avist, avext; /* acceptable value index start, acceptable value index extent */
    unsigned txav; /* number of times acval crossed */
} bva_t; /* base value array type */

size_t fszfind(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    size_t fbytsz = ftell(fp);
    rewind(fp);
    return fbytsz;
}

int infla(FILE *source, unsigned char **bf, size_t *bfsz)
{
    unsigned i, j, lcou, used_up;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    unsigned char *tbf=*bf; /* possibly temp buffer */
    size_t tbfsz=*bfsz; /* possibly temp buffer SIZE */
    size_t last_TO=0;

    /* define and then allocate inflate state: need strm and head */
    z_stream strm={0};
    /* don't bother try to one-line the following ... NULL ptrs don't work that way! */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = Z_NULL;
    strm.next_out = Z_NULL;
    strm.msg = Z_NULL;

    int ret = inflateInit2(&strm , 16+MAX_WBITS); /* mere activation, source is not involved yet */
    if (ret != Z_OK)
        return ret;

    /* now we go for the header */
    gz_header head={0};
    ret = inflateGetHeader(&strm , &head);
    if (ret != Z_OK)
        return ret;

    lcou=0;
    j=0;
    do { /* decompress until deflate stream ends or end of file */
        strm.avail_in = fread(in, sizeof(unsigned char), CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0) /* fread returned nothing */
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR: case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }

            used_up = CHUNK - strm.avail_out;
            if(used_up != 0) { /* i.e. the output buf was definitely used */
                if(tbfsz < strm.total_out) {
                    tbfsz=strm.total_out;
                    tbf=realloc(tbf, tbfsz*sizeof(unsigned char));
                }
                memcpy(tbf+last_TO, out, used_up);

                for(i=0;i<used_up;++i) {
                    if(lcou == MAXL)
                        break;
                    if(out[i] == '\n')
                        lcou++;
                    // putchar(out[i]);  // for printing out some stuff.
                }
                last_TO = strm.total_out;
            }

        } while (strm.avail_out == 0); /* while the outb is being fully used */

        /* done when inflate() says it's done */
        j++;
    } while (ret != Z_STREAM_END);

    /* clean up */
    (void)inflateEnd(&strm);

    /* note this step is needed, realloc has probably changed the memory loc for both buffer and (shockhorror) even its size! */
    *bf=tbf;
    *bfsz=tbfsz;

    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void zerr(int ret) /* report a zlib or i/o error */
{
    fputs("zlib routine: ", stderr);
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
        case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
        case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
        case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
void prtele(bva_t **paa, unsigned nel) /* print a certain element */
{
    int i;
    for(i=0;i<paa[nel]->idsz;++i) 
        putchar(paa[nel]->id[i]);
    putchar('\n');
    for(i=0;i<paa[nel]->sz;++i) 
        putchar(paa[nel]->bca[i]);
    putchar('\n');
    for(i=0;i<paa[nel]->sz;++i) 
        putchar(paa[nel]->va[i]);
    putchar('\n');
}

void prtifsz(bva_t **paa, unsigned ecou) /* quick and dirty: print sequences over a certain size */
{
    int i, j;
    ifsz =1000;
    for(j=0;j<ecou;++j) {
        if(paa[j]->sz >= ifsz) {
            for(i=0;i<paa[j]->idsz;++i) 
                putchar(paa[j]->id[i]);
            putchar('\n');
            for(i=0;i<paa[j]->sz;++i) 
                putchar(paa[j]->bca[i]);
            putchar('\n');
            for(i=0;i<paa[j]->sz;++i) 
                putchar(paa[j]->va[i]);
            putchar('\n');
        }
    }
}

bva_t *crea_bva(void)
{
    bva_t *sr=malloc(sizeof(bva_t)); /* short read */
    sr->id=malloc(HBUF*sizeof(char));
    sr->bca=malloc(HBUF*sizeof(char));
    sr->va=NULL; /* this will be malloc'd when we know the size of the base call array */
    sr->sz=0;
    sr->idsz=0;
    sr->bf=HBUF;
    return sr;
}


void free_bva(bva_t *sr)
{
    free(sr->id);
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

char fillbctoma(unsigned char *bf, size_t compfsz, size_t *bfidx, char *str2ma, size_t *sqidx, bva_t *pa) /* fill base call array till match */
{
    int c, ssz;
    size_t currbfidx=*bfidx;
    char *scanstr=scas(str2ma, &ssz);
    if(ssz<1) {
        printf("Error. The sliding window must be 1 or more.\n");
        exit(EXIT_FAILURE);
    }
    cl_t *d2ma=creaclstr(scanstr, ssz); /* the cl_t to match, so to speak */
    cl_t *mou=creacl(ssz);
    unsigned nmoves=0;
    while( currbfidx < compfsz) {
        c = (int)bf[currbfidx++];
        CONDREALLOC(nmoves, pa->bf, HBUF, pa->bca, char);
        pa->bca[nmoves]=mou->c= c;
        if((mou->n->c) && (cmpcl(mou, d2ma))) {
            // printf("yes: at %zu\n", sqidx);
            goto outro;
        }
        mou=mou->n;
        nmoves++;
    }

    printf("Abnormal c or no match or unplanned EOF was reached. ");
    printf("Last seen c was %x\n", c); 
    *bfidx=currbfidx;
    return 1;

outro:    
    *sqidx+=nmoves-ssz+1;
    pa->sz=nmoves-ssz+1;
    pa->bca=realloc(pa->bca, pa->sz*sizeof(char));
    free(scanstr);
    freering(mou);
    freering(d2ma);
    *bfidx=currbfidx;

    return 0;
}

char fillidtonl(unsigned char *bf, size_t compfsz, size_t *bfidx, bva_t *pa) /* read chars to bva until end of line */
{
    size_t currbfidx=*bfidx;
    int c;
    char STATE=0;
    unsigned cidx=0, idbuf=HBUF;
    for(;;) {
        c = (int)bf[currbfidx++];
        if(currbfidx >= compfsz) // we still want final processed
            return 2;
        else if( c== '\n')
            break;
        else if( (c=='@') & !STATE)
            STATE=1;
        if(STATE) {
            CONDREALLOC(cidx, idbuf, HBUF, pa->id, char);
            pa->id[cidx++]=c;
        }
    }
    if(!STATE)
        return 1;
    pa->idsz=cidx;
    pa->id=realloc(pa->id, pa->idsz*sizeof(char));
    *bfidx=currbfidx;
    return 0;
}

void ncharstova(unsigned char *bf, size_t *bfidx, bva_t *pa, unsigned nchars, smmry_t *smmry) /* read a number of chars to the value array */
{
    int c;
    size_t currbfidx=*bfidx;
    unsigned cidx=0;
    /* We already know nchars */
    pa->va=malloc(nchars*sizeof(char));
    while( ( cidx!= nchars) ) {
        c = (int)bf[currbfidx++];
        pa->va[cidx]=c;
        if(pa->va[cidx]>smmry->mx)
            smmry->mx = pa->va[cidx];
        if(pa->va[cidx]<smmry->mn)
            smmry->mn = pa->va[cidx];
        cidx++;
    }
    *bfidx=currbfidx;
    return;
}

void processfq(char *fname, unsigned char *bf, size_t compfsz, osmmry_t *osm)
{
    size_t sqidx=0UL, bfidx=0UL;
    int i, c;
    unsigned ecou=0, ebuf=EBUF;
    char *nxtok, retval;

    bva_t **paa=malloc(ebuf*sizeof(bva_t*));
    for(i=0;i<ebuf;++i) 
        paa[i]=crea_bva();

    /* want to see what the max and min values are like */
    smmry_t smmry;
    smmry.mn=127;
    smmry.mx=0;
    smmry.totb=0;
    smmry.mnnbps=2048;
    smmry.mxnbps=0;

    for(;;) {
        /* first check if we have space in our array for more sequences */
        if(ecou == ebuf-1) {
            ebuf += EBUF;
            paa=realloc(paa, ebuf*sizeof(bva_t));
            for(i=ebuf-EBUF;i<ebuf;++i) 
                paa[i]=crea_bva();
        }

        /* shoved in because of invalid read error from valgrind ... fillidtonl was being called when there was no sequence left */
        if(bfidx >= compfsz)
            break;
        /* we want to catch the first title line */
        retval=fillidtonl(bf, compfsz, &bfidx, paa[ecou]);
        if(retval==2)
            break;
        else if(retval) {
            printf("Error on reading first line, no \'@\' symbol forthcoming\n");
            exit(EXIT_FAILURE);
        }

        nxtok=ENDB;
        fillbctoma(bf, compfsz, &bfidx, nxtok, &sqidx, paa[ecou]);
        if(paa[ecou]->sz > smmry.mxnbps)
            smmry.mxnbps = paa[ecou]->sz;
        if(paa[ecou]->sz < smmry.mnnbps)
            smmry.mnnbps = paa[ecou]->sz;
        smmry.totb += paa[ecou]->sz;

        ncharstova(bf, &bfidx, paa[ecou], paa[ecou]->sz, &smmry); /* read a number of chars to the value array */
        ecou++;
        c=(int)bf[bfidx++];
        if( bfidx == compfsz-1 )
            break;
        else if( c != '\n' ) {
            printf("Error. What's \"%c\" doing here?\n", c);
            printf("Error. Expected character to be newline at this point. Bailing out.\n");
            exit(EXIT_FAILURE);
        }
    }

    /* now normalize the size of the short read array */
    for(i=ecou;i<ebuf;++i) 
        free_bva(paa[i]);
    paa=realloc(paa, ecou*sizeof(bva_t));

    // OK get ready for print out
    char *fnp=strchr(fname+1, '.'); // +1 to avoid starting dot
    // printf("%-*s\t%'10u\t%'10zu\t%10u\t%10u\t%10c\t%10c\n", (int)(fnp-fname), fname, ecou, smmry.totb, smmry.mxnbps, smmry.mnnbps, smmry.mx, smmry.mn);
    // printf("%-*s\t%'10u\t%'10zu\t%10u\t%10u\t%10c\t%10c\n", (int)(fnp-fname), fname, ecou, smmry.totb, smmry.mxnbps, smmry.mnnbps, smmry.mx, smmry.mn);
    printf("FRN\t%-*s\tNSQ\t%'u\tNBA\t%'zu\tMXSZ\t%u\tMNSZ\t%u\tMXQL\t%c\tMNQL\t%c\n", (int)(fnp-fname), fname, ecou, smmry.totb, smmry.mxnbps, smmry.mnnbps, smmry.mx, smmry.mn);
    osm->totb += smmry.totb;
    osm->nsqs += ecou;

#ifdef DBG
    for(i=0;i<ecou;++i) 
        prtele(paa, i);
#endif
    prtifsz(paa, ecou);

    for(i=0;i<ecou;++i) 
        free_bva(paa[i]);
    free(paa);

    return;
}

int main(int argc, char *argv[])
{
    if(argc==1) {
        printf("Usage: fqzinfo <fastq.gz file 1> [<fastq.gz file 2> etc.]\n");
        printf("A program to give essential details of fastq.gz files in one-line form.\n");
        printf("Abbreviation meanings:\n\tFN fastq.gz file name; NSQ Number of reads(sequences); NBA total number of bases in file; MXSZ Longest sequence/read size; MNSZ Smallest sequence/read size.\n");
       printf("\tMXQL Highest sequence/read quality in Phred coding; MNQL Lowest sequence/read quality in Phred coding.\n");
        exit(EXIT_FAILURE);
    }
    setlocale(LC_NUMERIC, ""); /* for the thousands' separator */
    osmmry_t osm={0};

    /* Let's set out output first */
    // printf("%s\t%10s\t%10s\t%10s\t%10s\t%10s\t%10s\n", "Readset Name", "NumSeqs", "NumBases", "MxReadLen", "MinReadSize", "MaxQualVal", "MinQualVal");

    FILE *fpa;
    size_t compfsz;
    // unsigned char *fbf=malloc(GBUF*sizeof(unsigned char));
    unsigned char *fbf=NULL;
    int i, ret;
    for(i=1;i<argc;i++) {
        fpa=fopen(argv[i], "r");
        if(fpa == NULL) {
            printf("Error opening file with name \"%s\", skipping.\n", argv[i]); 
            continue;
        }
        compfsz = fszfind(fpa);
        fbf=realloc(fbf, compfsz*sizeof(unsigned char));
        ret = infla(fpa, &fbf, &compfsz); // yes, this function DOES take care of enlarging bf as will no doubt be necessary !!!
        processfq(argv[i], fbf, compfsz, &osm);
        fclose(fpa);
    }

    // printf("Overall total sequences = %'zu\n", osm.nsqs);
    // printf("Overall total bases = %'zu\n", osm.totb);

    free(fbf);

    return ret;
}
