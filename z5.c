/* z5: inflates a gzipped file. valgrind says OK, if gzip is smaller than uncompressed file, which is what you'd expect
 * However, if the gzipped file is bigger, then the compfsz is not updated to the smaller value and valgrind will complain */
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<zlib.h>

/* give a general chunksize:1<<14: */
#define CHUNK 64
#define MAXL 4

size_t gettotsysmem(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}

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
                    putchar(out[i]);
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
int main(int argc, char **argv)
{
    int ret;

    /* argument accounting */
    if(argc!=2) {
        printf("Error. Pls supply 2 arguments 1) input gzipped filename and 2) output filename\n");
        exit(EXIT_FAILURE);
    }
    char *dotp; /* dot pointer */
    dotp=strrchr(argv[1], '.');
    if((dotp==NULL) & (!strncmp("gz", dotp+1, 2)) ) {
        printf("Error. The input file must have the gz extension. Not a water tight indicator of course, but for appearances sake, it should have one.\n");
        exit(EXIT_FAILURE);
    }

    FILE *fpa=fopen(argv[1], "r");
    size_t compfsz = fszfind(fpa);
    size_t tssz = gettotsysmem();
    printf("Input file is %zu bytes, which is %2.1f%% of system memory.\n", compfsz, 100*(float)compfsz/tssz); 

    unsigned char *bf=malloc(compfsz*sizeof(unsigned char));
    ret = infla(fpa, &bf, &compfsz); // yes, this function DOES take care of enlarging bf as will no doubt be necessary !!!
    fclose(fpa);

    if (ret != Z_OK)
        zerr(ret);

    printf("De-compressed output file is %zu bytes, which is %2.1f%% of system memory.\n", compfsz, 100*(float)compfsz/tssz); 
    // printf("%.*s", (int)compfsz, bf); 
//    printf("%s", bf); 
    int i;
    for(i=0;i<compfsz;++i) 
        putchar(bf[i]);

    free(bf);
    return ret;
}
