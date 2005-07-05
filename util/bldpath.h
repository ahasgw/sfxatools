/***********************************************************************
 * $Id: bldpath.h,v 1.2 2005/07/05 07:33:27 aki Exp $
 *
 * bldpath header file
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

#ifndef BLDPATH_H
#define BLDPATH_H 1
#define BLDPATH_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * function declarations
 *======================================================================*/

char *bldpathtmpl(const char *fmt, const char *path, const char *ext, int col);
char *bldpath(const char *tmpl, int num);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BLDPATH_H */
