/***********************************************************************
 * $Id: lcp.c,v 1.2 2005/03/22 13:12:47 aki Exp $
 *
 * lcp
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
 * The program code in this file is based on GetHeight() introduced in
 *   T. Kasai, G. Lee, H. Arimura, S. Arikawa, and K. Park.
 *   Linear-time Longest-Common-Prefix Computation in Suffix Arrays and
 *   Its Applications.
 *   In Proc. the 12th Annual Symposium on Combinatorial Pattern Matching
 *   (CPM'01), LNCS 2089, 181-192, 2001.
 ***********************************************************************/

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include "lcp.h"

//#include <assert.h>

#include <errno.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define intXX_t	    int32_t
# define lcpXX		    lcp32
#else
# define intXX_t	    int64_t
# define lcpXX		    lcp64
#endif

/*======================================================================
 * function definitions
 *======================================================================*/

/* lcpXX */
int lcpXX(const char *txt, const intXX_t *idx, intXX_t *lcp, const intXX_t len)
{
    intXX_t *rank = NULL;
    intXX_t h, i, j;

    if ((rank = (intXX_t*)malloc(len * sizeof(intXX_t))) == NULL)
	return errno;

    for (i = 0; i < len; ++i)
	rank[idx[i]] = i;
    for (h = i = 0; i < len; ++i) {
	if (rank[i] > 0) {
	    j = idx[rank[i] - 1];
	    while (txt[i + h] == txt[j + h])
		++h;
	    lcp[rank[i]] = h;
	    if (h > 0)
		--h;
	}
    }

    free(rank), rank = NULL;
    return 0;
}
