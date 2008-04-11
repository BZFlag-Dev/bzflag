/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/**
 *
 * @brief
 * convert doubles to and from host/network format
 *
 * Library routines for conversion between the local host 64-bit
 * ("double precision") representation, and 64-bit IEEE double
 * precision representation, in "network order", ie, big-endian, the
 * MSB in byte [0], on the left.
 *
 * As a quick review, the IEEE double precision format is as follows:
 * sign bit, 11 bits of exponent (bias 1023), and 52 bits of mantissa,
 * with a hidden leading one (0.1 binary).
 *
 * When the exponent is 0, IEEE defines a "denormalized number", which
 * is not supported here.
 *
 * When the exponent is 2047 (all bits set), and:
 *	all mantissa bits are zero,
 *	value is infinity*sign,
 *	mantissa is non-zero, and:
 *		msb of mantissa=0:  signaling NAN
 *		msb of mantissa=1:  quiet NAN
 *
 * Note that neither the input or output buffers need be word aligned,
 * for greatest flexability in converting data, even though this
 * imposes a speed penalty here.
 *
 * These subroutines operate on a sequential block of numbers, to save
 * on subroutine linkage execution costs, and to allow some hope for
 * vectorization.
 *
 * These routines originated and were derived from BRL-CAD sources.
 */

#include "common.h"

#include <iostream>
#include <string.h>


typedef enum {
    BZ_LITTLE_ENDIAN    = 1234, /* LSB first: i386, VAX order */
    BZ_BIG_ENDIAN       = 4321, /* MSB first: 68000, IBM, network order */
    BZ_PDP_ENDIAN       = 3412  /* LSB first in word, MSW first in long */
} bz_endian_t;

static const int SIZEOF_NETWORK_DOUBLE = 8;


inline bz_endian_t
bz_byteorder()
{
    const union bob {
        unsigned long i;
        unsigned char c[sizeof(unsigned long)];
    } b = {1};

   /* give run-time test preference to compile-time endian, tested
    * much faster than stashing in a static.
    */
#ifdef WORDS_BIGENDIAN
    if (b.c[sizeof(unsigned long)-1])
        return BZ_BIG_ENDIAN;
    if (b.c[0])
        return BZ_LITTLE_ENDIAN;
#else
    if (b.c[0])
        return BZ_LITTLE_ENDIAN;
    if (b.c[sizeof(unsigned long)-1])
        return BZ_BIG_ENDIAN;
#endif
    if (b.c[1])
        return BZ_PDP_ENDIAN;
    
    return (bz_endian_t)0;
}


/**
 * Host to Network Doubles
 */
void
htond(register unsigned char *out, register const unsigned char *in, int count)
{
    register int i;

    switch (bz_byteorder()) {
	case BZ_BIG_ENDIAN:
	    /*
	     * First, the case where the system already operates in
	     * IEEE format internally, using big-endian order.  These
	     * are the lucky ones.
	     */
	    memcpy(out, in, count*8);
	    return;
	case BZ_LITTLE_ENDIAN:
	default:
	    /*
	     * This machine uses IEEE, but in little-endian byte order
	     */
	    for ( i=count-1; i >= 0; i-- )  {
		*out++ = in[7];
		*out++ = in[6];
		*out++ = in[5];
		*out++ = in[4];
		*out++ = in[3];
		*out++ = in[2];
		*out++ = in[1];
		*out++ = in[0];
		in += SIZEOF_NETWORK_DOUBLE;
	    }
	    return;
    }
}


/**
 * Network to Host Doubles
 */
void
ntohd(register unsigned char *out, register const unsigned char *in, int count)
{
    register int i;

    switch (bz_byteorder()) {
	case BZ_BIG_ENDIAN:
	    /*
	     *  First, the case where the system already operates in
	     *  IEEE format internally, using big-endian order.  These
	     *  are the lucky ones.
	     */
	    if ( sizeof(double) != SIZEOF_NETWORK_DOUBLE )
		std::cerr << "ntohd:  sizeof(double) != SIZEOF_NETWORK_DOUBLE" << std::endl;
	    memcpy(out, in, count*SIZEOF_NETWORK_DOUBLE);
	    return;
	case BZ_LITTLE_ENDIAN:
	default:
	    /*
	     * This machine uses IEEE, but in little-endian byte order
	     */
	    for ( i=count-1; i >= 0; i-- )  {
		*out++ = in[7];
		*out++ = in[6];
		*out++ = in[5];
		*out++ = in[4];
		*out++ = in[3];
		*out++ = in[2];
		*out++ = in[1];
		*out++ = in[0];
		in += SIZEOF_NETWORK_DOUBLE;
	    }
	    return;
    }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
