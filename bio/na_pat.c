/***********************************************************************
 * $Id: na_pat.c,v 1.1 2005/10/31 03:03:43 aki Exp $
 *
 * na_pat.c
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

#include <errno.h>
#include <string.h>

#include "na_pat.h"
#include "mbuf.h"

/*======================================================================
 * macro definitions
 *======================================================================*/

#define PUSH(c) \
    do { ret = push_char(&buf, (c)); } while (0)

#define PUSHN(s) \
    do { ret = push_chars(&buf, (s), inclass); } while (0)

#define IUPAC \
	 'A': case 'B': case 'C': case 'D': case 'G': case 'H': case 'K': \
    case 'M': case 'N': case 'R': case 'S': case 'T': case 'U': case 'V': \
    case 'W': case 'Y'

#define DIGIT \
         '0': case '1': case '2': case '3': case '4': case '5': case '6': \
    case '7': case '8': case '9'

/*======================================================================
 * constants definitions
 *======================================================================*/

const char * const  na_alphabet = "ABCDGHKMNRSTUVWY";

/*======================================================================
 * prototypes
 *======================================================================*/

static int push_char(mbuf_t *buf, const char ch);
static int push_chars(mbuf_t *buf, const char *syms, int inclass);

/*======================================================================
 * function definitions
 *======================================================================*/

int na_pat_complement(const char *pat, char **pat_complement)
{
    int ret = 0;
    int inclass = 0;
    mbuf_t buf;

    *pat_complement = NULL;

    if ((ret = mbuf_init(&buf, NULL, 0)) != 0)
	return ret;

    for (; !ret && *pat; ++pat) {
	switch (*pat) {
	    case 'A':	PUSH('T');	break;
	    case 'B':	PUSHN("ACGV");	break;
	    case 'C':	PUSH('G');	break;
	    case 'D':	PUSHN("ACTUH");	break;
	    case 'G':	PUSH('C');	break;
	    case 'H':	PUSHN("AGTUD");	break;
	    case 'K':	PUSHN("ACM");	break;
	    case 'M':	PUSHN("GTUK");	break;
	    case 'N':	PUSHN("ACGTURYMKWSBDHVN");  break;
	    case 'R':	PUSHN("CTUY");	break;
	    case 'S':	PUSHN("CGS");	break;
	    case 'T':	PUSH('A');	break;
	    case 'U':	PUSH('A');	break;
	    case 'V':	PUSHN("CGTUB");	break;
	    case 'W':	PUSHN("ATUW");	break;
	    case 'Y':	PUSHN("AGY");	break;
	    case '[':	PUSH(*pat); inclass = 1;    break;
	    case ']':	PUSH(*pat); inclass = 0;    break;
	    default:	PUSH(*pat);	break;
	}
    }
    PUSH('\0');

    if (ret)
	goto bail0;

    if ((*pat_complement = (char*)malloc(mbuf_size(&buf))) == NULL) {
	ret = errno;
	goto bail0;
    }

    memcpy(*pat_complement, mbuf_ptr(&buf), mbuf_size(&buf));

bail0:
    mbuf_free(&buf);
    return ret;
}

static int push_char(mbuf_t *buf, const char ch)
{
    if (mbuf_push_back_1(buf, ch) != 1)
	return 1;
    return 0;
}

static int push_chars(mbuf_t *buf, const char *syms, int inclass)
{
    size_t n = strlen(syms);
    if (!inclass)
	if (mbuf_push_back_1(buf, '[') != 1)
	    return 1;
    if (mbuf_push_back(buf, syms, n) != n)
	return 1;
    if (!inclass)
	if (mbuf_push_back_1(buf, ']') != 1)
	    return 1;
    return 0;
}
