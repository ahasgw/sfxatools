/***********************************************************************
 * $Id: sort.c,v 1.3 2005/02/18 08:38:49 aki Exp $
 *
 * sort
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

/***********************************************************************
 * The program code in this file is based on M. Hiroi's bsm10.c
 * introduced on his web page <http://www.geocities.jp/m_hiroi/>.
 * 
 * See, <http://www.geocities.jp/m_hiroi/zsaru/zsarub22.html>.
 * 
 * Original copyright notice is as follows:
 *     -----------------------------------------------------
 *     bsm10.c : 二段階ソート法 三分割法に rate-2 を適用する
 *               個数の少ない type をソートする
 *             Copyright (C) 2004 Makoto Hiroi
 *     -----------------------------------------------------.
 ***********************************************************************/

/***********************************************************************
 * sort_buffer -> txt
 * sort_table -> idx
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

#include "sort.h"

#include <assert.h>
#include <msg.h>
#include <mmfile.h>

#include <errno.h>
#include <minmax.h>
#include <string.h>
#include <stdint.h>
#include <xalloc.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define sortXX		    sort32
# define sortXX_arg_type    sort32_arg_type
#else
# define intXX_t	    int64_t
# define sortXX		    sort64
# define sortXX_arg_type    sort64_arg_type
#endif

#define ALPH_SIZE	    (UCHAR_MAX + 1)
#define ISORT_THRESH	    (10)

#define SWAP(i, j) {\
    intXX_t tmp = arg->idx[(i)];\
    arg->idx[(i)] = arg->idx[(j)];\
    arg->idx[(j)] = tmp;\
}

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct sortXX_arg_type {
    const char	*txt;
    intXX_t	*idx;
    intXX_t	len;

    intXX_t	*count_sum;
    intXX_t	*start2;
    intXX_t	*work_table;
} sortXX_arg_t;

/*======================================================================
 * prototypes
 *======================================================================*/

inline static int get_code(const char *txt, const intXX_t idx);
static void sort_type_C(const sortXX_arg_t *arg);
static void copy_type_AB(const sortXX_arg_t *arg);
static void sort_type_A(const sortXX_arg_t *arg);
static void copy_type_BC(const sortXX_arg_t *arg);
static void mk_qsort(const sortXX_arg_t *arg, intXX_t low, intXX_t high, intXX_t depth);
static int select_pivot(const sortXX_arg_t *arg, intXX_t low, intXX_t high, intXX_t depth);
static void isort(const sortXX_arg_t *arg, intXX_t low, intXX_t high, intXX_t depth);

/*======================================================================
 * inline function definitions
 *======================================================================*/

inline static int get_code(const char *txt, const intXX_t idx)
{
    return (((int)(txt[idx]) << CHAR_BIT) + txt[idx + 1]);
}

/*======================================================================
 * function definitions
 *======================================================================*/

/* sortXX */
int sortXX(const char *txt, intXX_t *idx, const intXX_t len)
{
    intXX_t i, nA, nC, max;
    intXX_t count[ALPH_SIZE * ALPH_SIZE];
    intXX_t count_sum[ALPH_SIZE * ALPH_SIZE + 1];
    intXX_t start2[ALPH_SIZE * ALPH_SIZE + 1];

    /* bin sort */
    for (i = 0; i < ALPH_SIZE * ALPH_SIZE; ++i)
	count[i] = 0;
    for (i = 0; i < len; ++i)
	++count[get_code(txt, i)];
    count_sum[0] = 0;
    for (i = 1; i <= ALPH_SIZE * ALPH_SIZE; ++i)
	count_sum[i] = count[i - 1] + count_sum[i - 1];
    for (i = 1; i < ALPH_SIZE * ALPH_SIZE; ++i)
	count[i] += count[i - 1];
    for (i = len - 1; i >= 0; --i)
	idx[--count[get_code(txt, i)]] = i;

    /* count number of type A and C */
    for (i = 0, nA = 0, nC = 0, max = 0; i < ALPH_SIZE; ++i) {
	intXX_t n = count_sum[(i << CHAR_BIT) + i] - count_sum[i << CHAR_BIT];
	max = (n > max) ? n : max;
	nA += n;
	n = count_sum[(i + 1) << CHAR_BIT] - count_sum[(i << CHAR_BIT) + i + 1];
	max = (n > max) ? n : max;
	nC += n;
    }

    if (((off_t)sizeof(intXX_t) * max) > SIZE_MAX)  /* !!! */
	return (errno = EFBIG);

    /* sort */
    {
	sortXX_arg_t arg;
	arg.txt = txt;
	arg.idx = idx;
	arg.len = len;
	arg.count_sum = count_sum;
	arg.start2 = start2;
	arg.work_table = (intXX_t*)malloc(sizeof(intXX_t) * (size_t)max);
	if (arg.work_table == NULL)
	    return errno;

	if (nA > nC) {
	    sort_type_C(&arg);
	    copy_type_AB(&arg);
	} else {
	    sort_type_A(&arg);
	    copy_type_BC(&arg);
	}

	free(arg.work_table), arg.work_table = NULL;
    }

    return 0;
}

