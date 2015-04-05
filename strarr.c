/* strarr_scfl.c building on lnarr.c to produce a program which slurps each line of a text file nto a SINGLE array EXCEPT for the first line however
   Copyright (C) 2014  Ramon Fallon
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LNBUF 8
#define GSTRBUF 8 /* general string buffer */
/* quick macro for conditionally enlarging a general native type array */
#define CONDREALLOC(x, b, c, a, t); \
    if((x)>=((b)-3)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        memset((a)+(b)-(c), 0, (c)*sizeof(t)); \
    }
//        memset(((a)-(c)), 0, (c)*sizeof(t));

/* quick macro for conditionally enlarging a char pointer, space always available for final null char */
#define CONDREALLOCP(x, b, c, a); \
    if((x)==((b)-2)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(char)); \
        memset((a)+(b)-(c), '\0', (c)*sizeof(char)); \
    }

/* quick macro for conditionally enlarging a DOUBLE char pointer, space always available for final null char */
#define CONDREALLODCP(x, b, c, a, i, strbuf); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(char*)); \
        for((i)=(b)-(c);(i)<(b);++(i)) \
            (a)[(i)]=calloc((strbuf), sizeof(char)); \
    }

/* quick macro for conditionally enlarging a numeric native type array and setting extended memory to zero */
#define CONDREALLOD(x, b, c, a, t); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        memset((a)+(b)-(c), 0, (c)*sizeof(t)); \
    }

/* quick macro for conditionally enlarging a twin digit and char array based on same lnbuf, a2 MUST be a double char ptr */
#define CONDREALLOTDCA(x, b, c, a, a1, a2, t, i, strbuf); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        (a1)=realloc((a1), (b)*sizeof(t)); \
        (a2)=realloc((a2), (b)*sizeof(char*)); \
        for((i)=(b)-(c);(i)<(b);++(i)) \
            (a2)[(i)]=calloc((strbuf), sizeof(char)); \
        memset((a)+(b)-(c), 0, (c)*sizeof(t)); \
    }

typedef struct /* strua_t */
{
    char **stra; /* the string array, size of each string will be one less than ua */
    unsigned *ua;
    unsigned *plwc; /* word per line counts */
    unsigned uasz;
} strua_t; /* String into unsigned array type */

void f2strua_t(char *fname, strua_t **lnarr_p)
{
    FILE *fin=fopen(fname, "r");
    int c;
    unsigned lnsz=0, lnbuf=LNBUF, strbuf=GSTRBUF, j;
    unsigned lidx=1;
    unsigned char inw=0;
    unsigned plwc=0;

    for(;;) {
        c=fgetc(fin);
        if(c == EOF) break;
        if(c == '\n') {
            (*lnarr_p)->ua[lidx-1]=lnsz;
            (*lnarr_p)->plwc[lidx-1]=plwc;
            (*lnarr_p)->stra[lidx-1][lnsz]='\0';
            CONDREALLOTDCA(lidx-1, lnbuf, LNBUF, (*lnarr_p)->ua, (*lnarr_p)->plwc, (*lnarr_p)->stra, unsigned, j, GSTRBUF);
            lidx++;
            lnsz=0;
            strbuf=GSTRBUF;
            inw=0;
            plwc=0;
        } else {
            CONDREALLOCP(lnsz, strbuf, GSTRBUF, (*lnarr_p)->stra[lidx-1]);
            (*lnarr_p)->stra[lidx-1][lnsz]=c;
            if( (!inw) & ((c!=' ') | (c!='\t')) ) {
                inw=1;
                plwc++;
            } else if( inw & ((c==' ') | (c=='\t')) )
                inw=0;
            lnsz++;
        }
    }
    (*lnarr_p)->uasz=lidx-1;
    (*lnarr_p)->ua=realloc((*lnarr_p)->ua, (*lnarr_p)->uasz*sizeof(unsigned));
    for(j=(*lnarr_p)->uasz;j<lnbuf;++j) 
        free((*lnarr_p)->stra[j]);
    (*lnarr_p)->stra=realloc((*lnarr_p)->stra, (*lnarr_p)->uasz*sizeof(char*));

    fclose(fin);
    return;
}

