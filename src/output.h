/***********************************************************************
 * $Id: output.h,v 1.4 2005/03/22 13:12:48 aki Exp $
 *
 * output header file
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

#ifndef OUTPUT_H
#define OUTPUT_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

#ifndef CMAP_H_INCLUDED
# define CMAP_H_INCLUDED 1
# include "cmap.h"
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct output_param_type {
    cmap_t  *cmap;  /* cmap */
    int	    pos;    /* whether print position column */
    int	    idx;    /* whether print index column */
    int	    sfx;    /* length of suffix column */
    int	    pre;    /* length of substring ahead of the suffix */
    int	    chop;   /* whether chop suffix beyond character '\0' */
} output_param_t;

/*======================================================================
 * function declarations
 *======================================================================*/

int output32(const char *txt, const int32_t *idx, const int32_t len,
	const int32_t beg, const int32_t end, const output_param_t *param);

#if SIZEOF_OFF_T >= 8
int output64(const char *txt, const int64_t *idx, const int64_t len,
	const int64_t beg, const int64_t end, const output_param_t *param);
#endif

#endif /* OUTPUT_H */
