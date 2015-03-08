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

#define LNBUF 2
#define GSTRBUF 2 /* general string buffer */
/* quick macro for conditionally enlarging a general native type array */
#define CONDREALLOC(x, b, c, a, t); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
    }

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
#define CONDREALLOTDCA(x, b, c, a, a2, t, i, strbuf); \
    if((x)==((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        (a2)=realloc((a2), (b)*sizeof(char*)); \
        for((i)=(b)-(c);(i)<(b);++(i)) \
            (a2)[(i)]=calloc((strbuf), sizeof(char)); \
        memset((a)+(b)-(c), 0, (c)*sizeof(t)); \
    }

typedef struct
{
    char **stra; /* the string array, size of each string will be one less than ua */
    unsigned *ua;
    unsigned uasz;
} strua_t; /* First Line Plus Unsigned Array Type */

void f2strua_t(char *fname, strua_t **lnarr_p)
{
    FILE *fin=fopen(fname, "r");
    int c;
    unsigned lnsz=0, lnbuf=LNBUF, strbuf=GSTRBUF, j;
    unsigned lidx=1;

    for(;;) {
        c=fgetc(fin);
        if(c == EOF) break;
        if(c == '\n') {
            (*lnarr_p)->ua[lidx-1]=lnsz;
            (*lnarr_p)->stra[lidx-1][lnsz]='\0';
            CONDREALLOTDCA(lidx-1, lnbuf, LNBUF, (*lnarr_p)->ua, (*lnarr_p)->stra, unsigned, j, GSTRBUF);
            lidx++;
            lnsz=0;
            strbuf=GSTRBUF;
        } else {
            CONDREALLOCP(lnsz, strbuf, GSTRBUF, (*lnarr_p)->stra[lidx-1]);
            (*lnarr_p)->stra[lidx-1][lnsz]=c;
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
    lnarr_p->stra=malloc(LNBUF*sizeof(char*));
    for(j=0;j<LNBUF;++j) 
        lnarr_p->stra[j]=calloc(GSTRBUF, sizeof(char));
    return lnarr_p;
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
    prto_strua_t(lnarr_p);

    free_strua_t(&lnarr_p);

    return 0;
}