/***********************************************************************
 * $Id: cmap.h,v 1.1 2005/03/17 12:50:15 aki Exp $
 *
 * cmap header file
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

#ifndef CMAP_H
#define CMAP_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#endif

#define CMAP_ALPH_SIZE	(UCHAR_MAX + 1)
#define CMAP_UNMAP	(SHRT_MAX)

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * typedefs
 *======================================================================*/

/* character mapping information */
typedef struct cmap_type {
    short   map[CMAP_ALPH_SIZE];
    short   rmap[CMAP_ALPH_SIZE];
} cmap_t;

/*======================================================================
 * prototypes
 *======================================================================*/

cmap_t *cmap_new(void);
void cmap_init(cmap_t *cm);
void cmap_free(cmap_t *cm);
void cmap_delete(cmap_t *cm);

int cmap_char2num(const cmap_t *cm, const char ch);
int cmap_num2char(const cmap_t *cm, const int n);

void cmap_copy(cmap_t * restrict dest_cm, const cmap_t * restrict src_cm);
cmap_t *cmap_dup(const cmap_t *cm);

void cmap_assign(cmap_t *cm, const char ch, const int n);

void cmap_identity(cmap_t *cm);

int cmap_load(cmap_t *cm, const char *path);
int cmap_save(const cmap_t *cm, const char *path);

int cmap_translate(const cmap_t *cm, const char *s, unsigned char **t, int *n);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMAP_H */
