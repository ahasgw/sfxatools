/***********************************************************************
 * $Id: mmfile.h,v 1.2 2005/07/05 05:12:57 aki Exp $
 *
 * Header file for mmfile functions
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

#ifndef MMFILE_H
#define MMFILE_H 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * typedefs
 *======================================================================*/

/* memory mapped file information */
typedef struct mmfile_type {
    char	*path;
    void	*ptr;
    off_t	sz;
    int		fd;
} mmfile_t;

/*======================================================================
 * prototypes
 *======================================================================*/

/*
 *              PROT
 *          RD   RW   WR
 *         ---------
 *              ---------
 * SHARED   x    x    na
 * PRIVATE  x    x    na
 */

mmfile_t *mmfile_new(const char *path);
int mmfile_init(mmfile_t *mf, const char *path);
void mmfile_free(mmfile_t *mf);
void mmfile_delete(mmfile_t *mf);

const char *mmfile_path(const mmfile_t *mf);
void *mmfile_ptr(const mmfile_t *mf);
off_t mmfile_len(const mmfile_t *mf);
int mmfile_fileno(const mmfile_t *mf);
int mmfile_truncate(mmfile_t *mf, off_t len);

int mmfile_map_shared_rd(mmfile_t *mf);
int mmfile_map_shared_rw(mmfile_t *mf, off_t len);
int mmfile_map_private_rd(mmfile_t *mf);
int mmfile_map_private_rw(mmfile_t *mf);
int mmfile_sync(mmfile_t *mf, int flag);
int mmfile_unmap(mmfile_t *mf);

#ifndef NDEBUG
void mmfile_print(const mmfile_t *mf, const char *comment);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MMFILE_H */
