/***********************************************************************
 * $Id: search.c,v 1.8 2005/08/22 06:18:59 aki Exp $
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

#ifndef NDEBUG
# include <stdio.h>
#endif

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "search.h"

#include "regexp.h"
#include "u32stk.h"
#include "ptrstk.h"

#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define search_regexp_max_repeat    search_regexp_max_repeat
#endif

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define searchXX	    search32
# define search_regexpXX    search_regexp32
# define search_regexpXX_data_type  search_regexp32_data_type
# define search_regexpXX_data_t	search_regexp32_data_t
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
# define search_regexpXX_data_type  search_regexp64_data_type
# define search_regexpXX_data_t	search_regexp64_data_t
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

typedef enum head_tail_type {
    none,
    head,
    tail
} head_tail_t;

typedef struct search_regexpXX_data_type {
    regexp_code_t	*code;
    size_t		ip;
    ptrstk_t		regions;
    regexp_charbits_t	bits;
    regexp_charbits_t	alpha;
    mbuf_t		chs;
    searchXX_arg_t	arg;
} search_regexpXX_data_t;

/*======================================================================
 * prototypes
 *======================================================================*/

static int search_chs(search_regexpXX_data_t *d, size_t skip);
static int search_branch(search_regexpXX_data_t *d, size_t skip);
static int search_subexp(search_regexpXX_data_t *d, size_t skip);

static void data_init(search_regexpXX_data_t *d, regexp_code_t *code);
static int merge_top2_region(ptrstk_t *stk);
static void swap_pop_region(ptrstk_t *stk);
static int dup_n_push_region(ptrstk_t *stk, size_t skip);
static void build_charlist(search_regexpXX_data_t *d);
static void build_charlist_alpha(search_regexpXX_data_t *d);
#if 0
static void delete_top_region(ptrstk_t *stk);
static int dup_push_region(ptrstk_t *stk);
static uint32_t increment_counter(u32stk_t *stk);
#endif

static int bf_searchXX_1(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const char ch, head_tail_t ht);
static int bf_searchXX(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const mbuf_t *chs, head_tail_t ht);
static int search_interval(const char ch, searchXX_arg_t *arg, rangeXX_t *res);

#ifndef NDEBUG
static void code_print(FILE *s, size_t n, size_t level, regexp_code_t code);
static void print_ptrstk(FILE *s, const ptrstk_t *stk, const char *name);
#endif

/*======================================================================
 * function definitions
 *======================================================================*/

