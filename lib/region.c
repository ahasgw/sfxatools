/***********************************************************************
 * $Id: region.c,v 1.2 2005/08/01 11:24:37 aki Exp $
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

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#include "region.h"

#include "search.h"

#include <minmax.h>

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
    mbuf_delete(re->ranges);
    re->sa = NULL;
}

int region_search(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet)
{
    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	search32(re, pattern, patlen, opt_alphabet);
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	search64(re, pattern, patlen, opt_alphabet);
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}

int region_search_regexp(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet)
{
    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	search_regexp32(re, pattern, patlen, opt_alphabet);
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	search_regexp64(re, pattern, patlen, opt_alphabet);
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}

int region_print(const region_t *re,
	region_print32_f pf32, region_print64_f pf64, void *param)
{
    if (re == NULL || re->sa == NULL || re->ranges == NULL)
	return errno = EINVAL;

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	region_print32_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

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
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	region_print64_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

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
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}

int region_dump(const region_t *re,
	region_print32_f pf32, region_print64_f pf64, void *param)
{
    if (re == NULL || re->sa == NULL || re->ranges == NULL)
	return errno = EINVAL;

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_UNKNOWN)
	return errno = EINVAL;	    /* sa is not open */

    if (sfxa_idxbits(re->sa) == SFXA_IDXBITS_32) {
	region_print32_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

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
#if SIZEOF_OFF_T >= 8
    } else {	/* SFXA_IDXBITS_64 */
	region_print64_arg_t arg;
	arg.txt = sfxa_txt_ptr(re->sa);
	arg.idx = sfxa_idx_ptr(re->sa);
	arg.len = sfxa_txt_len(re->sa);
	arg.adj = 0;
	arg.param = param;

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
#endif /* SIZEOF_OFF_T >= 8 */
    }
    return 0;
}
