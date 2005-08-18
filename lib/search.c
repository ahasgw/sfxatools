/***********************************************************************
 * $Id: search.c,v 1.5 2005/08/18 11:20:36 aki Exp $
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

typedef enum head_tail_type {
    none,
    head,
    tail
} head_tail_t;

/*======================================================================
 * prototypes
 *======================================================================*/

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
    size_t i, level = 0;
    searchXX_arg_t arg;
    regexp_charbits_t alph;
    regexp_charbits_t bits;
    regexp_t rx;
    regexp_opt_t rx_opt;
    mbuf_t chs;
    u32stk_t counters;
    ptrstk_t regions;
    size_t adjust = 0;	/* 1: if preceded code is KIND_OR, otherwise 0 */

    assert(re != NULL);
    if (re == NULL)
	return EINVAL;
    assert(re->ranges != NULL);
    if (re->ranges == NULL)
	return EINVAL;

    /*
     * prepare
     */ 
    rx_opt.max_repeat = repeat_max;
    if ((ret = regexp_init(&rx, pattern, &rx_opt)) != 0) {  /* error */
#if !defined(NDEBUG) && 0
	fprintf(stdout, "Error at '%c' (%u)\n",
		pattern[rx.error_at], rx.error_at + 1);
#endif
	goto bail0;
    }

    if ((ret = mbuf_init(&chs, NULL, 128)) != 0)
	goto bail1;
    
    if ((ret = u32stk_init(&counters)) != 0)	/* error */
	goto bail2;

    if ((ret = ptrstk_init(&regions)) != 0)	/* error */
	goto bail3;

    {	/* move the renges_in into stack */
	if ((ret = ptrstk_push_back(&regions, re->ranges)) != 1) {
	    ret = errno;
	    goto bail4;
	}
	re->ranges = NULL;
    }

    /* setup args */
    arg.txt = (const char *)sfxa_txt_ptr(re->sa);
    arg.idx = (const intXX_t *)sfxa_idx_ptr(re->sa);
    arg.len = (intXX_t)sfxa_txt_len(re->sa);

    /* setup alphabet */
    if (opt_alphabet) {	    /* if alphabet is specified */
	const char *cp = opt_alphabet;
	regexp_charbits_clr(&alph);
	for (; *cp; ++cp) {
	    regexp_charbits_set(&alph, *cp);
	}
    } else {		    /* otherwise set default alphabet */
	regexp_charbits_sp_and_print(&alph);
    }

#if !defined(NDEBUG) && 0
    regexp_stack_print(stdout, rx.code, rx.code_len);