/* searchXX */
int searchXX(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet)
{
    int ret = 0;

    assert(re != NULL);
    if (re == NULL)
	return EINVAL;
    assert(re->ranges != NULL);
    if (re->ranges == NULL)
	return EINVAL;

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
			    &arg, ch, none)) != 0)
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
int search_regexpXX(region_t *re, const char *pattern, size_t patlen,
	const char *opt_alphabet, unsigned long repeat_max)
{
    int ret = 0;
    regexp_t rx;
    regexp_opt_t rx_opt;
    search_regexpXX_data_t data;

    assert(re != NULL);
    assert(re->ranges != NULL);
    if (re == NULL)
	return EINVAL;
    if (re->ranges == NULL)
	return EINVAL;

    /*
     * prepare
     */ 
    rx_opt.max_repeat = repeat_max;
    if ((ret = regexp_init(&rx, pattern, &rx_opt)) != 0) {
#if !defined(NDEBUG) && 0
	fprintf(stdout, "Error at '%c' (%u)\n",
		pattern[rx.error_at], rx.error_at + 1);
#endif
	goto bail0;
    }
    data_init(&data, rx.code);

    if ((ret = mbuf_init(&data.chs, NULL, 128)) != 0)
	goto bail1;
    
    if ((ret = ptrstk_init(&data.regions)) != 0)
	goto bail2;

    {	/* move the renges_in into stack */
	if (ptrstk_push_back(&data.regions, re->ranges) != 1) {
	    ret = errno;
	    goto bail3;
	}
	re->ranges = NULL;
    }

    /* setup args */
    data.arg.txt = (const char *)sfxa_txt_ptr(re->sa);
    data.arg.idx = (const intXX_t *)sfxa_idx_ptr(re->sa);
    data.arg.len = (intXX_t)sfxa_txt_len(re->sa);

    /* setup alphabet */
    if (opt_alphabet) {	    /* if alphabet is specified */
	const char *cp = opt_alphabet;
	regexp_charbits_clr(&data.alpha);
	for (; *cp; ++cp) {
	    regexp_charbits_set(&data.alpha, *cp);
	}
    } else {		    /* otherwise set default alphabet */
	regexp_charbits_sp_and_print(&data.alpha);
    }

#if !defined(NDEBUG) && 0
    regexp_stack_print(stdout, rx.code, rx.code_len);
#endif

#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &data.regions, "initial");
#endif

    /*
     * execute code
     */
    if (search_subexp(&data, 0)) {
	ret = errno;
	goto bail3;
    }

#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &data.regions, "regions final (should be 2)");
#endif

    /* copy the top of the ranges stack back to the region's ranges */
    re->ranges = ptrstk_back(&data.regions);
    ptrstk_pop_back(&data.regions);

    /* cleaning */
    mbuf_free(ptrstk_back(&data.regions));
    ptrstk_pop_back(&data.regions);

#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &data.regions, "regions final (should be 0)");
#endif

bail3:
    ptrstk_free(&data.regions);
bail2:
    mbuf_free(&data.chs);
bail1:
    regexp_free(&rx);
bail0:
    return ret;
}

#if SIZEOF_INTXX_T < 8
/* search_regexp_max_repeat */
unsigned int search_regexp_max_repeat(void)
{
    return (unsigned int)(REGEXP_MAX_REPEAT);
}
#endif

/*======================================================================
 * private function definitions
 *======================================================================*/

static int search_chs(search_regexpXX_data_t *d, size_t skip)
{
    regexp_code_t code = d->code[d->ip];
    regexp_code_kind_t kind = regexp_code_get_kind(code);
    unsigned int least = regexp_code_get_least(code);
    unsigned int most = regexp_code_get_most(code);
    unsigned int j;
    head_tail_t ht = none;
    char ch = '\0';

#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &d->regions, "chs() enter");
#endif
    /* prepare stack */
    if (!dup_n_push_region(&d->regions, skip))
	return errno;

    /* prepare search target */
    switch (kind) {
	case KIND_HEAD:	ht = head; break;
	case KIND_TAIL:	ht = tail; break;
	case KIND_CH:	ch = regexp_code_get_ch(code); break;
	case KIND_CHS:	build_charlist(d); break;
	case KIND_ANY:	build_charlist_alpha(d); break;
	default: break;
    }

    /* search */
    for (j = 1; j <= most; ++j) {
	int ret;
	mbuf_t *result;
	/* prepare search result holder */
	if ((result = mbuf_new(NULL, 0)) == NULL)
	    return errno;

	/* search: read from top of the stack, then write out to result */ 
	ret = (kind == KIND_CH || kind == KIND_HEAD || kind == KIND_TAIL)
	    ? bf_searchXX_1((mbuf_t*)ptrstk_back(&d->regions), result, &d->arg,
		    ch, ht)
	    : bf_searchXX((mbuf_t*)ptrstk_back(&d->regions), result, &d->arg,
		    &d->chs, none);
	if (ret != 0)
	    return errno;

	/* push the result to the stack */
	if (!ptrstk_push_back(&d->regions, result))
	    return errno;

	if (j <= least)
	    swap_pop_region(&d->regions);   /* swap and pop the stack */
    }

    /* merge */
    for (j = most; j > least; --j) {
	if (merge_top2_region(&d->regions))
	    return errno;
    }
#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &d->regions, "chs() leave");
#endif

    return 0;
}

