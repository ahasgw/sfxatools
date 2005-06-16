/***********************************************************************
 * $Id: sfxa.h,v 1.1 2005/06/16 09:59:46 aki Exp $
 *
 * Header file for sfxa
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

#ifndef SFXA_H
#define SFXA_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <mmfile.h>

/*======================================================================
 * type definitions
 *======================================================================*/

/* suffix array data type */
typedef struct sfxa_type {
    mmfile_t        ftxt;
    mmfile_t        fidx;
    mmfile_t        flcp;
} sfxa_t;

/*======================================================================
 * function declarations
 *======================================================================*/

sfxa_t *sfxa_new(const char *ftxt, const char *fidx, const char *flcp);
int sfxa_init(sfxa_t *sa, const char *ftxt, const char *fidx, const char *flcp);
int sfxa_free(sfxa_t *sa);
int sfxa_delete(sfxa_t *sa);

#endif /* SFXA_H */
