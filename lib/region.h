/***********************************************************************
 * Copyright (C) 2005, 2006 RIKEN. All rights reserved.
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

#ifndef REGION_H
#define REGION_H 1
#define REGION_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

#ifndef SFXA_H_INCLUDED
# define SFXA_H_INCLUDED 1
# include "sfxa.h"
#endif
#ifndef OUTPUT_H_INCLUDED
# define OUTPUT_H_INCLUDED 1
# include "output.h"
#endif
#ifndef REGEXP_H_INCLUDED
# define REGEXP_H_INCLUDED 1
# include "regexp.h"
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

typedef struct range32_type {
    int32_t		beg;
    int32_t		end;
    unsigned int	ofst: 30;
    unsigned int	head: 1;
    unsigned int	tail: 1;
} range32_t;

#if SIZEOF_OFF_T >= 8
typedef struct range64_type {
    int64_t		beg;
    int64_t		end;
    unsigned int	ofst: 30;
    unsigned int	head: 1;
    unsigned int	tail: 1;
} range64_t;
#endif /* SIZEOF_OFF_T >= 8 */

typedef struct region_type {
    const sfxa_t	*sa;
    mbuf_t		*ranges;
} region_t;

/*======================================================================
 * function declarations
 *======================================================================*/

int region_init(region_t *re, const sfxa_t *sa);
void region_free(region_t *re);

int region_search(region_t *re, const char *pattern, size_t patlen,
	const char *opt_alphabet);
int region_search_regexp(region_t *re, const regexp_t *rx,
	const char *opt_alphabet, unsigned long rep_max);

int region_narrow_down(region_t *re);
size_t region_count(const region_t *re);

int region_print(const region_t *re, void *param);
int region_dump(const region_t *re, void *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REGION_H */