static int search_branch(search_regexpXX_data_t *d, size_t skip)
{
#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &d->regions, "branch() enter");
#endif
    /* prepare stack */
    if (!dup_n_push_region(&d->regions, skip))
	return errno;

    for (;;) {
	int ret = 0;
	regexp_code_t code = d->code[d->ip];
	regexp_code_kind_t kind = regexp_code_get_kind(code);

	if (kind == KIND_OR || kind == KIND_UP)
	    break;

#if !defined(NDEBUG) && 0
printf("branch()\t%u\t%p\t", d->ip, d->code + d->ip);
regexp_code_print(stdout, code); putchar('\n');
#endif

	ret = (kind == KIND_DOWN)
	    ? search_subexp(d, 0) : search_chs(d, 0);
	if (ret != 0)
	    return errno;

	swap_pop_region(&d->regions);   /* swap and pop the stack */
	++(d->ip);
    }
#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &d->regions, "branch() leave");
#endif

    return 0;
}

static int search_subexp(search_regexpXX_data_t *d, size_t skip)
{
    regexp_code_t code = d->code[d->ip];
    unsigned int least = regexp_code_get_least(code);
    unsigned int most = regexp_code_get_most(code);
    unsigned int ip_goback = 0;
    unsigned int j;

#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &d->regions, "subexp() enter");
#endif
    /* prepare stack */
    if (!dup_n_push_region(&d->regions, skip))
	return errno;

    /* search */
    for (j = 1; j <= most; ++j) {
	size_t k = 0;
	regexp_code_kind_t kind;

	d->ip -= ip_goback;

	/* search branches */
	do {
	    ++(d->ip);
	    code = d->code[d->ip];

#if !defined(NDEBUG) && 0
printf("subexp()\t%u\t%p\t", d->ip, d->code + d->ip);
regexp_code_print(stdout, code); putchar('\n');
#endif
	    if (search_branch(d, k))	/* search_branch advances d->ip */
		return errno;		/* could be memory leak */
	    code = d->code[d->ip];

#if !defined(NDEBUG) && 0
printf("->subexp()\t%u\t%p\t", d->ip, d->code + d->ip);
regexp_code_print(stdout, code); putchar('\n');
#endif
	    ++k;

	    kind = regexp_code_get_kind(code);
	} while (kind != KIND_UP);

	for (--k; k > 0; --k) {
	    if (merge_top2_region(&d->regions))
		return errno;
	}

	if (j <= least)
	    swap_pop_region(&d->regions);   /* swap and pop the stack */

	/* to the next code */
	ip_goback = regexp_code_get_least(code);
    }

    /* merge */
    for (j = most; j > least; --j) {
	if (merge_top2_region(&d->regions))
	    return errno;
    }
#if !defined(NDEBUG) && 0
print_ptrstk(stdout, &d->regions, "subexp() leave");
#endif

    return 0;
}

/*----------------------------------------------------------------------*/

/* initialize data type */
static void data_init(search_regexpXX_data_t *d, regexp_code_t *code)
{
    d->code = code;
    d->ip = 0;
}

/* merge top two of the stack */
static int merge_top2_region(ptrstk_t *stk)
{
    int ret = 0;
    mbuf_t *r_tmp = (mbuf_t*)ptrstk_back(stk);
    ptrstk_pop_back(stk);
    if (mbuf_append(ptrstk_back(stk), r_tmp) != mbuf_size(r_tmp))
	ret = errno;
    mbuf_free(r_tmp);
    return ret;
}

/* swap and pop */
static void swap_pop_region(ptrstk_t *stk)
{
    mbuf_t *r_tmp = (mbuf_t*)ptrstk_back(stk);
    ptrstk_pop_back(stk);
    mbuf_free(ptrstk_back(stk));
    ptrstk_pop_back(stk);
    ptrstk_push_back(stk, r_tmp);
}

