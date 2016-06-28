/*
 * sha1sum.h
 *
 *  Created on: 2011-7-15
 *      Author: root
 */
#ifndef SHA1SUM_H_
#define SHA1SUM_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


#undef BIG_ENDIAN_HOST
typedef unsigned int u32;

/****************
 * Rotate a 32 bit integer by n bytes
 ****************/
#if defined(__GNUC__) && defined(__i386__)
static inline u32 rol( u32 x, int n)
{
	__asm__("roll %%cl,%0"
            :"=r" (x)
            :"0" (x),"c" (n));
	return x;
}
#else
#define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )
#endif

typedef struct {
    u32  h0,h1,h2,h3,h4;
    u32  nblocks;
    unsigned char buf[64];
    int  count;
} SHA1_CONTEXT;

void sha1_init( SHA1_CONTEXT *hd );
static void transform( SHA1_CONTEXT *hd, unsigned char *data );
static void sha1_write( SHA1_CONTEXT *hd, unsigned char *inbuf, size_t inlen);
static void sha1_final(SHA1_CONTEXT *hd);
int GetSha1Sum(char *fns,char Sum[]);


#endif
