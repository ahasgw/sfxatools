/***********************************************************************
 * $Id: bldpath.c,v 1.1 2005/07/05 05:12:56 aki Exp $
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
#include <string.h>

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#else
# ifndef MAXPATHLEN
#  define MAXPATHLEN	4096
# endif
#endif

#include <bldpath.h>

/*======================================================================
 * function definitions
 *======================================================================*/

char *bldpathtmpl(const char *fmt, const char *path, const char *ext, int col)
{
    int ret = 0;
    char tmpl[MAXPATHLEN];

    assert(fmt != NULL);
    assert(path != NULL);
    assert(ext != NULL);
    if ((ret = snprintf(tmpl, MAXPATHLEN, fmt, path, col, ext)) < 0)
	return NULL;
    if (ret >= MAXPATHLEN) {
	errno = ENAMETOOLONG;
	return NULL;
    }
    return strdup(tmpl);
}

char *bldpath(const char *tmpl, int num)
{
    char s[MAXPATHLEN];
    int ret = 0;
    
    assert(tmpl != NULL);
    ret = snprintf(s, MAXPATHLEN, tmpl, num);
    if (ret >= MAXPATHLEN) {
	errno = ENAMETOOLONG;
	return NULL;
    }
    if (ret < 0)
	return NULL;
    return strdup(s);
}