strua_t *crea_strua_t(void) /* with minimum memory allocations as well */
{
    unsigned j;
    strua_t *lnarr_p=malloc(sizeof(strua_t));
    lnarr_p->ua=calloc(LNBUF, sizeof(unsigned));
    lnarr_p->plwc=calloc(LNBUF, sizeof(unsigned));
    lnarr_p->stra=malloc(LNBUF*sizeof(char*));
    for(j=0;j<LNBUF;++j) 
        lnarr_p->stra[j]=calloc(GSTRBUF, sizeof(char));
    return lnarr_p;
}

unsigned *repseekarray(strua_t *lnarr_p, unsigned *rpasz)
{
    unsigned j, jj, m, buf=GSTRBUF;
    unsigned *rpa=calloc(buf, sizeof(unsigned));

    j=jj=m=0;
    while(jj<lnarr_p->uasz) {
        CONDREALLOC(m, buf, GSTRBUF, rpa, unsigned);
        rpa[m]=j;
        rpa[m+1]=lnarr_p->ua[j];
        do {
            rpa[m+2]++;
            jj++;
        } while( (jj<lnarr_p->uasz) & (lnarr_p->ua[jj] == lnarr_p->ua[j]) );
        j=jj;
        m+=3;
    }
    rpa=realloc(rpa, m*sizeof(unsigned));
   *rpasz=m; 
   return rpa;
}

void prto_linesizes(char *fname, strua_t *lnarr_p)
{
    unsigned j;
    printf("File \"%s\" has %u lines; per line chars is :\n", fname, lnarr_p->uasz); 
    for(j=0;j<lnarr_p->uasz;++j) 
        printf("%u ", lnarr_p->ua[j]);
    printf("\n"); 
    for(j=0;j<lnarr_p->uasz;++j) 
        printf("%u ", lnarr_p->plwc[j]);
    printf("\n"); 
    return;
}

void prto_strua_t(strua_t *lnarr_p)
{
    unsigned j;
    printf("Input has %u lines; per line chars is :\n", lnarr_p->uasz); 
    printf("Listing of the lines\n"); 
    for(j=0;j<lnarr_p->uasz;++j) 
        // printf("l%3u:\"%s\"(%u)\n", j, lnarr_p->stra[j], lnarr_p->ua[j]);
        printf("%s\n", lnarr_p->stra[j]);
    return;
}

void free_strua_t(strua_t **lnarr_p)
{
    unsigned j;
    free((*lnarr_p)->ua);
    free((*lnarr_p)->plwc);
    for(j=0;j<(*lnarr_p)->uasz;++j) 
        free((*lnarr_p)->stra[j]);
    free((*lnarr_p)->stra);
    free((*lnarr_p));
    return;
}

int main(int argc, char *argv[])
{
    /* argument accounting: remember argc, the number of arguments, _includes_ the executable */
    if(argc!=2) {
        printf("Error. Pls supply argument (name of file).\n");
        exit(EXIT_FAILURE);
    }

    /* convert this file into a line array */
    strua_t *lnarr_p=crea_strua_t();
    f2strua_t(argv[1], &lnarr_p);
    prto_linesizes(argv[1], lnarr_p);
    unsigned rpsz=0;
    unsigned *rpa=repseekarray(lnarr_p, &rpsz);

    int i;
    printf("repseek array is of size %u\n", rpsz); 
    for(i=0;i<rpsz;i+=3) 
        printf("%u/%u/%u ", rpa[i], rpa[i+1], rpa[i+2]);
    printf("\n"); 

    free(rpa);
    free_strua_t(&lnarr_p);

    return 0;
}
