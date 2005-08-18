/***********************************************************************
 * $Id: search.h,v 1.3 2005/08/18 11:20:36 aki Exp $
 *
 * search header file
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

#ifndef SEARCH_H
#define SEARCH_H 1
#define SEARCH_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef REGION_H_INCLUDED
# define REGION_H_INCLUDED 1
# include "region.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

unsigned int search_regexp_max_repeat(void);

int search32(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet);
int search_regexp32(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet, unsigned long rep_max);

#if SIZEOF_OFF_T >= 8
int search64(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet);
int search_regexp64(region_t *re, const char *pattern, size_t patlen, const char *opt_alphabet, unsigned long rep_max);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SEARCH_H */
