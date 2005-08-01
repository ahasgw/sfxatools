/***********************************************************************
 * $Id: search.c,v 1.2 2005/08/01 11:24:37 aki Exp $
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
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "search.h"
#include "search_impl.h"

#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define searchXX	    search32
# define search_regexpXX    search_regexp32
# define searchXX_arg_type  search32_arg_type
# define searchXX_arg_t	    search32_arg_t
# define rangeXX_t	    range32_t
# define bf_searchXX_1	    bf_search32_1
# define bf_searchXX	    bf_search32
# define rXX		    r32
#else
# define intXX_t	    int64_t
# define searchXX	    search64
# define search_regexpXX    search_regexp64
# define searchXX_arg_type  search64_arg_type
# define searchXX_arg_t	    search64_arg_t
# define rangeXX_t	    range64_t
# define bf_searchXX_1	    bf_search64_1
# define bf_searchXX	    bf_search64
# define rXX		    r64
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct searchXX_arg_type {
    const char	    *txt;
    const intXX_t   *idx;
    intXX_t	    len;
} searchXX_arg_t;

/*======================================================================
 * prototypes
 *======================================================================*/

extern int yyparse(void *arg);

static int bf_searchXX_1(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const char ch);
static int bf_searchXX(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const mbuf_t *chs, head_tail_t ht);

static int search_interval(const char ch, searchXX_arg_t *arg, rangeXX_t *res);

/*======================================================================
 * function definitions
 *======================================================================*/

/* searchXX */
int searchXX(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet)
{
    int ret = 0;

    assert(re != NULL);
#if 0
printf("# %*s\n", patlen, pattern);
#endif

    if (patlen > 0) {
	searchXX_arg_t arg;
	/* setup args */
	arg.txt = (const char *)sfxa_txt_ptr(re->sa);
	arg.idx = (const intXX_t *)sfxa_idx_ptr(re->sa);
	arg.len = (intXX_t)sfxa_txt_len(re->sa);

	mbuf_t  ranges_tmp;
	size_t ofst;

	/* initialize mbuf */
	if ((ret = mbuf_init(&ranges_tmp, NULL, 0)) != 0)
	    return ret;

	for (ofst = 0; ofst < patlen; ++ofst)
	{
	    const char ch = *(pattern + ofst);

	    /* break if (re->ranges) is empty */
	    if (mbuf_empty(re->ranges))
		break;

	    /* clear */
	    mbuf_clear(&ranges_tmp);
	    /* search */
	    if ((ret = bf_searchXX_1(re->ranges, &ranges_tmp,
			    &arg, ch)) != 0)
	    {
		break;
	    }
	    /* copy back */
	    if (mbuf_assign(re->ranges, &ranges_tmp)
		    != mbuf_size(&ranges_tmp))
	    {
		ret = errno;
		break;
	    }
	}

	/* finalize mbuf */
	mbuf_free(&ranges_tmp);
    }

    return ret;
}

/* search_regexpXX */
int search_regexpXX(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet)
{
    assert(re != NULL);
#if 0
printf("# %*s\n", patlen, pattern);
#endif

    if (patlen > 0) {
	searchXX_arg_t arg;
	parser_arg_t parser_arg;

	/* setup args */
	arg.txt = (const char *)sfxa_txt_ptr(re->sa);
	arg.idx = (const intXX_t *)sfxa_idx_ptr(re->sa);
	arg.len = (intXX_t)sfxa_txt_len(re->sa);

	/* setup parser arg */
	parser_arg.ranges_in = re->ranges;
	parser_arg.ranges_out = NULL;
	parser_arg.chs = mbuf_new(NULL, 256);
	parser_arg.alphabet = opt_alphabet;
	parser_arg.search_arg = (void*)&arg;
	parser_arg.search_func = bf_searchXX;
	parser_arg.repeat_max = ULONG_MAX;
	parser_arg.ptr = pattern;
	parser_arg.state = 0;
	if (parser_arg.chs == NULL)
	    return errno;

	if (yyparse((void*)&parser_arg) != 0)
	    return 1;

	mbuf_free(re->ranges);
	re->ranges = parser_arg.ranges_out;

	mbuf_delete(parser_arg.chs);
    }

    return 0;
}

/*======================================================================
 * private function definitions
 *======================================================================*/

static int bf_searchXX_1(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const char ch)
{
    const rangeXX_t *r_beg = (const rangeXX_t*)mbuf_ptr(ranges_in);
    const rangeXX_t *r_end = r_beg + (mbuf_size(ranges_in) / sizeof(rangeXX_t));
    const rangeXX_t *rp = NULL;

    for (rp = r_beg; rp < r_end; ++rp) {
	rangeXX_t r = *rp;
	if (search_interval(ch, (searchXX_arg_t *)arg, &r)) {
	    if (mbuf_push_back(ranges_out, &r, sizeof r) != sizeof r)
		return errno;
	}
    }
    return 0;
}

