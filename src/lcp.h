/***********************************************************************
 * $Id: lcp.h,v 1.2 2005/03/22 13:12:47 aki Exp $
 *
 * lcp header file
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

#ifndef LCP_H
#define LCP_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

int lcp32(const char *txt, const int32_t *idx, int32_t *lcp, const int32_t len);

#if SIZEOF_OFF_T >= 8
int lcp64(const char *txt, const int64_t *idx, int64_t *lcp, const int64_t len);
#endif

#endif /* LCP_H */
