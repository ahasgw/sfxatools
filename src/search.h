/***********************************************************************
 * $Id: search.h,v 1.3 2005/03/17 12:50:14 aki Exp $
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

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef RANGE_H
# include "range.h"
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

int search32(const char *txt, const int32_t *idx, const int32_t len,
	const char *pattern, size_t patlen, range32_t *result);

#if SIZEOF_OFF_T >= 8
int search64(const char *txt, const int64_t *idx, const int64_t len,
	const char *pattern, size_t patlen, range64_t *result);
#endif

#endif /* SEARCH_H */
