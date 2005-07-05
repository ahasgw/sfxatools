/***********************************************************************
 * $Id: cmap.c,v 1.2 2005/07/05 05:12:56 aki Exp $
 *
 * cmap
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
#include <errno.h>
#include <string.h>

#include "cmap.h"

/*======================================================================
 * ctor & dtor
 *======================================================================*/

/* allocate cmap_t object */
cmap_t *cmap_new(void)
{
    cmap_t *cm = (cmap_t*)malloc(sizeof(cmap_t));
    if (cm != NULL)
	    cmap_init(cm);
    return cm;
}

/* initialize struct cmap_t with no mapping */
void cmap_init(cmap_t *cm)
{
    short i;
    for (i = 0; i < CMAP_ALPH_SIZE; ++i){
	cm->map[i] = CMAP_UNMAP;
    }
}

/* free */
void cmap_free(cmap_t *cm)
{
}

/* delete cmap_t object */
void cmap_delete(cmap_t *cm)
{
    cmap_free(cm);
    free(cm);
}

/*======================================================================
 * accessor
 *======================================================================*/

/* character symbol to number mapping */
int cmap_char2num(const cmap_t *cm, const char ch)
{
    return cm->map[(unsigned char)ch];
}

/* number to character symbol mapping */
int cmap_num2char(const cmap_t *cm, const int n)
{
    return (n >= 0 && n < CMAP_ALPH_SIZE) ? cm->rmap[n] : CMAP_UNMAP;
}

/*======================================================================
 * manipulator
 *======================================================================*/

/* copying */
void cmap_copy(cmap_t * restrict dest_cm, const cmap_t * restrict src_cm)
{
    memcpy(dest_cm, src_cm, sizeof(cmap_t));
}

/* duplicating */
cmap_t *cmap_dup(const cmap_t *cm)
{
    cmap_t *new = cmap_new();
    if (new != NULL)
	cmap_copy(new, cm);
    return new;
}

/* set character symbol mapping */
void cmap_assign(cmap_t *cm, const char ch, const int n)
{
    if (n >= 0 && n < CMAP_ALPH_SIZE) {
	cm->map[(unsigned char)ch] = n;
	cm->rmap[n] = ch;
    } else {
	cm->map[(unsigned char)ch] = CMAP_UNMAP;
    }
}

/* initialize struct cmap_t with identical mapping */
void cmap_identity(cmap_t *cm)
{
    short i;
    for (i = 0; i < CMAP_ALPH_SIZE; ++i)
	cm->map[i] = cm->rmap[i] = i;
}

/* read cmap from file */
int cmap_load(cmap_t *cm, const char *path)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(path, "r");
    if (fp == NULL)
	return errno;
    cmap_init(cm);
    while ((read = getline(&line, &len, fp)) != -1) {
	if (len >= 3 && line[1] == ' ') {	/* [X Y...] */
	    char ch;
	    int n;
	    if (sscanf(line, "%c %i", &ch, &n) == 2) {
		cmap_assign(cm, ch, n);
	    }
	}
    }
    if (line)
	free(line);
    fclose(fp);
    return 0;
}

/* store cmap to file */
int cmap_save(const cmap_t *cm, const char *path)
{
    FILE *fp;
    int i;
    fp = fopen(path, "w");
    if (fp == NULL)
	return errno;
    for (i = 0; i < CMAP_ALPH_SIZE; ++i) {
	if (cm->map[i] != CMAP_UNMAP)
	    fprintf(fp, "%c %d\n", i, (char)cm->map[i]);
    }
    fclose(fp);
    return 0;
}

/* translate C string according to cmap */
int cmap_translate(const cmap_t *cm, const char *s, unsigned char **t, int *n)
{
    int slen = strlen(s);
    int i;

    *t = NULL;
    *n = 0;

    *t = (unsigned char *)malloc(slen);
    if (*t == NULL)
	return errno;

    for (i = 0; i < slen; ++i) {
	int ch = cmap_char2num(cm, s[i]);
	if (ch != CMAP_UNMAP) {
	    (*t)[(*n)++] = (unsigned char)ch;
	}
    }
    return 0;
}
