/***********************************************************************
 * $Id: search.c,v 1.1.1.1 2005/02/02 10:39:30 aki Exp $
 *
 * search
 * Copyright (C) 2005 RIKEN. All rights reserved.
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

#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#endif

#ifndef SEARCH_H
# include "search.h"
#endif
#ifndef RANGE_H
# include "range.h"
#endif

#include <mmfile.h>
#include <msg.h>

#include <assert.h>
#include <errno.h>
#include <minmax.h>
#include <stdint.h>
#include <string.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define searchXX	    search32
# define searchXX_arg_type  search32_arg_type
# define searchXX_arg_t	    search32_arg_t
# define rangeXX_t	    range32_t
#else
# define intXX_t	    int64_t
# define searchXX	    search64
# define searchXX_arg_type  search64_arg_type
# define searchXX_arg_t	    search64_arg_t
# define rangeXX_t	    range64_t
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct searchXX_arg_type {
    const char *restrict    txt;
    const intXX_t *restrict idx;
    const intXX_t	    len;

    const char		    *pattern;
    const intXX_t	    pat_len;
} searchXX_arg_t;

/*======================================================================
 * prototypes
 *======================================================================*/

inline static intXX_t lcp_len(const char *txt, const char *end, const char *pat);
static intXX_t bsearch_beg(searchXX_arg_t *arg);
static intXX_t bsearch_end(searchXX_arg_t *arg);

/*======================================================================
 * function definitions
 *======================================================================*/

/* searchXX */
int searchXX(const char *restrict txt, const intXX_t *restrict idx,
	const intXX_t len, const char *pattern, rangeXX_t *result)
{
    searchXX_arg_t arg = {txt, idx, len, pattern, strlen(pattern)};

    assert(result != NULL);
    result->beg = bsearch_beg(&arg);
    result->end = bsearch_end(&arg);
    return 0;
}

/*======================================================================
 * private function definitions
 *======================================================================*/

inline static intXX_t lcp_len(const char *txt, const char *end, const char *pat)
{
    intXX_t len;
    for (len = 0; *pat && txt < end && *pat == *txt; ++len) {
	++pat;
	++txt;
    }
    return len;
}

/* to accelerate, see Gusfield pp.152-153 */

static intXX_t bsearch_beg(searchXX_arg_t *arg)
{
    const char *txt = arg->txt;
    const intXX_t *idx = arg->idx;
    const char *pat = arg->pattern;
    const size_t pat_len = arg->pat_len;
    const char *txt_end = txt + arg->len;
    const char *cp;

    intXX_t beg = 0;
    intXX_t end = arg->len - 1;
    while (beg < end) {
	intXX_t mid = (beg + end) / 2;
	cp = txt + idx[mid];
	if (strncmp(cp, pat, MIN((size_t)(txt_end - cp), pat_len)) < 0) {
	    beg = mid + 1;
	} else {
	    end = mid;
	}
    }
    cp = txt + idx[beg];
    return (strncmp(cp, pat, MIN((size_t)(txt_end - cp), pat_len)) == 0)
	? beg : -1;
}

static intXX_t bsearch_end(searchXX_arg_t *arg)
{
    const char *txt = arg->txt;
    const intXX_t *idx = arg->idx;
    const char *pat = arg->pattern;
    const size_t pat_len = arg->pat_len;
    const char *txt_end = txt + arg->len;
    const char *cp;

    intXX_t beg = 0;
    intXX_t end = arg->len - 1;
    while (beg < end) {
	intXX_t mid = (beg + end + 1) / 2;
	cp = txt + idx[mid];
	if (strncmp(cp, pat, MIN((size_t)(txt_end - cp), pat_len)) > 0) {
	    end = mid - 1;
	} else {
	    beg = mid;
	}
    }
    cp = txt + idx[beg];
    return (strncmp(cp, pat, MIN((size_t)(txt_end - cp), pat_len)) == 0)
	? beg : -1;
}
