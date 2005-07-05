/***********************************************************************
 * $Id: ofiles.h,v 1.1 2005/07/05 05:12:54 aki Exp $
 *
 * ofiles header file
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

#ifndef OFILES_H
#define OFILES_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <stdio.h>

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct ofnames_type {
    char            *fhdrname;
    char            *fhdxname;
    char            *fseqname;
} ofnames_t;

typedef struct ofiles_type {
    FILE            *fhdr;
    FILE            *fhdx;
    FILE            *fseq;
    int             num;
} ofiles_t;

/*======================================================================
 * function declarations
 *======================================================================*/

void ofnames_init(ofnames_t *ofns, const char *basename, const char *fhdr_ext, const char *fhdx_ext, const char *fseq_ext, int ext_num_len);
void ofnames_free(ofnames_t *ofns);
void ofiles_open(ofiles_t *ofs, const ofnames_t *ofns, int num);
void ofiles_close(ofiles_t *ofs);

void ofiles_open_next(ofiles_t *newofs, const ofiles_t *ofs, const ofnames_t *ofn);
void ofiles_subst(ofiles_t *destofs, const ofiles_t *srcofs);

#endif /* OFILES_H */