static int bf_searchXX(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const mbuf_t *chs, head_tail_t ht)
{
#if 0
    const rangeXX_t *r_beg = (const rangeXX_t*)mbuf_ptr(ranges_in);
    const rangeXX_t *r_end = r_beg + (mbuf_size(ranges_in) / sizeof(rangeXX_t));
    const rangeXX_t *rp = NULL;

    for (rp = r_beg; rp < r_end; ++rp) {
	intXX_t next_beg = rp->beg;
	const char *cp = (const char *)mbuf_ptr(chs);
	const char *cp_end = cp + mbuf_size(chs);
	/* searching from small end */
	/* searching from both end might be better */
	for (; cp < cp_end; ++cp) {
	    rangeXX_t r = *rp;
	    r.beg = next_beg;
	    if (search_interval(*cp, (searchXX_arg_t *)arg, &r)) {
		r.head |= (ht == head);
		r.tail |= (ht == tail);
		if (mbuf_push_back(ranges_out, &r, sizeof r) != sizeof r)
		    return errno;
		next_beg = r.end + 1;
	    }
	}
    }
    return 0;
#else
    const rangeXX_t *r_beg = (const rangeXX_t*)mbuf_ptr(ranges_in);
    const rangeXX_t *r_end = r_beg + (mbuf_size(ranges_in) / sizeof(rangeXX_t));
    const rangeXX_t *rp = NULL;

    for (rp = r_beg; rp < r_end; ++rp) {
	intXX_t next_beg = rp->beg;
	intXX_t next_end = rp->end;
	const char *cp_beg = (const char *)mbuf_ptr(chs);
	const char *cp_end = cp_beg + mbuf_size(chs) - 1;
	rangeXX_t r = *rp;
	/* searching from both end */
	for (;;) {
	    if (!(cp_beg <= cp_end))
		break;
	    /* searching from small end */
	    {
		r.ofst = rp->ofst;
		r.beg = next_beg;
		r.end = next_end;
		if (search_interval(*cp_beg, (searchXX_arg_t *)arg, &r)) {
		    r.head |= (ht == head);
		    r.tail |= (ht == tail);
		    if (mbuf_push_back(ranges_out, &r, sizeof r) != sizeof r)
			return errno;
		    next_beg = r.end + 1;
		}
		++cp_beg;
	    }
	    if (!(cp_beg <= cp_end))
		break;
	    /* searching from large end */
	    {
		r.ofst = rp->ofst;
		r.beg = next_beg;
		r.end = next_end;
		if (search_interval(*cp_end, (searchXX_arg_t *)arg, &r)) {
		    r.head |= (ht == head);
		    r.tail |= (ht == tail);
		    if (mbuf_push_back(ranges_out, &r, sizeof r) != sizeof r)
			return errno;
		    next_end = r.beg - 1;
		}
		--cp_end;
	    }
	}
    }
    return 0;
#endif
}

static int search_interval(const char ch, searchXX_arg_t *arg, rangeXX_t *range_io)
{
    if (range_io->beg <= range_io->end) {
	const char *txt = arg->txt + range_io->ofst;
	const intXX_t *idx = arg->idx;

	if (range_io->beg == range_io->end) {
	    if (*(txt + idx[range_io->beg]) != ch) {
		range_io->beg = -1;
		range_io->end = -2;
	    }
	} else {
	    intXX_t beg0 = range_io->beg;
	    intXX_t end0 = range_io->end;

	    /* these blocks can be parallel sections */

	    {
		intXX_t beg = beg0;
		intXX_t end = end0;
		while (beg < end) {
		    intXX_t mid = (beg + end) / 2;
		    if (*(txt + idx[mid]) < ch) {
			beg = mid + 1;
		    } else {
			end = mid;
		    }
		}
		range_io->beg = (*(txt + idx[beg]) == ch) ? beg : -1;
	    }

	    {
		intXX_t beg = beg0;
		intXX_t end = end0;
		while (beg < end) {
		    intXX_t mid = (beg + end + 1) / 2;
		    if (*(txt + idx[mid]) > ch) {
			end = mid - 1;
		    } else {
			beg = mid;
		    }
		}
		range_io->end = (*(txt + idx[beg]) == ch) ? beg : -2;
	    }

	}
	range_io->ofst += 1;
    }
    return (range_io->beg <= range_io->end);
}
