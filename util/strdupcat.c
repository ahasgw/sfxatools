/***********************************************************************
 * $Id: strdupcat.c,v 1.2 2005/07/05 05:12:57 aki Exp $
 *
 * string utility
 * Copyright (C) 2004 RIKEN. All rights reserved.
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

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "strdupcat.h"

/*======================================================================
 * string handler
 *======================================================================*/

char *
strdupcat(const char *s1, const char *s2)
{
    char *cp = NULL;
    int s1_len = 0;
    int s2_len = 0;

    if (s1 == NULL)
	return NULL;

    if (s2 != NULL)
	s2_len = strlen(s2);

    s1_len = strlen(s1);
    cp = (char*)malloc(s1_len + s2_len + 1);
    if (cp == NULL)
	return NULL;

    strcpy(cp, s1);

    if (s2 != NULL)
	strcpy(cp + s1_len, s2);

    *(cp + s1_len + s2_len) = '\0';
    return cp;
}
