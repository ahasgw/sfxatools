/***********************************************************************
 * $Id: parsefa.h,v 1.3 2005/07/05 07:33:25 aki Exp $
 *
 * parsefa header file
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

#ifndef PARSEFA_H
#define PARSEFA_H 1
#define PARSEFA_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef CMAP_H_INCLUDED
# define CMAP_H_INCLUDED 1
# include "cmap.h"
#endif

#ifndef OFILES_H_INCLUDED
# define OFILES_H_INCLUDED 1
# include "ofiles.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct parsefa_param_type {
    cmap_t	    *cmap;
    int		    split_size;
    unsigned int    ignore_case: 1;
} parsefa_param_t;

/*======================================================================
 * function declarations
 *======================================================================*/

int parse_fasta32(ofiles_t *ofiles, const ofnames_t *ofnames, const parsefa_param_t *param);

#if SIZEOF_OFF_T >= 8
int parse_fasta64(ofiles_t *ofiles, const ofnames_t *ofnames, const parsefa_param_t *param);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PARSEFA_H */