/*======================================================================
 * private function definitions
 *======================================================================*/

static void sort_type_C(const sortXX_arg_t *arg)
{
    intXX_t i, j;
    for (i = 0; i < ALPH_SIZE; ++i) {
	for (j = i + 1; j < ALPH_SIZE; ++j) {
	    intXX_t k = (i << CHAR_BIT) + j;
	    intXX_t h = arg->count_sum[k + 1];
	    intXX_t m, n;
	    arg->start2[k] = arg->count_sum[k];
	    for (n = m = arg->count_sum[k]; n < h; ++n) {
		intXX_t x = arg->idx[n];
		if (arg->txt[x] > arg->txt[x + 2]) {
		    arg->idx[n] = arg->idx[m];
		    arg->idx[m++] = x;
		}
	    }
	    if (h - m > 1)
		mk_qsort(arg, m, h - 1, 2);
	}
    }
}

static void copy_type_AB(const sortXX_arg_t *arg)
{
    intXX_t start[ALPH_SIZE];
    intXX_t end[ALPH_SIZE];
    intXX_t i;

    for (i = 0; i < ALPH_SIZE; ++i) {
	intXX_t j, wp;
	for (j = i; j < ALPH_SIZE; ++j) {
	    intXX_t k = (j << CHAR_BIT) + i;
	    start[j] = arg->count_sum[k];
	    end[j] = arg->count_sum[k + 1] - 1;
	}
	/* front to back */
	for (j = arg->count_sum[i << CHAR_BIT]; j < start[i]; ++j) {
	    intXX_t x = arg->idx[j];
	    if (x == 0) x += arg->len;
	    if (arg->txt[x - 1] >= i)
		arg->idx[start[(intXX_t)arg->txt[x - 1]]++] = x - 1;
	    /* rate-2 */
	    x = arg->idx[j];
	    if (x < 2) x += arg->len;
	    if (arg->txt[x - 2] >= i
		    && arg->txt[x - 2] < arg->txt[x - 1]
		    && arg->txt[x - 2] > arg->txt[x])
		arg->idx[
		    arg->start2[
		    (arg->txt[x - 2] << CHAR_BIT) + arg->txt[x - 1]
		    ]++
		    ] = x - 2;
	}
	/* back to front */
	for (wp = 0, j = arg->count_sum[(i + 1) << CHAR_BIT] - 1; j > end[i]; --j) {
	    intXX_t x = arg->idx[j];
	    if (x == 0) x += arg->len;
	    if (arg->txt[x - 1] >= i)
		arg->idx[end[(intXX_t)arg->txt[x - 1]]--] = x - 1;
	    /* rate-2 */
	    x = arg->idx[j];
	    if (x < 2) x += arg->len;
	    if (arg->txt[x - 2] >= i
		    && arg->txt[x - 2] < arg->txt[x - 1]
		    && arg->txt[x - 2] > arg->txt[x])
		arg->work_table[wp++] = x - 2;
	}
	/* write back from work_table */
	while (wp > 0) {
	    intXX_t x = arg->work_table[--wp];
	    arg->idx[
		arg->start2[(arg->txt[x] << CHAR_BIT) + arg->txt[x + 1]]++
		] = x;
	}
    }
}

static void sort_type_A(const sortXX_arg_t *arg)
{
    intXX_t i, j;
    for (i = ALPH_SIZE - 1; i >= 0; --i) {
	for (j = 0; j < i; ++j) {
	    intXX_t k = (i << CHAR_BIT) + j;
	    intXX_t m, n;
	    intXX_t h = arg->count_sum[k + 1];
	    arg->start2[k] = h - 1;
	    for (n = m = arg->count_sum[k]; n < h; ++n) {
		intXX_t x = arg->idx[n];
		if (arg->txt[x] >= arg->txt[x + 2]) {
		    arg->idx[n] = arg->idx[m];
		    arg->idx[m++] = x;
		}
	    }
	    if (m - arg->count_sum[k] > 1)
		mk_qsort(arg, arg->count_sum[k], m - 1, 2);
	}
    }
}

