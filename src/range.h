/***********************************************************************
 * $Id: range.h,v 1.2 2005/03/22 13:12:48 aki Exp $
 *
 * range header file
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

#ifndef RANGE_H
#define RANGE_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

struct range32_type;

typedef struct range32_type {
    struct range32_type	*next;
    struct range32_type	*prev;
    int32_t		beg;
    int32_t		end;
} range32_t;

#if SIZEOF_OFF_T >= 8
struct range64_type;

typedef struct range64_type {
    struct range64_type	*next;
    struct range64_type	*prev;
    int64_t		beg;
    int64_t		end;
} range64_t;
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

range32_t *range32_union(range32_t *r1, range32_t *r2);
range32_t *range32_isect(range32_t *r1, range32_t *r2);

#if SIZEOF_OFF_T >= 8
range64_t *range64_union(range64_t *r1, range64_t *r2);
range64_t *range64_isect(range64_t *r1, range64_t *r2);
#endif

#endif /* RANGE_H */
