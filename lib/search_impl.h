/***********************************************************************
 * $Id: search_impl.h,v 1.2 2005/08/17 10:11:42 aki Exp $
 *
 * search implement header file
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

#ifndef SEARCH_IMPL_H
#define SEARCH_IMPL_H 1
#define SEARCH_IMPL_H_INCLUDED 1

#if 0
#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif
#endif

#ifndef MBUF_H_INCLUDED
# define MBUF_H_INCLUDED 1
# include "mbuf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef enum head_tail_type { none, head, tail } head_tail_t;

typedef int (*search_f)(const mbuf_t *, mbuf_t *, void *, const mbuf_t *, head_tail_t);

typedef struct parser_arg_type {
    const mbuf_t    *ranges_in;
    mbuf_t	    *ranges_out;
    mbuf_t	    *chs;
    const char	    *alphabet;
    void	    *search_arg;
    search_f	    search_func;
    unsigned long   repeat_max;
    /* for lexical scanner */
    const char	    *ptr;
    int		    state;
} parser_arg_t;

/*======================================================================
 * function declarations
 *======================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SEARCH_IMPL_H */
