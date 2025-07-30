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

#ifndef PROSITE_H
#define PROSITE_H 1
#define PROSITE_H_INCLUDED 1

#if 0
#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif
#endif

#include "mbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

int prosite_to_regex(const char *pat, mbuf_t *re);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PROSITE_H */
