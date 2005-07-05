/***********************************************************************
 * $Id: bldpath.c,v 1.2 2005/07/05 07:33:27 aki Exp $
 *
 * bldpath.c
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

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "bldpath.h"

/*======================================================================
 * function definitions
 *======================================================================*/

char *bldpathtmpl(const char *fmt, const char *path, const char *ext, int col)
{
    int ret = 0;
    char tmpl[PATH_MAX + 1];

    assert(fmt != NULL);
    assert(path != NULL);
    assert(ext != NULL);
    if ((ret = snprintf(tmpl, PATH_MAX, fmt, path, col, ext)) < 0)
	return NULL;
    if (ret > PATH_MAX) {
	errno = ENAMETOOLONG;
	return NULL;
    }
    return strdup(tmpl);
}

char *bldpath(const char *tmpl, int num)
{
    char s[PATH_MAX + 1];
    int ret = 0;
    
    assert(tmpl != NULL);
    ret = snprintf(s, PATH_MAX, tmpl, num);
    if (ret > PATH_MAX) {
	errno = ENAMETOOLONG;
	return NULL;
    }
    if (ret < 0)
	return NULL;
    return strdup(s);
}
