/***********************************************************************
 * $Id: output.c,v 1.2 2005/02/02 12:14:13 aki Exp $
 *
 * output
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

#include "output.h"

#include <mmfile.h>
#include <msg.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <minmax.h>
#include <string.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define outputXX	    output32
# define outputXX_arg_type  output32_arg_type
# define DIGITOF_INTXX_T    11
#else
# define intXX_t	    int64_t
# define outputXX	    output64
# define outputXX_arg_type  output64_arg_type
# define DIGITOF_INTXX_T    20
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct outputXX_arg_type {
    const char *restrict    txt;
    const intXX_t *restrict idx;
    intXX_t		    len;

    const output_param_t    *param;
    int			    max_digit;
} outputXX_arg_t;

/*======================================================================
 * prototypes
 *======================================================================*/

inline static int get_max_digit(const intXX_t n);
static void print_line(const outputXX_arg_t *arg, const intXX_t x);
static void print_suffix(const outputXX_arg_t *arg, const intXX_t x);

/*======================================================================
 * function definitions
 *======================================================================*/

/* outputXX */
int outputXX(const char *restrict txt, const intXX_t *restrict idx,
	const intXX_t len, const intXX_t beg, const intXX_t end,
	const output_param_t *param)
{
    intXX_t x;
    outputXX_arg_t arg;

    if (beg < 0 || end < 0)
	return (errno = EINVAL);

    arg.txt = txt;
    arg.idx = idx;
    arg.len = len;
    arg.param = param;
    arg.max_digit = get_max_digit(len);

    for (x = beg; x <= end; ++x) {
	print_line(&arg, x);
    }
    return 0;
}

/*======================================================================
 * private function definitions
 *======================================================================*/

inline static int get_max_digit(const intXX_t n)
{
    char buf[DIGITOF_INTXX_T + 1];
    sprintf(buf, "%lld", (long long)n);
    return strlen(buf);
}

static void print_line(const outputXX_arg_t *arg, const intXX_t x)
{
    int col = 0;
    const char *sep = " ";
#if SIZEOF_INTXX_T < 8
    static char fmt[] = "%0*" PRId32;
#else
    static char fmt[] = "%0*" PRId64;
#endif

    assert(arg != NULL);
    assert(arg->param != NULL);

    if (arg->param->pos) {
	if (col++) printf("%s", sep);
	printf(fmt, arg->max_digit, x);
    }

    if (arg->param->idx) {
	if (col++) printf("%s", sep);
	printf(fmt, arg->max_digit, arg->idx[x]);
    }

#if 0
    if (arg->param->pre) {
	if (col++) printf("%s", sep);
	print_prefix(arg, arg->idx[x]);
    }
#endif

    if (arg->param->sfx) {
	if (col++) printf("%s", sep);
	print_suffix(arg, arg->idx[x]);
    }

    printf("\n");
}

static void print_suffix(const outputXX_arg_t *arg, const intXX_t x)
{
    const char *cp = arg->txt + x;
    const char *end = MIN(cp + arg->param->sfx, arg->txt + arg->len);

    if (arg->param->sfx < 0)
	return;

    for (; cp < end; ++cp) {
	if (*cp == '\0') {
	    putchar('\\'); putchar('0');
	    if (arg->param->chop) {
		++cp; break;
	    }
	}
	else if (*cp == '\a') { putchar('\\'); putchar('a'); }
	else if (*cp == '\b') { putchar('\\'); putchar('b'); }
	else if (*cp == '\t') { putchar('\\'); putchar('t'); }
	else if (*cp == '\n') { putchar('\\'); putchar('n'); }
	else if (*cp == '\v') { putchar('\\'); putchar('v'); }
	else if (*cp == '\f') { putchar('\\'); putchar('f'); }
	else if (*cp == '\r') { putchar('\\'); putchar('r'); }
	else if (*cp == '\\') { putchar('\\'); putchar('\\'); }
	else if (isgraph(*cp)) { putchar(*cp); }
	else { putchar('\\'); putchar('?'); }
    }

    if (cp < arg->txt + arg->len)
	printf("...");
}