/* duplicate (top - skip)-th element of the stack then push */
static int dup_n_push_region(ptrstk_t *stk, size_t skip)
{
    mbuf_t *r = (mbuf_t*)ptrstk_at(stk, ptrstk_back_idx(stk) - skip);
    mbuf_t *r_dup = mbuf_dup(r);
    return (r_dup == NULL) ? 0 : ptrstk_push_back(stk, r_dup);
}

/* build character list */
static void build_charlist(search_regexpXX_data_t *d)
{
    unsigned int k;
    regexp_code_t code = d->code[d->ip];
    regexp_charbits_read(d->code, &d->ip, &d->bits);
    if (regexp_code_get_ch(code) == '!')
	regexp_charbits_not(&d->bits);
    regexp_charbits_and(&d->bits, &d->alpha);
    mbuf_clear(&d->chs);
    for (k = 0; k < CHAR_MAX; ++k) {
	if (regexp_charbits_test(&d->bits, (char)k))
	    mbuf_push_back_1(&d->chs, (char)k);
    }
}

/* build character list of alphabet */
static void build_charlist_alpha(search_regexpXX_data_t *d)
{
    unsigned int k;
    mbuf_clear(&d->chs);
    for (k = 0; k < CHAR_MAX; ++k) {
	if (regexp_charbits_test(&d->alpha, (char)k))
	    mbuf_push_back_1(&d->chs, (char)k);
    }
}

#if 0
/* remove the top of regions stack */
static void delete_top_region(ptrstk_t *stk)
{
    mbuf_free((mbuf_t*)ptrstk_back(stk));
    ptrstk_pop_back(stk);
}

/* duplicate stack top then push */
static int dup_push_region(ptrstk_t *stk)
{
    mbuf_t *r_dup = mbuf_dup((mbuf_t*)ptrstk_back(stk));
    return ptrstk_push_back(stk, r_dup);
}

/* increment counter */
static uint32_t increment_counter(u32stk_t *stk)
{
    uint32_t old = u32stk_back(stk);
    u32stk_pop_back(stk);
    u32stk_push_back(stk, old + 1);
    return old;
}
#endif

/*----------------------------------------------------------------------*/

static int bf_searchXX_1(const mbuf_t *ranges_in, mbuf_t *ranges_out,
	void *arg, const char ch, head_tail_t ht)
{
    const rangeXX_t *r_beg = (const rangeXX_t*)mbuf_ptr(ranges_in);
    const rangeXX_t *r_end = r_beg + (mbuf_size(ranges_in) / sizeof(rangeXX_t));
    const rangeXX_t *rp = NULL;

    for (rp = r_beg; rp < r_end; ++rp) {
	rangeXX_t r = *rp;
	if (search_interval(ch, (searchXX_arg_t *)arg, &r)) {
	    r.head |= (ht == head);
	    r.tail |= (ht == tail);
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
	/* search from small end */
	/* search from both end might be better */
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
	/* search from both end */
	for (;;) {
	    if (!(cp_beg <= cp_end))
		break;
	    /* search from small end */
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
	    /* search from large end */
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

/*----------------------------------------------------------------------*/
#ifndef NDEBUG

static void code_print(FILE *s, size_t n, size_t level, regexp_code_t code)
{
    int i;
    regexp_code_kind_t kind = regexp_code_get_kind(code);
    printf("%d\t", n);
    for (i = 0; i < (kind == KIND_UP ? level - 1 : level); ++i)
	putchar(' ');
    regexp_code_print(stdout, code); putchar('\n');
}

static void print_ptrstk(FILE *s, const ptrstk_t *stk, const char *name)
{
    int i;
    fprintf(s, "## %s, size: %u ##\n", name, ptrstk_size(stk));
    for (i = 0 ; i < ptrstk_size(stk); ++i)
	printf("[[ %p ]]\t%u\n", ptrstk_at(stk, i),
		mbuf_size(ptrstk_at(stk, i)) / sizeof(rangeXX_t));
}

#endif
