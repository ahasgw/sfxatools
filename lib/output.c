/***********************************************************************
 * Copyright (C) 2005 RIKEN. All rights reserved.
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
#include <limits.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include "output.h"

#include "msg.h"
#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define outputXX	    output32
# define outputXX_arg_t	    output32_arg_t
#else
# define intXX_t	    int64_t
# define outputXX	    output64
# define outputXX_arg_t	    output64_arg_t
#endif

/*======================================================================
 * prototypes
 *======================================================================*/

static void print_suffix(const outputXX_arg_t *arg, const intXX_t x);

/*======================================================================
 * function definitions
 *======================================================================*/

void outputXX(intXX_t pos, const outputXX_arg_t *arg)
{
    int col = 0;
    const char *sep = " ";
    output_param_t *param;  /* param should be replaced with format string */
			    /* c.f. glibc's register_print_function() */
#if SIZEOF_INTXX_T < 8
    static char fmt[] = "%*" PRId32;
#else
    static char fmt[] = "%*" PRId64;
#endif

    assert(arg != NULL);
    assert(arg->param != NULL);

    param = (output_param_t *)arg->param;

    if (param->idx) {			/* index */
	if (col++) fputs(sep, stdout);
	printf(fmt, param->max_digit, arg->idx[pos] + arg->adj);
    }

    if (param->pos) {			/* index position */
	if (col++) fputs(sep, stdout);
	printf(fmt, param->max_digit, pos);
    }

#if 0
    if (param->pre) {
	if (col++) fputs(sep, stdout);
	print_prefix(arg, arg->idx[pos]);
    }
#endif

    if (param->sfx) {			/* suffix */
	if (col++) fputs(sep, stdout);
	print_suffix(arg, arg->idx[pos] + arg->adj);
    }

    putchar('\n');
}

/*======================================================================
 * private function definitions
 *======================================================================*/

static void print_suffix(const outputXX_arg_t *arg, const intXX_t x)
{
    output_param_t *param = (output_param_t *)arg->param;
    const char *cp = arg->txt + x;
    const char *end = MIN(cp + param->sfx, arg->txt + arg->len);

    if (param->sfx < 0)
	return;

    if (param->cmap == NULL) {
	/* without cmap */
	for (; cp < end; ++cp) {
	    char c = *cp;
	    if (c == '\0') {
		putchar('\\'); putchar('0');
		if (param->chop) {
		    ++cp; break;
		}
	    }
	    else if (c == '\a') { putchar('\\'); putchar('a'); }
	    else if (c == '\b') { putchar('\\'); putchar('b'); }
	    else if (c == '\t') { putchar('\\'); putchar('t'); }
	    else if (c == '\n') { putchar('\\'); putchar('n'); }
	    else if (c == '\v') { putchar('\\'); putchar('v'); }
	    else if (c == '\f') { putchar('\\'); putchar('f'); }
	    else if (c == '\r') { putchar('\\'); putchar('r'); }
	    else if (c == '\\') { putchar('\\'); putchar('\\'); }
	    else if (isgraph(c)) { putchar(c); }
	    else { putchar('\\'); putchar('?'); }
	}
    } else {
	/* with cmap */
	for (; cp < end; ++cp) {
	    char c = cmap_num2char(param->cmap, *cp);
#if 0
	    if (c == '\0') {
		putchar('\\'); putchar('0');
		if (param->chop) {
		    ++cp; break;
		}
	    }
#else
	    if (*cp == '\0') {
		if (isgraph(c))
		    putchar(c);
		else {
		    putchar('\\'); putchar('0');
		}
		if (param->chop) {
		    ++cp; break;
		}
	    }
#endif
	    else if (c == '\a') { putchar('\\'); putchar('a'); }
	    else if (c == '\b') { putchar('\\'); putchar('b'); }
	    else if (c == '\t') { putchar('\\'); putchar('t'); }
	    else if (c == '\n') { putchar('\\'); putchar('n'); }
	    else if (c == '\v') { putchar('\\'); putchar('v'); }
	    else if (c == '\f') { putchar('\\'); putchar('f'); }
	    else if (c == '\r') { putchar('\\'); putchar('r'); }
	    else if (c == '\\') { putchar('\\'); putchar('\\'); }
	    else if (isgraph(c)) { putchar(c); }
	    else { putchar('\\'); putchar('?'); }
	}
    }

    if (cp < arg->txt + arg->len)
	fputs("...", stdout);
}
