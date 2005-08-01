/***********************************************************************
 * $Id: mbuf.c,v 1.1 2005/08/01 09:04:50 aki Exp $
 *
 * Memory buffer utility.
 * Copyright (C) 2004 RIKEN. All rights reserved.
 * Written by Aki Hasegawa <aki@gsc.riken.jp>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ***********************************************************************/

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "mbuf.h"

#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

/*
 * Why 64? see libgnu/xalloc.c: x2nrealloc_inline.
 * * 64 bytes is the largest "small" request for the
 * * GNU C library malloc.
 */ 
#define MBUF_DEFAULT_MAX    64

/*======================================================================
 * forward declarations
 *======================================================================*/

inline static size_t clp2(size_t x);

/*======================================================================
 * memory buffer functions
 *======================================================================*/

mbuf_t *mbuf_new(const void *p, size_t n)
{
    mbuf_t *mp = (mbuf_t*)malloc(sizeof(mbuf_t));
    if (mp == NULL)
	return NULL;
    if (mbuf_init(mp, p, n) != 0)
	return NULL;
    return mp;
}

int mbuf_init(mbuf_t *mp, const void *p, size_t n)
{
    mp->ptr = NULL;
    mp->max = 0;
    mp->cnt = 0;

    if (mbuf_reserve(mp, ((n > 0) ? n : MBUF_DEFAULT_MAX)) != 0)
	return errno;
    if (n > 0 && p != NULL) {
	memcpy(mp->ptr, p, n);
	mp->cnt = n;
    }
    return 0;
}

void mbuf_free(mbuf_t *mp)
{
    free(mp->ptr), mp->ptr = NULL;
    mp->max = 0;
    mp->cnt = 0;
}

size_t mbuf_assign(mbuf_t *dest, const mbuf_t *src)
{
    if (dest == src)
	return 0;
    mbuf_clear(dest);
    return mbuf_push_back(dest, mbuf_ptr(src), mbuf_size(src));
}

size_t mbuf_append(mbuf_t *dest, const mbuf_t *src)
{
    if (dest == src)
	return 0;
    return mbuf_push_back(dest, mbuf_ptr(src), mbuf_size(src));
}

void mbuf_delete(mbuf_t *mp)
{
    mbuf_free(mp);
    free(mp);
}

int mbuf_reserve(mbuf_t *mp, size_t n)
{
    n = clp2(MAX(n, mp->cnt));
    if (n != mp->max) {
	void *p = (void*)realloc(mp->ptr, n);
	if (p == NULL)
	    return errno;
	mp->ptr = p;
	mp->max = n;
    }
    return 0;
}

int mbuf_resize(mbuf_t *mp, size_t n)
{
    void *p;
    n = clp2(n);
    if ((p = (void*)realloc(mp->ptr, n)) == NULL) {
	return errno;
    }
    mp->ptr = p;
    mp->max = n;
    mp->cnt = MIN(mp->cnt, n);
    return 0;
}

size_t mbuf_replace(mbuf_t *mp, size_t at, size_t m, const void *p, size_t n)
{
    assert(mp != NULL);
    errno = 0;
    if (mp == NULL) {
	errno = EINVAL;
	return 0;
    }
    /* if (n > 0 && p == NULL) holds, uninitialized bytes are inserted. */

    at = MIN(at, mp->cnt);
    m = MIN(m, mp->cnt - at);

    if (n != m) {
	if (n > m) {
	    const size_t incr = n - m;
	    if (incr >= mbuf_max_size() - mp->cnt) { 
		/* this avoids overflow of (mp->cnt + n) */
		errno = ENOMEM;
		return 0;
	    }
	    if (incr >= mp->max - mp->cnt) {
		if (mbuf_resize(mp, mp->cnt + incr) != 0)
		    return 0;
	    }
	    mp->cnt += incr;
	} else {    /* (n < m) */
	    mp->cnt -= (m - n);
	}
	memmove(mp->ptr + at + n, mp->ptr + at + m, mp->cnt - (at + m));
    }
    if (n > 0 && p != NULL)
	memcpy(mp->ptr + at, p, n);

    return n;
}

size_t mbuf_push_back(mbuf_t *mp, const void *p, size_t n)
{
    assert(p != NULL);
    errno = 0;
    if (p == NULL) {
	errno = EINVAL;
	return 0;
    }
    if (n > 0) {
	if (n >= mbuf_max_size() - mp->cnt) {
	    /* this avoids overflow of (mp->cnt + n) */
	    errno = ENOMEM;
	    return 0;
	}
	if (n > mp->max - mp->cnt) {
	    if (mbuf_resize(mp, mp->cnt + n) != 0)
		return 0;
	}
	memcpy(mp->ptr + mp->cnt, p, n);
	mp->cnt += n;
    }
    return n;
}

size_t mbuf_push_front(mbuf_t *mp, const void *p, size_t n)
{
    assert(p != NULL);
    errno = 0;
    if (p == NULL) {
	errno = EINVAL;
	return 0;
    }
    if (n > 0) {
	if (n >= mbuf_max_size() - mp->cnt) {
	    /* this avoids overflow of (mp->cnt + n) */
	    errno = ENOMEM;
	    return 0;
	}
	if (n > mp->max - mp->cnt) {
	    if (mbuf_resize(mp, mp->cnt + n) != 0)
		return 0;
	}
	memmove(mp->ptr + n, mp->ptr, mp->cnt);
	memcpy(mp->ptr, p, n);
	mp->cnt += n;
    }
    return n;
}

size_t mbuf_pop_back(mbuf_t *mp, size_t n)
{
    size_t m = MIN(mp->cnt, n);
    mp->cnt -= m;
    return m;
}

size_t mbuf_pop_front(mbuf_t *mp, size_t n)
{
    size_t m = MIN(mp->cnt, n);
    mp->cnt -= m;
    memmove(mp->ptr, mp->ptr + m, mp->cnt);
    return m;
}

/*----------------------------------------------------------------------
 * private functions
 *----------------------------------------------------------------------*/

/*
 * Least power of 2 greater than or equal to x
 * H. S. Warren, Jr., Hacker's Delight, pp.48.
 */
inline static size_t clp2(size_t x)
{
    --x;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
#if SIZEOF_SIZE_T >= 8
    x |= (x >> 32);
#endif
    return x + 1;
}
