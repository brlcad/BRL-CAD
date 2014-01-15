/*                          B I T V . C
 * BRL-CAD
 *
 * Copyright (c) 2004-2014 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bu.h"


/**
 * Private 32-bit recursive reduction using "SIMD Within A Register"
 * (SWAR) to count the number of one bits in a given integer. The
 * first step is mapping 2-bit values into sum of 2 1-bit values in
 * sneaky way. This technique was taken from the University of
 * Kentucky's Aggregate Magic Algorithms collection.
 *
 * LLVM 3.2 complains about a static inline function here, so use a macro instead
 */
#define COUNT_ONES32(x) { \
    x -= ((x >> 1) & 0x55555555); \
    x  = (((x >> 2) & 0x33333333) + (x & 0x33333333)); \
    x  = (((x >> 4) + x) & 0x0f0f0f0f); \
    x += (x >> 8); \
    x += (x >> 16); \
    x &= 0x0000003f; \
}


/**
 * Private 32-bit recursive reduction using "SIMD Within A Register"
 * (SWAR) to compute a base-2 integer logarithm for a given integer.
 * This technique was taken from the University of Kentucky's
 * Aggregate Magic Algorithms collection.
 *
 * LLVM 3.2 complains about a static inline function here, so use a macro instead
 */
#define FLOOR_ILOG2(x) { \
    x |= (x >> 1); \
    x |= (x >> 2); \
    x |= (x >> 4); \
    x |= (x >> 8); \
    x |= (x >> 16); \
    x >>= 1; \
    COUNT_ONES32(x) \
}


/**
 * Wrap the above private routines for computing the bitv shift size.
 * Users should not call this directly, instead calling the
 * BU_BITV_SHIFT macro instead.
 */
inline size_t
bu_bitv_shift(void)
{
    size_t x = sizeof(bitv_t) * 8;

    FLOOR_ILOG2(x);

    return x;
}


struct bu_bitv *
bu_bitv_new(size_t nbits)
{
    struct bu_bitv *bv;
    size_t bv_bytes;
    size_t total_bytes;

    bv_bytes = BU_BITS2BYTES(nbits);
    total_bytes = sizeof(struct bu_bitv) - 2*sizeof(bitv_t) + bv_bytes;

    /* allocate bigger than struct, bits array extends past the end */
    bv = (struct bu_bitv *)bu_malloc(total_bytes, "struct bu_bitv");

    /* manually initialize */
    BU_LIST_INIT_MAGIC(&(bv->l), BU_BITV_MAGIC);
    bv->nbits = bv_bytes * 8;
    BU_BITV_ZEROALL(bv);

    return bv;
}


void
bu_bitv_free(struct bu_bitv *bv)
{
    BU_CK_BITV(bv);

    bv->l.forw = bv->l.back = BU_LIST_NULL;	/* sanity */
    bu_free((char *)bv, "struct bu_bitv");
}


void
bu_bitv_clear(struct bu_bitv *bv)
{
    BU_CK_BITV(bv);

    BU_BITV_ZEROALL(bv);
}


void
bu_bitv_or(struct bu_bitv *ov, const struct bu_bitv *iv)
{
    register bitv_t *out;
    register const bitv_t *in;
    register size_t words;

    if (UNLIKELY(ov->nbits != iv->nbits))
	bu_bomb("bu_bitv_or: length mis-match");

    out = ov->bits;
    in = iv->bits;
    words = BU_BITS2WORDS(iv->nbits);
#ifdef VECTORIZE
    for (--words; words >= 0; words--)
	out[words] |= in[words];
#else
    while (words-- > 0)
	*out++ |= *in++;
#endif
}


void
bu_bitv_and(struct bu_bitv *ov, const struct bu_bitv *iv)
{
    register bitv_t *out;
    register const bitv_t *in;
    register size_t words;

    if (UNLIKELY(ov->nbits != iv->nbits))
	bu_bomb("bu_bitv_and: length mis-match");

    out = ov->bits;
    in = iv->bits;
    words = BU_BITS2WORDS(iv->nbits);
#ifdef VECTORIZE
    for (--words; words >= 0; words--)
	out[words] &= in[words];
#else
    while (words-- > 0)
	*out++ &= *in++;
#endif
}