#endif

    /*
     * execute code
     */
    for (i = 0; i < rx.code_len; ++i) {
	regexp_code_t code = rx.code[i];
	regexp_code_kind_t kind = regexp_code_get_kind(code);
	mbuf_t *region_ref = NULL;
	unsigned int j;
	unsigned int least = regexp_code_get_least(code);
	unsigned int most = regexp_code_get_most(code);
	char ch;

#if !defined(NDEBUG) && 0
	code_print(stdout, i, level, code);
#endif

	switch (kind) {
	    case KIND_DOWN:
		if (regexp_code_get_dj(code)) {
		    ptrstk_push_back(&regions,
			    mbuf_dup(ptrstk_back(&regions)));
		}

		/* push new counter */
		u32stk_push_back(&counters, 1);

		/* duplicate the stack top and push if least == 0 */
		if (least == 0) {
		    ptrstk_push_back(&regions,
			    mbuf_dup(ptrstk_back(&regions)));
		}

#if !defined(NDEBUG) && 0
		print_ptrstk(stdout, &regions, "regions");
#endif
		++level;
		break;
	    case KIND_UP:
		{
		    size_t counters_top = u32stk_size(&counters) - 1;
		    unsigned int goback = least;
		    unsigned int count, count_least, count_most;
		    count_least = regexp_code_get_least(rx.code[i - least]);
		    count_most = regexp_code_get_most(rx.code[i - least]);
		    count = u32stk_at(&counters, counters_top);

#if !defined(NDEBUG) && 0
		    printf("-- c:%u, cl:%u, cm:%u, goback:%u, i:%u->%u --\n",
			    count, count_least, count_most, goback, i,
			    i - goback);
#endif

		    if (regexp_code_get_dj(code)) {
			/* swap and pop */
			mbuf_t *ranges_top = ptrstk_back(&regions);
			ptrstk_pop_back(&regions);
			mbuf_free(ptrstk_back(&regions));
			ptrstk_pop_back(&regions);
			ptrstk_push_back(&regions, ranges_top);
		    }

		    /* 1 .. */
		    if (count < count_most) {
			if (count >= count_least) {
			    ptrstk_push_back(&regions,
				    mbuf_dup(ptrstk_back(&regions)));
			}
			if (regexp_code_get_dj(code)) {
			    ptrstk_push_back(&regions,
				    mbuf_dup(ptrstk_back(&regions)));
			}
			u32stk_replace(&counters, counters_top, ++count);
			i -= goback;
		    } else {
			for (; count > count_least; --count) {
			    /* merge top two of the stack */
			    mbuf_t *ranges_top = ptrstk_back(&regions);
			    ptrstk_pop_back(&regions);
			    mbuf_append(ptrstk_back(&regions), ranges_top);
			    mbuf_free(ranges_top);
			}
			u32stk_pop_back(&counters);
			--level;
		    }
#if !defined(NDEBUG) && 0
		    print_ptrstk(stdout, &regions, "regions");
#endif
		}
		break;
	    case KIND_OR:
		break;
	    case KIND_HEAD:
	    case KIND_TAIL:
		{
		    /* prepare output */
		    mbuf_t *ranges_out = mbuf_new(NULL, 0);
		    if  (ranges_out == NULL) {
			ret = errno;
			goto bail4;
		    }
		    /* search.
		     * read from top of regions stack. write to regions_out
		     */ 
		    if ((ret = bf_searchXX_1(
				    ptrstk_at(&regions,
					ptrstk_size(&regions) - 1 - adjust),
				    ranges_out, &arg, '\0',
				    (kind == KIND_HEAD ? head : tail))) != 0)
		    {
			goto bail4;
		    }
		    if (adjust == 0) {	/* not just after the KIND_OR */
			/* remove the top of regions stack */
			mbuf_free(ptrstk_back(&regions));
			ptrstk_pop_back(&regions);
			/* push the regions_out to the top of regions stack */
			ptrstk_push_back(&regions, ranges_out);
		    } else {
			/* merge */
			mbuf_append(ptrstk_back(&regions), ranges_out);
		    }
		}
		break;
	    case KIND_CH:
		ch = regexp_code_get_ch(code);
		region_ref = ptrstk_at(&regions, ptrstk_size(&regions) - 1 - adjust);
		for (j = 1; j <= most; ++j) {
#if !defined(NDEBUG) && 0
		    printf("%d: [%u] %p\n", j, ptrstk_size(&regions), region_ref);
#endif
		    /* prepare output */
		    mbuf_t *ranges_out = mbuf_new(NULL, 0);
		    if  (ranges_out == NULL) {
			ret = errno;
			goto bail4;
		    }
		    /* search.
		     * read from top of regions stack. write to regions_out
		     */ 
		    if ((ret = bf_searchXX_1(region_ref,
				    ranges_out, &arg, ch, none)) != 0)
		    {
			goto bail4;
		    }
		    /* */
		    if (j <= least) {
			if (j != 1 || adjust != 1) {
			    /* remove the top of regions stack */
			    mbuf_free(ptrstk_back(&regions));
			    ptrstk_pop_back(&regions);
			}
		    }
		    /* push the regions_out to the top of regions stack */
		    ptrstk_push_back(&regions, ranges_out);
		    region_ref = ptrstk_back(&regions);
		}
		for (--j, j += adjust; j > least; --j) {
		    /* merge top two of the stack */
		    mbuf_t *ranges_top = ptrstk_back(&regions);
		    ptrstk_pop_back(&regions);
		    mbuf_append(ptrstk_back(&regions), ranges_top);
		    mbuf_free(ranges_top);
		}
		break;
	    case KIND_CHS:
		ch = regexp_code_get_ch(code);	/* '[' or '!' */
		region_ref = ptrstk_at(&regions, ptrstk_size(&regions) - 1 - adjust);
		/* read charbits */
		regexp_charbits_read(rx.code + i + 1, &bits);
		i += sizeof(regexp_charbits_t) / sizeof(uint32_t);
#if !defined(NDEBUG) && 0
		putchar('\t'); regexp_charbits_print(stdout, &bits); putchar('\n');
#endif
		/* complement if ch == '!' */
		if (ch == '!')
		    regexp_charbits_not(&bits);
		/* and with alphabet bits */
		regexp_charbits_and(&bits, &alph);

		/* */
		if (regexp_charbits_any(&bits) > 0) {
		    unsigned int k;
		    mbuf_clear(&chs);
		    for (k = 0; k < CHAR_MAX; ++k) {
			if (regexp_charbits_test(&bits, (char)k))
			    mbuf_push_back_1(&chs, (char)k);
		    }
		    for (j = 1; j <= most; ++j) {
			/* prepare output */
			mbuf_t *ranges_out = mbuf_new(NULL, 0);
			if  (ranges_out == NULL) {
			    ret = errno;
			    goto bail4;
			}
			/* search.
			* read from top of regions stack. write to regions_out
			*/ 
			if ((ret = bf_searchXX(region_ref,
					ranges_out, &arg, &chs, none)) != 0)
			{
			    goto bail4;
			}
			/* */
			if (j <= least) {
			    if (j != 1 || adjust != 1) {
				/* remove the top of regions stack */
				mbuf_free(ptrstk_back(&regions));
				ptrstk_pop_back(&regions);
			    }
			}
			/* push the regions_out to the top of regions stack */
			ptrstk_push_back(&regions, ranges_out);
			region_ref = ptrstk_back(&regions);
		    }
		    for (--j, j += adjust; j > least; --j) {
			/* merge top two of the stack */
			mbuf_t *ranges_top = ptrstk_back(&regions);
			ptrstk_pop_back(&regions);
			mbuf_append(ptrstk_back(&regions), ranges_top);
			mbuf_free(ranges_top);
		    }
		}
		break;
	    case KIND_ANY:
		regexp_charbits_cpy(&bits, &alph);
		region_ref = ptrstk_at(&regions, ptrstk_size(&regions) - 1 - adjust);
		if (regexp_charbits_any(&bits) > 0) {
		    unsigned int k;
		    mbuf_clear(&chs);
		    for (k = 0; k < CHAR_MAX; ++k) {
			if (regexp_charbits_test(&bits, (char)k))
			    mbuf_push_back_1(&chs, (char)k);
		    }
		    for (j = 1; j <= most; ++j) {
			/* prepare output */
			mbuf_t *ranges_out = mbuf_new(NULL, 0);
			if  (ranges_out == NULL) {
			    ret = errno;
			    goto bail4;
			}
			/* search.
			* read from top of regions stack. write to regions_out
			*/ 
			if ((ret = bf_searchXX(region_ref,
					ranges_out, &arg, &chs, none)) != 0)
			{
			    goto bail4;
			}
			/* */
			if (j <= least) {
			    if (j != 1 || adjust != 1) {
				/* remove the top of regions stack */
				mbuf_free(ptrstk_back(&regions));
				ptrstk_pop_back(&regions);
			    }
			}
			/* push the regions_out to the top of regions stack */
			ptrstk_push_back(&regions, ranges_out);
			region_ref = ptrstk_back(&regions);
		    }
		    for (--j, j += adjust; j > least; --j) {
			/* merge top two of the stack */
			mbuf_t *ranges_top = ptrstk_back(&regions);
			ptrstk_pop_back(&regions);
			mbuf_append(ptrstk_back(&regions), ranges_top);
			mbuf_free(ranges_top);
		    }
		}
		break;
	    default:
		ret = errno = EINVAL;
		goto bail4;
	}

	adjust = (kind == KIND_OR) ? 1 : 0;
    }

    /* copy the top of the ranges stack back to the region's ranges */
    re->ranges = ptrstk_back(&regions);
    ptrstk_pop_back(&regions);

    /* cleaning */
    mbuf_free(ptrstk_back(&regions));
    ptrstk_pop_back(&regions);

#if !defined(NDEBUG) && 0
    print_ptrstk(stdout, &regions, "regions");
#endif

bail4:
    ptrstk_free(&regions);
bail3:
    u32stk_free(&counters);
bail2:
    mbuf_free(&chs);
bail1:
    regexp_free(&rx);
bail0:
    return ret;
}

#if SIZEOF_INTXX_T < 8
/* search_regexp_max_repeat */
unsigned int search_regexp_max_repeat(void)
{
    return REGEXP_MAX_REPEAT;
}
#endif

/*======================================================================
 * private function definitions
 *======================================================================*/

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
	printf("[[ %p ]]\n", ptrstk_at(stk, i));
}

#endif
