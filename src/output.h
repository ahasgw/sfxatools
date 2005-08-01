/***********************************************************************
 * $Id: output.h,v 1.8 2005/08/01 11:24:37 aki Exp $
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
#define OUTPUT_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

#ifndef REGION_H_INCLUDED
# define REGION_H_INCLUDED 1
# include "region.h"
#endif
#ifndef CMAP_H_INCLUDED
# define CMAP_H_INCLUDED 1
# include "cmap.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct output_param_type {
    cmap_t  *cmap;  /* cmap */
    int	    hdr;    /* whether print information header */
    int	    pos;    /* whether print position column */
    int	    idx;    /* whether print index column */
    int	    sfx;    /* length of suffix column */
    int	    pre;    /* length of substring ahead of the suffix */
    int	    chop;   /* whether chop suffix beyond character '\0' */
    int	    max_digit;
} output_param_t;

/*======================================================================
 * function declarations
 *======================================================================*/

void output32(int32_t pos, const region_print32_arg_t *arg);

#if SIZEOF_OFF_T >= 8
void output64(int64_t pos, const region_print64_arg_t *arg);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OUTPUT_H */