void
bu_bitv_vls(struct bu_vls *v, register const struct bu_bitv *bv)
{
    int seen = 0;
    register size_t i;
    size_t len;

    BU_CK_VLS(v);
    BU_CK_BITV(bv);

    len = bv->nbits;

    bu_vls_strcat(v, "(");

    /* Visit all the bits in ascending order */
    for (i=0; i<len; i++) {
	if (BU_BITTEST(bv, i) == 0)
	    continue;
	if (seen)
	    bu_vls_strcat(v, ", ");
	bu_vls_printf(v, "%lu", i);
	seen = 1;
    }
    bu_vls_strcat(v, ") ");
}


void
bu_pr_bitv(const char *str, register const struct bu_bitv *bv)
{
    struct bu_vls v = BU_VLS_INIT_ZERO;

    BU_CK_BITV(bv);

    if (str) {
	bu_vls_strcat(&v, str);
	bu_vls_strcat(&v, ": ");
    }
    bu_bitv_vls(&v, bv);
    bu_log("%s", bu_vls_addr(&v));
    bu_vls_free(&v);
}


void
bu_bitv_to_hex(struct bu_vls *v, const struct bu_bitv *bv)
{
    size_t word_count = 0;
    size_t chunksize = 0;
    /* necessarily volatile to keep the compiler from complaining
     * about unreachable code during optimization.
     */
    volatile size_t BVS = sizeof(bitv_t);

    BU_CK_VLS(v);
    BU_CK_BITV(bv);

    word_count = bv->nbits / 8 / BVS;
    bu_vls_extend(v, word_count * BVS * 2 + 1);

    while (word_count--) {
	chunksize = BVS;
	while (chunksize--) {
	    unsigned long val = (unsigned long)((bv->bits[word_count] & ((bitv_t)(0xff)<<(chunksize*8))) >> (chunksize*8)) & (bitv_t)0xff;
	    bu_vls_printf(v, "%02lx", val);
	}
    }
}


struct bu_bitv *
bu_hex_to_bitv(const char *str)
{
    char abyte[3];
    const char *str_start;
    unsigned int len=0;
    int bytes;
    struct bu_bitv *bv;
    unsigned long c;
    size_t word_count;
    size_t chunksize = 0;
    volatile size_t BVS = sizeof(bitv_t);

    abyte[2] = '\0';

    /* skip over any initial white space */
    while (isspace((int)(*str)))
	str++;

    str_start = str;
    /* count hex digits */
    while (isxdigit((int)(*str)))
	str++;

    len = str - str_start;

    if (len < 2 || len % 2) {
	/* Must be two digits per byte */
	bu_log("ERROR: bitv length is not a multiple of two.\nIllegal hex bitv: [%s]\n", str_start);
	return (struct bu_bitv *)NULL;
    }

    bytes = len / 2; /* two hex digits per byte */
    word_count = bytes / BVS;
    chunksize = bytes % BVS;
    if (chunksize == 0) {
	chunksize = BVS;
    } else {
	/* handle partial chunk before using chunksize == BVS */
	word_count++;
    }

    bv = bu_bitv_new(len * 4); /* 4 bits per hex digit */

    str = str_start;
    while (word_count--) {
	while (chunksize--) {
	    /* get next two hex digits from string */
	    abyte[0] = *str++;
	    abyte[1] = *str++;

	    /* convert into an unsigned long */
	    c = strtoul(abyte, (char **)NULL, 16);

	    /* set the appropriate bits in the bit vector */
	    bv->bits[word_count] |= (bitv_t)c<<(chunksize*8);
	}
	chunksize = BVS;
    }

    return bv;
}


struct bu_bitv *
bu_bitv_dup(register const struct bu_bitv *bv)
{
    struct bu_bitv *bv2;

    if (!bv)
	return bu_bitv_new(0);

    bv2 = bu_bitv_new(bv->nbits);
    bu_bitv_or(bv2, bv);

    return bv2;
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
