/***********************************************************************
 * $Id: region.c,v 1.6 2006/04/13 11:03:03 aki Exp $
 *
 * region
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

#if 1
#  include <stdio.h>
#endif

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#include "region.h"

#include "search.h"

#include <minmax.h>

/*======================================================================
 * prototypes
 *======================================================================*/

static int range32_compar(const void *r1, const void *r2);
static int range64_compar(const void *r1, const void *r2);

/*======================================================================
 * public function definitions
 *======================================================================*/

int region_init(region_t *re, const sfxa_t *sa)
{
    assert(re != NULL);
    assert(sa != NULL);
    if (re == NULL || sa == NULL || sfxa_idxbits(sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;
    re->sa = sa;
    if (sfxa_idxbits(sa) == SFXA_IDXBITS_32) {
	range32_t r = {0, (int32_t)sfxa_txt_len(sa) - 1, 0, 0, 0};
	if ((re->ranges = mbuf_new(&r, sizeof r)) == NULL)
	    return errno;
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	range64_t r = {0, (int64_t)sfxa_txt_len(sa) - 1, 0, 0, 0};
	if ((re->ranges = mbuf_new(&r, sizeof r)) == NULL)
	    return errno;
#endif /* SIZEOF_OFF_T >= 8 */
    }
    if (re->ranges == NULL)
	return errno;

    return 0;
}

void region_free(region_t *re)
{
    assert(re != NULL);
    if (re->ranges)
	mbuf_delete(re->ranges);
    re->sa = NULL;
}

int region_search(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet)
{
    int ret = 0;

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	ret = search32(re, pattern, patlen, opt_alphabet);
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	ret = search64(re, pattern, patlen, opt_alphabet);
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return ret;
}

int region_search_regexp(region_t *re, const regexp_t *rx, const char *opt_alphabet, unsigned long rep_max)
{
    int ret = 0;

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	ret = search_regexp32(re, rx, opt_alphabet, rep_max);
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	ret = search_regexp64(re, rx, opt_alphabet, rep_max);
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return ret;
}

int region_narrow_down(region_t *re)
{
    if (re == NULL || re->sa == NULL || re->ranges == NULL)
	return errno = EINVAL;
    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */
    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	size_t range_cnt = mbuf_size(re->ranges) / sizeof(range32_t);
	if (range_cnt > 1) {
	    range32_t *rp_beg = mbuf_ptr(re->ranges);
	    range32_t *rp_end = rp_beg + range_cnt;
	    range32_t *rp0, *rp;

	    /* sort original */
	    qsort(mbuf_ptr(re->ranges), range_cnt, sizeof(range32_t), &range32_compar);
	    /* merge ranges */
	    for (rp0 = rp_beg, rp = rp0 + 1; rp < rp_end; ++rp) {
		if (rp0->end < rp->end) {
		    if (rp->beg <= rp0->end + 1) {
			rp0->end = rp->end;
		    } else {
			*++rp0 = *rp;
		    }
		}
	    }
	    /* shrink wrap */
	    mbuf_resize(re->ranges, (rp0 - rp_beg + 1) * sizeof(range32_t));
	}
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	size_t range_cnt = mbuf_size(re->ranges) / sizeof(range64_t);
	if (range_cnt > 1) {
	    range64_t *rp_beg = mbuf_ptr(re->ranges);
	    range64_t *rp_end = rp_beg + range_cnt;
	    range64_t *rp0, *rp;

	    /* sort original */
	    qsort(mbuf_ptr(re->ranges), range_cnt, sizeof(range64_t), &range64_compar);
	    /* merge ranges */
	    for (rp0 = rp_beg, rp = rp0 + 1; rp < rp_end; ++rp) {
		if (rp0->end < rp->end) {
		    if (rp->beg <= rp0->end + 1) {
			rp0->end = rp->end;
		    } else {
			*++rp0 = *rp;
		    }
		}
	    }
	    /* shrink wrap */
	    mbuf_resize(re->ranges, (rp0 - rp_beg + 1) * sizeof(range64_t));
	}
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}

static int range32_compar(const void *r1, const void *r2)
{
    int32_t d1 = (((range32_t*)r1)->beg - ((range32_t*)r2)->beg);
    int32_t d2 = (((range32_t*)r2)->end - ((range32_t*)r1)->end);
    return (d1 == 0) ? d2 : d1;

}
#if SIZEOF_OFF_T >= 8
static int range64_compar(const void *r1, const void *r2)
{
    int64_t d1 = (((range64_t*)r1)->beg - ((range64_t*)r2)->beg);
    int64_t d2 = (((range64_t*)r2)->end - ((range64_t*)r1)->end);
    if (d1 != 0)
	return (d1 > 0) ? 1 : -1;
    else if (d2 != 0)
	return (d2 > 0) ? 1 : -1;
    return 0;
}
#endif /* SIZEOF_OFF_T >= 8 */

size_t region_count(const region_t *re)
{
    size_t num = 0;

    if (re == NULL || re->sa == NULL || re->ranges == NULL)
	return errno = EINVAL, 0;
    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL, 0;    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	size_t range_cnt = mbuf_size(re->ranges) / sizeof(range32_t);
	const range32_t *rp_beg = mbuf_ptr(re->ranges);
	const range32_t *rp_end = rp_beg + range_cnt;
	const range32_t *rp;

	for (rp = rp_beg; rp < rp_end; ++rp) {
	    num += rp->end - rp->beg + 1;
	}
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	size_t range_cnt = mbuf_size(re->ranges) / sizeof(range64_t);
	const range64_t *rp_beg = mbuf_ptr(re->ranges);
	const range64_t *rp_end = rp_beg + range_cnt;
	const range64_t *rp;

	for (rp = rp_beg; rp < rp_end; ++rp) {
	    num += rp->end - rp->beg + 1;
	}
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return num;
}

int region_print(const region_t *re, void *param)
{
    if (re == NULL || re->sa == NULL || re->ranges == NULL)
	return errno = EINVAL;

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	output32_f pf32 = output32;
	output32_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

	{
	    mbuf_t *mbp = re->ranges;
	    const range32_t *r = (const range32_t *)mbuf_ptr(mbp);
	    const range32_t *r_end = r + mbuf_size(mbp) / sizeof(range32_t);
	    for (; r < r_end; ++r)
	    {
		int32_t pos;
#if 0
		for (pos = r->beg; pos <= r->end; ++pos) {
		    (*pf32)(pos, &arg);
		}
#else
		if (r->head) {
		    arg.adj = 1;
		    for (pos = r->beg; pos <= r->end; ++pos) {
			if (pos > 0) {
			    (*pf32)(pos, &arg);
			}
		    }
		} else if (r->tail) {
		    arg.adj = 0;
		    for (pos = r->beg; pos <= r->end; ++pos) {
			if (arg.idx[pos] > 0) {
			    (*pf32)(pos, &arg);
			}
		    }
		} else {
		    arg.adj = 0;
		    for (pos = r->beg; pos <= r->end; ++pos) {
			(*pf32)(pos, &arg);
		    }
		}
#endif
	    }
	}
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	output64_f pf64 = output64;
	output64_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

	{
	    mbuf_t *mbp = re->ranges;
	    const range64_t *r = (const range64_t *)mbuf_ptr(mbp);
	    const range64_t *r_end = r + mbuf_size(mbp) / sizeof(range64_t);
	    for (; r < r_end; ++r)
	    {
		int64_t pos;
#if 0
		for (pos = r->beg; pos <= r->end; ++pos) {
		    (*pf64)(pos, &arg);
		}
#else
		if (r->head) {
		    arg.adj = 1;
		    for (pos = r->beg; pos <= r->end; ++pos) {
			if (pos > 0) {
			    (*pf64)(pos, &arg);
			}
		    }
		} else if (r->tail) {
		    arg.adj = 0;
		    for (pos = r->beg; pos <= r->end; ++pos) {
			if (arg.idx[pos] > 0) {
			    (*pf64)(pos, &arg);
			}
		    }
		} else {
		    arg.adj = 0;
		    for (pos = r->beg; pos <= r->end; ++pos) {
			(*pf64)(pos, &arg);
		    }
		}
#endif
	    }
	}
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}

int region_dump(const region_t *re, void *param)
{
    if (re == NULL || re->sa == NULL || re->ranges == NULL)
	return errno = EINVAL;

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	output32_f pf32 = output32;
	output32_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

	{
	    mbuf_t *mbp = re->ranges;
	    const range32_t *r = (const range32_t *)mbuf_ptr(mbp);
	    const range32_t *r_end = r + mbuf_size(mbp) / sizeof(range32_t);
	    for (; r < r_end; ++r)
	    {
		int32_t pos;
		for (pos = r->beg; pos <= r->end; ++pos) {
		    (*pf32)(pos, &arg);
		}
	    }
	}
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	output64_f pf64 = output64;
	output64_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

	{
	    mbuf_t *mbp = re->ranges;
	    const range64_t *r = (const range64_t *)mbuf_ptr(mbp);
	    const range64_t *r_end = r + mbuf_size(mbp) / sizeof(range64_t);
	    for (; r < r_end; ++r)
	    {
		int64_t pos;
		for (pos = r->beg; pos <= r->end; ++pos) {
		    (*pf64)(pos, &arg);
		}
	    }
	}
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}
