/***********************************************************************
 * $Id: sort.h,v 1.1.1.1 2005/02/02 10:39:30 aki Exp $
 *
 * sort header file
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

#ifndef SORT_H
#define SORT_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

int sort32(const char *restrict txt, int32_t *restrict idx, const int32_t len);

#if SIZEOF_OFF_T >= 8
int sort64(const char *restrict txt, int64_t *restrict idx, const int64_t len);
#endif

#endif /* SORT_H */