static void copy_type_BC(const sortXX_arg_t *arg)
{
    intXX_t start[ALPH_SIZE];
    intXX_t end[ALPH_SIZE];
    intXX_t i;

    for (i = ALPH_SIZE - 1; i >= 0; --i) {
	intXX_t j, wp;
	for (j = 0; j <= i; ++j) {
	    intXX_t k = (j << CHAR_BIT) + i;
	    start[j] = arg->count_sum[k];
	    end[j] = arg->count_sum[k + 1] - 1;
	}
	/* front to back */
	for (wp = 0, j = arg->count_sum[i << CHAR_BIT]; j < start[i]; ++j) {
	    intXX_t x = arg->idx[j];
	    if (x == 0) x += arg->len;
	    if (arg->txt[x - 1] <= i)
		arg->idx[start[(intXX_t)arg->txt[x - 1]]++] = x - 1;
	    /* rate-2 */
	    x = arg->idx[j];
	    if (x < 2) x += arg->len;
	    if (arg->txt[x - 2] <= i
		    && arg->txt[x - 2] > arg->txt[x - 1]
		    && arg->txt[x - 2] < arg->txt[x])
		arg->work_table[wp++] = x - 2;
	}
	/* back to front */
	for (j = arg->count_sum[(i + 1) << CHAR_BIT] - 1; j > end[i]; --j) {
	    intXX_t x = arg->idx[j];
	    if (x == 0) x += arg->len;
	    if (arg->txt[x - 1] <= i)
		arg->idx[end[(intXX_t)arg->txt[x - 1]]--] = x - 1;
	    /* rate-2 */
	    x = arg->idx[j];
	    if (x < 2) x += arg->len;
	    if (arg->txt[x - 2] <= i
		    && arg->txt[x - 2] > arg->txt[x - 1]
		    && arg->txt[x - 2] < arg->txt[x])
		arg->idx[
		    arg->start2[
		    (arg->txt[x - 2] << CHAR_BIT) + arg->txt[x - 1]
		    ]--
		    ] = x - 2;
	}
	/* writeback from work_table */
	while (wp > 0) {
	    intXX_t x = arg->work_table[--wp];
	    arg->idx[
		arg->start2[(arg->txt[x] << CHAR_BIT) + arg->txt[x + 1]]--
		] = x;
	}
    }
}

/* multi-key quick sort */
static void mk_qsort(const sortXX_arg_t *arg, intXX_t low, intXX_t high, intXX_t depth)
{
    for (;;) {
	if (high - low <= ISORT_THRESH) {
	    isort(arg, low, high, depth);
	    break;
	} else {
	    intXX_t i, j, k, l, m1, m2;
	    int pivot = select_pivot(arg, low, high, depth);
	    i = m1 = low;
	    j = m2 = high;
	    for (;; ++i, --j) {
		for (; i <= j; ++i) {
		    k = arg->txt[arg->idx[i] + depth] - pivot;
		    if (k > 0) break;
		    if (k == 0) {
			SWAP(i, m1);
			++m1;
		    }
		}
		for (; i <= j; --j) {
		    k = arg->txt[arg->idx[j] + depth] - pivot;
		    if (k < 0) break;
		    if (k == 0) {
			SWAP(j, m2);
			--m2;
		    }
		}
		if (i > j) break;
		SWAP(i, j);
	    }
	    k = MIN(m1 - low, i - m1);
	    for (l = 0; l < k; ++l) SWAP(low + l, j - l);
	    m1 = low + (i - m1);
	    k = MIN(high - m2, m2 - j);
	    for (l = 0; l < k; ++l) SWAP(i + l, high - l);
	    m2 = high - (m2 - j) + 1;
	    if (low < m1) mk_qsort(arg, low, m1 - 1, depth);
	    if (m2 <= high) mk_qsort(arg, m2, high, depth);
	    if (m1 >= m2 || depth + 1 == arg->len) break;
	    low = m1;
	    high = m2 - 1;
	    ++depth;
	}
    }
}

/* pivot selector */
static int select_pivot(const sortXX_arg_t *arg, intXX_t low, intXX_t high, intXX_t depth)
{
    int a = arg->txt[arg->idx[low] + depth];
    int b = arg->txt[arg->idx[(low + high) / 2] + depth];
    int c = arg->txt[arg->idx[high] + depth];

    if (a > b) {
	int t = a;
	a = b;
	b = t;
    }
    if (b > c) {
	b = c;
	if (a > b)
	    b = a;
    }
    return b;
}

/* insertion sort */
static void isort(const sortXX_arg_t *arg, intXX_t low, intXX_t high, intXX_t depth)
{
    intXX_t i, j, len = arg->len - depth;
    for (i = low + 1; i <= high; ++i) {
	intXX_t x =  arg->idx[i];
	for (j = i - 1;
		j >= low && memcmp(arg->txt + x + depth,
		    arg->txt + arg->idx[j] + depth, (size_t)len) < 0;
		--j)
	{
	    arg->idx[j + 1] = arg->idx[j];
	}
	arg->idx[j + 1] = x;
    }
}
